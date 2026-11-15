"""
InvestSphere Auth Routes
POST /api/auth/register
POST /api/auth/login
POST /api/auth/logout
GET  /api/auth/me
POST /api/auth/change-password
"""

from flask import Blueprint, request, g
from app import get_db
from security import (
    hash_password, verify_password, generate_token,
    is_valid_email, is_strong_password, is_valid_mobile,
    normalize_mobile, is_valid_pan, mask_pan, is_valid_dob,
    verify_math_captcha, generate_math_captcha
)
from helpers import require_auth, success, error
import datetime
import os

auth_bp = Blueprint("auth", __name__)


@auth_bp.route("/config", methods=["GET"])
def auth_config():
    """Return public authentication configuration and a fresh math captcha challenge."""
    captcha = generate_math_captcha()
    return success({
        "captcha_method": "math",
        "captcha_question": captcha["question"],
        "captcha_challenge_id": captcha["challenge_id"],
        "default_country": "India",
        "default_state": "Maharashtra"
    })


@auth_bp.route("/captcha", methods=["GET"])
def new_captcha():
    """Issue a fresh math challenge (called when user clicks Refresh)."""
    captcha = generate_math_captcha()
    return success({
        "question": captcha["question"],
        "challenge_id": captcha["challenge_id"]
    })


@auth_bp.route("/register", methods=["POST"])
def register():
    data = request.get_json(silent=True) or {}
    full_name = (data.get("full_name") or data.get("name") or "").strip()
    email = (data.get("email") or "").strip().lower()
    mobile_raw = (data.get("mobile_number") or data.get("mobile") or "").strip()
    country = (data.get("country") or "India").strip()
    state = (data.get("state") or "Maharashtra").strip()
    pan_raw = (data.get("pan") or "").strip().upper()
    dob_raw = (data.get("date_of_birth") or data.get("dob") or "").strip()
    password = data.get("password") or ""
    confirm_password = data.get("confirm_password") or ""
    # Math captcha fields
    captcha_challenge_id = (data.get("captcha_challenge_id") or "").strip()
    captcha_answer = (data.get("captcha_answer") or "").strip()

    # 1. Full Name
    if not full_name:
        return error("Full name is required.")
    if len(full_name) < 2 or len(full_name) > 100:
        return error("Full name must be between 2 and 100 characters.")

    # 2. Email Address
    if not email:
        return error("Email address is required.")
    if not is_valid_email(email):
        return error("Please enter a valid email address (e.g. user@example.com).")

    # 3. Mobile Number
    if mobile_raw:
        if not is_valid_mobile(mobile_raw):
            return error("Please enter a valid 10-digit mobile number (starting with 6, 7, 8, or 9).")
        mobile = normalize_mobile(mobile_raw)
    else:
        mobile = None

    # 4. PAN (Optional, but if provided must be strictly valid)
    pan_masked = None
    if pan_raw:
        if not is_valid_pan(pan_raw):
            return error("Invalid PAN format. Standard format is 5 letters, 4 digits, 1 letter (e.g. ABCDE1234F).")
        pan_masked = mask_pan(pan_raw)

    # 5. Date of Birth (Optional or validated if provided)
    if dob_raw:
        ok, dob_msg = is_valid_dob(dob_raw)
        if not ok:
            return error(dob_msg)

    # 6. Password & Confirmation
    if not password:
        return error("Password is required.")
    ok, msg = is_strong_password(password)
    if not ok:
        return error(msg)
    if password != confirm_password:
        return error("Passwords do not match. Please verify and retype.")

    # 7. Math Security Check (replaces reCAPTCHA — no third-party dependency)
    cap_ok, cap_msg = verify_math_captcha(captcha_challenge_id, captcha_answer)
    if not cap_ok:
        return error(cap_msg, 400)

    db = get_db()

    # 8. Uniqueness Checks
    existing = db.execute(
        "SELECT user_id FROM users WHERE email = ?", (email,)
    ).fetchone()
    if existing:
        return error("An account with this email address already exists.", 409)

    if mobile:
        existing_mobile = db.execute(
            "SELECT user_id FROM users WHERE mobile_number = ?", (mobile,)
        ).fetchone()
        if existing_mobile:
            return error("An account with this mobile number already exists.", 409)

    # 9. Password Hashing
    pw_hash = hash_password(password)
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

    try:
        cur = db.execute(
            """INSERT INTO users (full_name, email, password_hash, account_status,
                                  mobile_number, country, state, pan_masked, date_of_birth,
                                  created_at, updated_at)
               VALUES (?, ?, ?, 'ACTIVE', ?, ?, ?, ?, ?, ?, ?)""",
            (full_name, email, pw_hash, mobile, country, state, pan_masked, dob_raw or None, now, now)
        )
        db.commit()
        user_id = cur.lastrowid
    except Exception as e:
        db.rollback()
        return error(f"Database error during registration: {str(e)}", 500)

    # Log the registration
    _log_audit(db, user_id, "INSERT", "USER", user_id, "New user account registered")

    # Generate session token
    token = generate_token(user_id, email)

    return success({
        "token": token,
        "user": {
            "id": user_id,
            "full_name": full_name,
            "email": email,
            "mobile_number": mobile,
            "country": country,
            "state": state,
            "pan_masked": pan_masked
        }
    }, "Account created successfully! Welcome to InvestSphere.", 201)


@auth_bp.route("/login", methods=["POST"])
def login():
    data = request.get_json(silent=True) or {}
    email = (data.get("email") or "").strip().lower()
    password = data.get("password") or ""

    if not email or not password:
        return error("Email and password are required.")

    db = get_db()
    user = db.execute(
        """SELECT user_id, full_name, email, password_hash, account_status,
                  mobile_number, country, state, pan_masked, date_of_birth
           FROM users WHERE email = ?""", (email,)
    ).fetchone()

    # Generic error to prevent user enumeration
    generic_err = "Invalid email or password."
    if not user:
        return error(generic_err, 401)

    if not verify_password(password, user["password_hash"]):
        return error(generic_err, 401)

    if user["account_status"] in ("SUSPENDED", "CLOSED"):
        return error("Your account has been deactivated or suspended. Please contact support.", 403)

    token = generate_token(user["user_id"], user["email"])
    _log_audit(db, user["user_id"], "LOGIN", "USER", user["user_id"], "User login")

    return success({
        "token": token,
        "user": {
            "id": user["user_id"],
            "full_name": user["full_name"],
            "email": user["email"],
            "mobile_number": user["mobile_number"],
            "country": user["country"],
            "state": user["state"],
            "pan_masked": user["pan_masked"]
        }
    }, "Login successful.")


@auth_bp.route("/logout", methods=["POST"])
@require_auth
def logout():
    db = get_db()
    _log_audit(db, g.user_id, "LOGOUT", "USER", g.user_id, "User logout")
    return success(message="Logged out successfully.")


@auth_bp.route("/me", methods=["GET"])
@require_auth
def me():
    db = get_db()
    user = db.execute(
        """SELECT user_id, full_name, email, account_status, mobile_number,
                  country, state, pan_masked, date_of_birth, created_at
           FROM users WHERE user_id = ?""",
        (g.user_id,)
    ).fetchone()
    if not user:
        return error("User not found.", 404)
    return success({
        "id": user["user_id"],
        "full_name": user["full_name"],
        "email": user["email"],
        "mobile_number": user["mobile_number"],
        "country": user["country"],
        "state": user["state"],
        "pan_masked": user["pan_masked"],
        "date_of_birth": user["date_of_birth"],
        "account_status": user["account_status"],
        "member_since": user["created_at"]
    })


@auth_bp.route("/change-password", methods=["POST"])
@require_auth
def change_password():
    data = request.get_json(silent=True) or {}
    current_pw = data.get("current_password") or ""
    new_pw = data.get("new_password") or ""
    confirm_pw = data.get("confirm_new_password") or ""

    if not current_pw or not new_pw:
        return error("All password fields are required.")
    if new_pw != confirm_pw:
        return error("New password and confirmation do not match.")
    ok, msg = is_strong_password(new_pw)
    if not ok:
        return error(msg)

    db = get_db()
    user = db.execute(
        "SELECT password_hash FROM users WHERE user_id = ?", (g.user_id,)
    ).fetchone()
    if not user:
        return error("User not found.", 404)

    if not verify_password(current_pw, user["password_hash"]):
        return error("Current password is incorrect.", 401)

    new_hash = hash_password(new_pw)
    db.execute(
        "UPDATE users SET password_hash = ? WHERE user_id = ?",
        (new_hash, g.user_id)
    )
    db.commit()
    return success(message="Password updated successfully.")


@auth_bp.route("/reset-password", methods=["POST"])
def reset_password():
    """Allow user to reset password using email + verified security challenge."""
    data = request.get_json(silent=True) or {}
    email = (data.get("email") or "").strip().lower()
    new_pw = data.get("new_password") or ""
    confirm_pw = data.get("confirm_password") or ""
    captcha_challenge_id = (data.get("captcha_challenge_id") or "").strip()
    captcha_answer = (data.get("captcha_answer") or "").strip()

    if not email:
        return error("Email address is required.")
    if not new_pw:
        return error("New password is required.")
    if new_pw != confirm_pw:
        return error("Passwords do not match.")

    ok, msg = is_strong_password(new_pw)
    if not ok:
        return error(msg)

    # Captcha verification
    cap_ok, cap_msg = verify_math_captcha(captcha_challenge_id, captcha_answer)
    if not cap_ok:
        return error(cap_msg, 400)

    db = get_db()
    user = db.execute(
        "SELECT user_id, full_name, email, account_status FROM users WHERE email = ?",
        (email,)
    ).fetchone()

    if not user:
        return error("No account found with this email address. Please register a new account.", 404)

    if user["account_status"] in ("SUSPENDED", "CLOSED"):
        return error("This account has been deactivated. Please contact support.", 403)

    new_hash = hash_password(new_pw)
    db.execute(
        "UPDATE users SET password_hash = ? WHERE user_id = ?",
        (new_hash, user["user_id"])
    )
    db.commit()

    token = generate_token(user["user_id"], user["email"])
    _log_audit(db, user["user_id"], "PASSWORD_RESET", "USER", user["user_id"], "Password reset via security question")

    return success({
        "token": token,
        "user": {
            "id": user["user_id"],
            "full_name": user["full_name"],
            "email": user["email"]
        }
    }, "Password reset successfully! You are now signed in.")


@auth_bp.route("/profile", methods=["PUT"])
@require_auth
def update_profile():
    data = request.get_json(silent=True) or {}
    full_name = (data.get("full_name") or "").strip()
    if not full_name:
        return error("Full name cannot be empty.")
    if len(full_name) > 100:
        return error("Full name is too long.")

    db = get_db()
    db.execute(
        "UPDATE users SET full_name = ? WHERE user_id = ?",
        (full_name, g.user_id)
    )
    db.commit()
    return success({"full_name": full_name}, "Profile updated successfully.")


def _log_audit(db, user_id, action_type, entity_type, entity_id, description):
    try:
        db.execute(
            "INSERT INTO audit_logs (user_id, action_type, entity_type, entity_id, action_timestamp, description) "
            "VALUES (?, ?, ?, ?, datetime('now'), ?)",
            (user_id, action_type, entity_type, entity_id, description)
        )
        db.commit()
    except Exception:
        pass


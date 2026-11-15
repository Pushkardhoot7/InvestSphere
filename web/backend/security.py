"""
InvestSphere Security Module
Password hashing: PBKDF2-SHA256 (mirrors C++ PasswordHasher)
Session tokens: HMAC-SHA256 signed JWT-style tokens
"""

import os
import hmac
import hashlib
import secrets
import struct
import time
import json
import base64


# ─────────────────────────────────────────────────────────────────────────────
# PBKDF2-SHA256 Password Hashing
# Mirrors the C++ PasswordHasher implementation
# ─────────────────────────────────────────────────────────────────────────────

DEFAULT_ITERATIONS = 100_000
DEFAULT_SALT_BYTES = 32
DEFAULT_KEY_BYTES = 32


def hash_password(password: str, iterations: int = DEFAULT_ITERATIONS) -> str:
    """Hash a password using PBKDF2-SHA256 with random salt.
    Returns format: $pbkdf2-sha256$i=<iter>,l=<keylen>$<salt_hex>$<hash_hex>
    Compatible with C++ PasswordHasher format.
    """
    salt = secrets.token_bytes(DEFAULT_SALT_BYTES)
    salt_hex = salt.hex()

    dk = hashlib.pbkdf2_hmac(
        "sha256",
        password.encode("utf-8"),
        salt,
        iterations,
        dklen=DEFAULT_KEY_BYTES
    )
    hash_hex = dk.hex()

    return f"$pbkdf2-sha256$i={iterations},l={DEFAULT_KEY_BYTES}${salt_hex}${hash_hex}"


def verify_password(password: str, stored_hash: str) -> bool:
    """Verify a password against stored hash. Constant-time comparison."""
    if not stored_hash.startswith("$pbkdf2-sha256$"):
        return False

    parts = [p for p in stored_hash.split("$") if p]
    if len(parts) < 4:
        return False

    # parts[0] = "pbkdf2-sha256"
    # parts[1] = "i=100000,l=32"
    # parts[2] = salt_hex
    # parts[3] = hash_hex
    params = parts[1]
    salt_hex = parts[2]
    expected_hex = parts[3]

    iterations = DEFAULT_ITERATIONS
    key_len = DEFAULT_KEY_BYTES

    for param in params.split(","):
        if param.startswith("i="):
            try:
                iterations = int(param[2:])
            except ValueError:
                return False
        elif param.startswith("l="):
            try:
                key_len = int(param[2:])
            except ValueError:
                return False

    try:
        salt = bytes.fromhex(salt_hex)
    except ValueError:
        return False

    dk = hashlib.pbkdf2_hmac(
        "sha256",
        password.encode("utf-8"),
        salt,
        iterations,
        dklen=key_len
    )
    actual_hex = dk.hex()

    return hmac.compare_digest(actual_hex, expected_hex)


# ─────────────────────────────────────────────────────────────────────────────
# Session Token Management
# Simple HMAC-SHA256 signed tokens (no external JWT library dependency)
# ─────────────────────────────────────────────────────────────────────────────

def _get_secret_key() -> bytes:
    key = os.environ.get("SECRET_KEY", "")
    if not key:
        raise RuntimeError("SECRET_KEY environment variable is not set.")
    return key.encode("utf-8")


def generate_token(user_id: int, user_email: str, expires_in: int = 86400 * 7) -> str:
    """Generate a signed session token valid for expires_in seconds (default 7 days)."""
    payload = {
        "uid": user_id,
        "email": user_email,
        "exp": int(time.time()) + expires_in,
        "iat": int(time.time()),
        "jti": secrets.token_hex(16),
    }
    payload_b64 = base64.urlsafe_b64encode(
        json.dumps(payload).encode("utf-8")
    ).rstrip(b"=").decode("ascii")

    sig = hmac.new(
        _get_secret_key(),
        payload_b64.encode("ascii"),
        hashlib.sha256
    ).hexdigest()

    return f"{payload_b64}.{sig}"


def verify_token(token: str) -> dict | None:
    """Verify and decode a session token. Returns payload dict or None."""
    if not token:
        return None

    parts = token.split(".")
    if len(parts) != 2:
        return None

    payload_b64, sig = parts[0], parts[1]

    expected_sig = hmac.new(
        _get_secret_key(),
        payload_b64.encode("ascii"),
        hashlib.sha256
    ).hexdigest()

    if not hmac.compare_digest(expected_sig, sig):
        return None

    # Decode payload
    padding = 4 - len(payload_b64) % 4
    if padding != 4:
        payload_b64 += "=" * padding

    try:
        payload = json.loads(base64.urlsafe_b64decode(payload_b64).decode("utf-8"))
    except (ValueError, UnicodeDecodeError):
        return None

    # Check expiry
    if payload.get("exp", 0) < time.time():
        return None

    return payload


# ─────────────────────────────────────────────────────────────────────────────
# Input Validation
# ─────────────────────────────────────────────────────────────────────────────

import re

EMAIL_PATTERN = re.compile(r"^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$")


def is_valid_email(email: str) -> bool:
    return bool(EMAIL_PATTERN.match(email.strip()))


def is_strong_password(password: str) -> tuple[bool, str]:
    if len(password) < 8:
        return False, "Password must be at least 8 characters long."
    has_letter = any(c.isalpha() for c in password)
    has_digit = any(c.isdigit() for c in password)
    if not has_letter:
        return False, "Password must contain at least one alphabetical letter."
    if not has_digit:
        return False, "Password must contain at least one numeric digit."
    return True, ""


# ─────────────────────────────────────────────────────────────────────────────
# Additional Input Validations (Mobile, PAN, DOB, reCAPTCHA)
# ─────────────────────────────────────────────────────────────────────────────

MOBILE_PATTERN = re.compile(r"^(?:\+?91[\-\s]?)?[6-9]\d{9}$")
PAN_PATTERN = re.compile(r"^[A-Z]{5}[0-9]{4}[A-Z]$")


def is_valid_mobile(mobile: str) -> bool:
    """Validate Indian or standard international mobile number."""
    if not mobile:
        return False
    clean = re.sub(r"[\s\-\(\)]", "", mobile.strip())
    if clean.startswith("+91"):
        clean = clean[3:]
    elif clean.startswith("91") and len(clean) == 12:
        clean = clean[2:]
    return bool(re.match(r"^[6-9]\d{9}$", clean))


def normalize_mobile(mobile: str) -> str:
    """Normalize mobile to standard 10-digit Indian mobile string."""
    clean = re.sub(r"[\s\-\(\)]", "", mobile.strip())
    if clean.startswith("+91"):
        clean = clean[3:]
    elif clean.startswith("91") and len(clean) == 12:
        clean = clean[2:]
    return clean


def is_valid_pan(pan: str) -> bool:
    """Validate standard Indian Permanent Account Number (PAN)."""
    if not pan:
        return False
    return bool(PAN_PATTERN.match(pan.strip().upper()))


def mask_pan(pan: str) -> str:
    """Mask PAN for privacy protection (e.g. ABCDE1234F -> XXXXX1234F)."""
    if not pan:
        return ""
    p = pan.strip().upper()
    if len(p) == 10:
        return "XXXXX" + p[5:]
    return "XXXXX"


def is_valid_dob(dob_str: str) -> tuple[bool, str]:
    """Validate date of birth and verify age is at least 18 years."""
    if not dob_str:
        return False, "Date of birth is required."
    import datetime
    try:
        dob = datetime.date.fromisoformat(dob_str.strip())
    except ValueError:
        return False, "Invalid date format for date of birth (expected YYYY-MM-DD)."
    today = datetime.date.today()
    if dob > today:
        return False, "Date of birth cannot be in the future."
    # Calculate age
    age = today.year - dob.year - ((today.month, today.day) < (dob.month, dob.day))
    if age < 18:
        return False, f"You must be at least 18 years old to open an investment account (current age: {age})."
    if age > 120:
        return False, "Please enter a realistic date of birth."
    return True, ""


def verify_math_captcha(challenge_id: str, user_answer: str) -> tuple[bool, str]:
    """
    Verify a server-issued math challenge answer.
    Uses HMAC-signed challenge IDs so no server-side session storage is needed.
    Challenge format: base64(a:op:b:expiry).hmac_sig
    """
    if not challenge_id or not user_answer:
        return False, "Security check answer is required."

    try:
        import base64
        import time

        parts = challenge_id.split(".")
        if len(parts) != 2:
            return False, "Invalid security challenge. Please refresh and try again."

        payload_b64, sig = parts[0], parts[1]

        # Verify HMAC signature
        expected_sig = hmac.new(
            _get_secret_key(),
            payload_b64.encode("ascii"),
            hashlib.sha256
        ).hexdigest()[:16]

        if not hmac.compare_digest(expected_sig, sig):
            return False, "Security challenge tampered. Please refresh and try again."

        # Decode payload
        padding = 4 - len(payload_b64) % 4
        if padding != 4:
            payload_b64 += "=" * padding
        decoded = base64.urlsafe_b64decode(payload_b64).decode("utf-8")
        a_str, op, b_str, expiry_str = decoded.split(":")
        a, b, expiry = int(a_str), int(b_str), int(expiry_str)

        # Check expiry (5-minute window)
        if time.time() > expiry:
            return False, "Security challenge expired. Please refresh the page and try again."

        # Compute expected answer
        if op == "+":
            correct = a + b
        elif op == "-":
            correct = a - b
        elif op == "*":
            correct = a * b
        else:
            return False, "Unknown operation in challenge."

        try:
            given = int(str(user_answer).strip())
        except (ValueError, TypeError):
            return False, "Please enter a numeric answer for the security check."

        if given == correct:
            return True, "Security check passed."
        return False, f"Incorrect answer. Hint: {a} {op} {b} = ?"
    except Exception as e:
        return False, f"Security check error: {str(e)}"


def generate_math_captcha() -> dict:
    """
    Generate a new server-signed math challenge.
    Returns { question: str, challenge_id: str }
    No session required — answer is embedded in HMAC-signed token.
    """
    import base64
    import time
    import random

    ops = ["+", "-", "*"]
    op = random.choice(ops)
    if op == "+":
        a, b = random.randint(2, 20), random.randint(2, 20)
        label = f"{a} + {b}"
    elif op == "-":
        a = random.randint(10, 30)
        b = random.randint(1, a)
        label = f"{a} − {b}"
    else:  # *
        a, b = random.randint(2, 9), random.randint(2, 9)
        label = f"{a} × {b}"

    expiry = int(time.time()) + 300  # 5 minutes
    payload = f"{a}:{op}:{b}:{expiry}"
    payload_b64 = base64.urlsafe_b64encode(payload.encode("utf-8")).rstrip(b"=").decode("ascii")
    sig = hmac.new(
        _get_secret_key(),
        payload_b64.encode("ascii"),
        hashlib.sha256
    ).hexdigest()[:16]

    return {
        "question": f"What is {label}?",
        "challenge_id": f"{payload_b64}.{sig}"
    }



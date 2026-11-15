"""
InvestSphere Mutual Funds & SIP Routes
Endpoints for Indian mutual fund discovery, comparison, recommendations,
step-up SIP calculations, and user SIP portfolio management.
"""

import datetime
from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success, error
from services.mutual_funds_service import (
    list_schemes, get_scheme_by_id, calculate_sip,
    generate_sip_recommendations, compare_schemes
)

mutual_funds_bp = Blueprint("mutual_funds", __name__)


@mutual_funds_bp.route("", methods=["GET"])
def get_schemes():
    """List and filter Indian mutual fund schemes."""
    asset_class = request.args.get("asset_class", "ALL")
    category = request.args.get("category", "ALL")
    search = request.args.get("search", "")
    sort_by = request.args.get("sort_by", "returns_3y")
    page = int(request.args.get("page", 1))
    limit = int(request.args.get("limit", 12))

    data = list_schemes(asset_class=asset_class, category=category, search=search, sort_by=sort_by, page=page, limit=limit)
    return success(data)


@mutual_funds_bp.route("/<scheme_id>", methods=["GET"])
def scheme_detail(scheme_id):
    """Retrieve full details of a specific mutual fund scheme."""
    s = get_scheme_by_id(scheme_id)
    if not s:
        return error("Mutual fund scheme not found", 404)
    return success(s)


@mutual_funds_bp.route("/compare", methods=["POST"])
def compare():
    """Compare mutual fund schemes side by side (up to 5)."""
    data = request.get_json(silent=True) or {}
    scheme_ids = data.get("scheme_ids", [])
    if not scheme_ids or not isinstance(scheme_ids, list):
        return error("Please provide a list of scheme IDs to compare.")
    if len(scheme_ids) < 1 or len(scheme_ids) > 6:
        return error("Comparison supports between 1 and 5 schemes.")
    schemes = compare_schemes(scheme_ids)
    return success(schemes)


@mutual_funds_bp.route("/calculate-sip", methods=["POST"])
def sip_calculator():
    """Calculate future value of SIP with optional annual step-up."""
    data = request.get_json(silent=True) or {}
    try:
        monthly = float(data.get("monthly_investment", 5000))
        rate = float(data.get("annual_return_pct", 12.0))
        years = int(data.get("duration_years", 10))
        step_up = float(data.get("step_up_pct", 0.0))
    except (ValueError, TypeError):
        return error("Invalid input values. Please provide numeric parameters.")

    if monthly <= 0:
        return error("Monthly investment must be greater than zero.")
    if years <= 0 or years > 50:
        return error("Duration in years must be between 1 and 50.")
    if rate < -100 or rate > 100:
        return error("Annual return rate must be between -100% and 100%.")

    result = calculate_sip(monthly, rate, years, step_up)
    return success(result)


@mutual_funds_bp.route("/recommend", methods=["POST"])
def recommend():
    """Generate personalized mutual fund and asset allocation recommendations."""
    data = request.get_json(silent=True) or {}
    try:
        monthly_budget = float(data.get("monthly_budget", 5000))
        horizon_years = int(data.get("horizon_years", 7))
    except (ValueError, TypeError):
        return error("Please provide valid monthly budget and duration in years.")

    risk = str(data.get("risk_tolerance", "MODERATE"))
    goal = str(data.get("financial_goal", "WEALTH_CREATION"))
    tax_saving = bool(data.get("tax_saving", False))

    recs = generate_sip_recommendations(
        monthly_budget=monthly_budget,
        horizon_years=horizon_years,
        risk_tolerance=risk,
        financial_goal=goal,
        tax_saving=tax_saving
    )
    return success(recs)


# ── USER SIP PORTFOLIO MANAGEMENT ─────────────────────────────────────────────

@mutual_funds_bp.route("/my-sips", methods=["GET"])
@require_auth
def get_user_sips():
    """List all active and paused SIP plans created by the authenticated user."""
    db = get_db()
    uid = g.user_id
    rows = db.execute(
        """SELECT sip_plan_id, plan_name, monthly_amount, expected_annual_return,
                  duration_months, start_date, end_date, step_up_percent,
                  contribution_day_of_month, projected_invested_capital,
                  projected_future_value, status, created_at
           FROM sip_plans
           WHERE user_id = ?
           ORDER BY created_at DESC""",
        (uid,)
    ).fetchall()

    plans = []
    for r in rows:
        monthly_cents = r["monthly_amount"]
        monthly_rs = round(monthly_cents / 100, 2)
        invested_cents = r["projected_invested_capital"]
        invested_rs = round(invested_cents / 100, 2)
        fv_cents = r["projected_future_value"]
        fv_rs = round(fv_cents / 100, 2)

        plans.append({
            "sip_plan_id": r["sip_plan_id"],
            "plan_name": r["plan_name"],
            "monthly_amount": monthly_rs,
            "monthly_amount_cents": monthly_cents,
            "expected_annual_return": r["expected_annual_return"],
            "duration_months": r["duration_months"],
            "duration_years": round(r["duration_months"] / 12, 1),
            "start_date": r["start_date"],
            "end_date": r["end_date"],
            "step_up_percent": r["step_up_percent"],
            "contribution_day": r["contribution_day_of_month"],
            "projected_invested": invested_rs,
            "projected_future_value": fv_rs,
            "status": r["status"],
            "created_at": r["created_at"]
        })
    return success(plans)


@mutual_funds_bp.route("/my-sips", methods=["POST"])
@require_auth
def create_user_sip():
    """Create a new user-managed SIP plan."""
    data = request.get_json(silent=True) or {}
    plan_name = (data.get("plan_name") or data.get("scheme_name") or "").strip()
    try:
        monthly = float(data.get("monthly_amount") or data.get("monthly_investment") or 0)
        rate = float(data.get("expected_annual_return") or 12.0)
        years = float(data.get("duration_years") or 5)
        step_up = float(data.get("step_up_percent") or data.get("step_up_pct") or 0.0)
        day = int(data.get("contribution_day") or data.get("sip_day") or 5)
    except (ValueError, TypeError):
        return error("Please provide valid numeric values for amount, return, and duration.")

    if not plan_name:
        return error("Plan name or mutual fund scheme name is required.")
    if monthly < 500:
        return error("Minimum monthly SIP amount is ₹500.")
    if years <= 0:
        return error("Duration in years must be greater than zero.")

    duration_months = int(years * 12)
    start_date = data.get("start_date") or datetime.date.today().isoformat()
    end_date = data.get("end_date")
    if not end_date:
        start_dt = datetime.date.fromisoformat(start_date)
        end_dt = start_dt + datetime.timedelta(days=int(duration_months * 30.4375))
        end_date = end_dt.isoformat()

    # Calculate projections
    sim = calculate_sip(monthly, rate, int(years), step_up)
    invested_cents = int(round(sim["total_invested"] * 100))
    fv_cents = int(round(sim["future_value"] * 100))
    monthly_cents = int(round(monthly * 100))

    db = get_db()
    uid = g.user_id
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

    try:
        cur = db.execute(
            """INSERT INTO sip_plans (
                   user_id, plan_name, monthly_amount, expected_annual_return,
                   duration_months, start_date, end_date, step_up_percent,
                   contribution_day_of_month, projected_invested_capital,
                   projected_future_value, status, created_at
               ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 'ACTIVE', ?)""",
            (uid, plan_name, monthly_cents, rate, duration_months, start_date,
             end_date, step_up, day, invested_cents, fv_cents, now)
        )
        db.commit()
        plan_id = cur.lastrowid
        return success({"sip_plan_id": plan_id, "plan_name": plan_name}, "SIP Plan created successfully!", 201)
    except Exception as e:
        db.rollback()
        return error(f"Failed to record SIP plan: {str(e)}", 500)


@mutual_funds_bp.route("/my-sips/<int:plan_id>/status", methods=["PATCH", "POST"])
@require_auth
def update_sip_status(plan_id):
    """Update SIP plan status (ACTIVE, PAUSED, COMPLETED, CANCELLED)."""
    data = request.get_json(silent=True) or {}
    new_status = (data.get("status") or "").strip().upper()
    if new_status not in ("ACTIVE", "PAUSED", "COMPLETED", "CANCELLED"):
        return error("Invalid status. Allowed values: ACTIVE, PAUSED, COMPLETED, CANCELLED.")

    db = get_db()
    uid = g.user_id
    cur = db.execute(
        "UPDATE sip_plans SET status = ? WHERE sip_plan_id = ? AND user_id = ?",
        (new_status, plan_id, uid)
    )
    db.commit()
    if cur.rowcount == 0:
        return error("SIP plan not found or not owned by you.", 404)
    return success(message=f"SIP plan marked as {new_status}.")


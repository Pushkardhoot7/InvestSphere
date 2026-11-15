"""
InvestSphere Maturity Tracker Route
GET /api/maturity           — upcoming financial events (FD/Bond maturities, coupons)
PUT /api/maturity/<id>      — mark an event as PROCESSED / CANCELLED
"""

import datetime
from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success, error

maturity_bp = Blueprint("maturity", __name__)


@maturity_bp.route("", methods=["GET"])
@require_auth
def list_maturities():
    db = get_db()
    uid = g.user_id
    status_filter = request.args.get("status", "PENDING")
    days_ahead = int(request.args.get("days", 365))

    q = """
        SELECT fe.event_id, fe.investment_id, fe.event_type, fe.event_date,
               fe.amount, fe.status, fe.description, fe.created_at,
               inst.symbol, inst.instrument_name, it.type_code AS category,
               CAST((julianday(fe.event_date) - julianday(date('now'))) AS INTEGER) AS days_remaining
        FROM financial_events fe
        LEFT JOIN investments i ON fe.investment_id = i.investment_id
        LEFT JOIN instruments inst ON i.instrument_id = inst.instrument_id
        LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
        WHERE fe.user_id = ?
    """
    params = [uid]
    if status_filter and status_filter != "ALL":
        q += " AND fe.status = ?"
        params.append(status_filter)
    if days_ahead > 0:
        q += " AND julianday(fe.event_date) <= julianday(date('now')) + ?"
        params.append(days_ahead)
    q += " ORDER BY fe.event_date ASC"

    rows = db.execute(q, params).fetchall()

    # Enrich FD and Bond details
    result = []
    total_amount_cents = 0
    due_30_cents = 0
    due_90_cents = 0

    for r in rows:
        item = dict(r)
        amt_cents = item.get("amount") or 0
        total_amount_cents += amt_cents
        item["amount"] = round(amt_cents / 100, 2)
        item["maturity_amount"] = item["amount"]
        item["principal_amount"] = item["amount"]
        item["institution_name"] = item.get("description") or item.get("instrument_name") or "Fixed Deposit"
        item["type_code"] = item.get("category") or "FIXED_DEPOSIT"

        days = item.get("days_remaining") or 0
        if 0 <= days <= 30:
            due_30_cents += amt_cents
        if 0 <= days <= 90:
            due_90_cents += amt_cents

        if r["event_type"] == "FD_MATURITY" and r["investment_id"]:
            fd = db.execute(
                "SELECT principal_amount, annual_interest_rate, maturity_date, institution_name "
                "FROM fixed_deposits WHERE investment_id=?",
                (r["investment_id"],)
            ).fetchone()
            if fd:
                item["fd_details"] = dict(fd)
                if fd["institution_name"]:
                    item["institution_name"] = fd["institution_name"]
                if fd["principal_amount"]:
                    item["principal_amount"] = round(fd["principal_amount"] / 100, 2)
        elif r["event_type"] in ("BOND_MATURITY", "BOND_COUPON") and r["investment_id"]:
            bd = db.execute(
                "SELECT issuer_name, coupon_rate, maturity_date, bond_type "
                "FROM bonds WHERE investment_id=?",
                (r["investment_id"],)
            ).fetchone()
            if bd:
                item["bond_details"] = dict(bd)
                if bd["issuer_name"]:
                    item["institution_name"] = bd["issuer_name"]

        result.append(item)

    # Summary counts
    overdue = sum(1 for r in result if (r.get("days_remaining") or 0) < 0)
    due_30 = sum(1 for r in result if 0 <= (r.get("days_remaining") or 0) <= 30)
    due_90 = sum(1 for r in result if 0 <= (r.get("days_remaining") or 0) <= 90)

    return success({
        "events": result,
        "count": len(result),
        "summary": {
            "overdue": overdue,
            "overdue_count": overdue,
            "due_within_30_days": due_30,
            "due_30_count": due_30,
            "due_within_90_days": due_90,
            "due_90_count": due_90,
            "due_30_amount": round(due_30_cents / 100, 2),
            "due_90_amount": round(due_90_cents / 100, 2),
            "total_maturity_amount": round(total_amount_cents / 100, 2),
        }
    })


@maturity_bp.route("/<int:event_id>", methods=["PUT"])
@require_auth
def update_event(event_id):
    db = get_db()
    row = db.execute(
        "SELECT event_id FROM financial_events WHERE event_id=? AND user_id=?",
        (event_id, g.user_id)
    ).fetchone()
    if not row:
        return error("Event not found.", 404)

    data = request.get_json(silent=True) or {}
    new_status = data.get("status", "PROCESSED")
    if new_status not in ("PROCESSED", "CANCELLED", "PENDING", "OVERDUE"):
        return error("Invalid status value.")

    db.execute(
        "UPDATE financial_events SET status=? WHERE event_id=? AND user_id=?",
        (new_status, event_id, g.user_id)
    )
    db.commit()
    return success(message=f"Event status updated to {new_status}.")


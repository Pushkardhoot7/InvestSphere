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

    # Auto-detect and sync any portfolio FDs and Bonds that don't have events yet
    _auto_sync_maturity_events(db, uid)

    status_filter = request.args.get("status", "ALL")
    days_ahead = int(request.args.get("days", 3650))

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
        item["maturity_date"] = item.get("event_date")
        item["institution_name"] = item.get("description") or item.get("instrument_name") or "Fixed Deposit"
        item["type_code"] = item.get("category") or ("BOND" if "BOND" in (r["event_type"] or "") else "FIXED_DEPOSIT")

        days = item.get("days_remaining") or 0
        if 0 <= days <= 30:
            due_30_cents += amt_cents
        if 0 <= days <= 90:
            due_90_cents += amt_cents

        # Instrument-specific enrichment and calculations
        if r["event_type"] == "FD_MATURITY" and r["investment_id"]:
            fd = db.execute(
                """SELECT principal_amount, annual_interest_rate, compounding_frequency,
                          start_date, maturity_date, estimated_maturity_amount, institution_name, payout_type
                   FROM fixed_deposits WHERE investment_id=?""",
                (r["investment_id"],)
            ).fetchone()
            if fd:
                item["fd_details"] = dict(fd)
                if fd["institution_name"]:
                    item["institution_name"] = fd["institution_name"]
                if fd["principal_amount"]:
                    item["principal_amount"] = round(fd["principal_amount"] / 100, 2)
                item["interest_rate"] = fd["annual_interest_rate"] or 0.0
                item["compounding_frequency"] = fd["compounding_frequency"] or "QUARTERLY"
                item["start_date"] = fd["start_date"]
                if fd["estimated_maturity_amount"]:
                    item["maturity_amount"] = round(fd["estimated_maturity_amount"] / 100, 2)
                item["expected_returns"] = round(item["maturity_amount"] - item["principal_amount"], 2)

        elif r["event_type"] in ("BOND_MATURITY", "BOND_COUPON") and r["investment_id"]:
            bd = db.execute(
                """SELECT issuer_name, coupon_rate, coupon_frequency, issue_date,
                          maturity_date, bond_type, face_value, credit_rating
                   FROM bonds WHERE investment_id=?""",
                (r["investment_id"],)
            ).fetchone()
            if bd:
                item["bond_details"] = dict(bd)
                if bd["issuer_name"]:
                    item["institution_name"] = bd["issuer_name"]
                item["interest_rate"] = bd["coupon_rate"] or 0.0
                item["coupon_rate"] = bd["coupon_rate"] or 0.0
                item["coupon_frequency"] = bd["coupon_frequency"] or "SEMI_ANNUALLY"
                item["bond_type"] = bd["bond_type"] or "GOVERNMENT"
                item["start_date"] = bd["issue_date"]
                face_rs = round((bd["face_value"] or 0) / 100, 2)
                item["face_value"] = face_rs
                item["expected_returns"] = round(item["maturity_amount"] - item["principal_amount"], 2)

        result.append(item)

    # Summary counts
    overdue = sum(1 for r in result if (r.get("days_remaining") or 0) < 0 and r.get("status") != "PROCESSED")
    due_30 = sum(1 for r in result if 0 <= (r.get("days_remaining") or 0) <= 30 and r.get("status") != "PROCESSED")
    due_90 = sum(1 for r in result if 0 <= (r.get("days_remaining") or 0) <= 90 and r.get("status") != "PROCESSED")
    matured = sum(1 for r in result if (r.get("days_remaining") or 0) < 0 or r.get("status") == "PROCESSED")

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
            "matured_count": matured,
            "due_30_amount": round(due_30_cents / 100, 2),
            "due_90_amount": round(due_90_cents / 100, 2),
            "total_maturity_amount": round(total_amount_cents / 100, 2),
        }
    })


def _auto_sync_maturity_events(db, uid):
    """
    Auto-detect Fixed Deposits and Bonds from the user's active portfolio holdings
    that do not yet have corresponding financial_events records, calculate their
    instrument-specific maturity value, and insert them automatically.
    """
    import datetime

    # 1. Sync Fixed Deposits
    fd_rows = db.execute("""
        SELECT i.investment_id, i.instrument_id,
               fd.principal_amount, fd.annual_interest_rate, fd.compounding_frequency,
               fd.payout_type, fd.start_date, fd.maturity_date, fd.institution_name,
               fd.estimated_maturity_amount
        FROM investments i
        JOIN instruments inst ON i.instrument_id = inst.instrument_id
        JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
        JOIN fixed_deposits fd ON fd.investment_id = i.investment_id
        WHERE i.user_id = ? AND i.status = 'ACTIVE'
          AND it.type_code = 'FIXED_DEPOSIT'
          AND fd.maturity_date IS NOT NULL
    """, (uid,)).fetchall()

    for fd in fd_rows:
        inv_id = fd["investment_id"]
        existing = db.execute(
            "SELECT event_id FROM financial_events WHERE investment_id = ? AND event_type = 'FD_MATURITY' AND user_id = ?",
            (inv_id, uid)
        ).fetchone()
        if existing:
            continue

        mat_date_str = (fd["maturity_date"] or "").strip()[:10]
        if not mat_date_str:
            continue

        principal_cents = fd["principal_amount"] or 0
        rate = fd["annual_interest_rate"] or 0.0
        start_date_str = (fd["start_date"] or "").strip()[:10] or datetime.date.today().isoformat()

        # Calculate estimated maturity amount using compound interest if not pre-computed
        if fd["estimated_maturity_amount"] and fd["estimated_maturity_amount"] > 0:
            maturity_cents = fd["estimated_maturity_amount"]
        else:
            try:
                s_dt = datetime.date.fromisoformat(start_date_str)
                m_dt = datetime.date.fromisoformat(mat_date_str)
                tenure_years = max(0.08, (m_dt - s_dt).days / 365.25)
            except Exception:
                tenure_years = 1.0

            freq_map = {"MONTHLY": 12, "QUARTERLY": 4, "HALF_YEARLY": 2, "YEARLY": 1, "ANNUAL": 1}
            n = freq_map.get((fd["compounding_frequency"] or "QUARTERLY").upper(), 4)
            if rate > 0 and tenure_years > 0:
                p_val = principal_cents / 100.0
                m_val = p_val * ((1.0 + (rate / 100.0) / n) ** (n * tenure_years))
                maturity_cents = int(round(m_val * 100))
            else:
                maturity_cents = principal_cents

        inst_name = fd["institution_name"] or "Bank Fixed Deposit"
        desc = f"{inst_name} FD Maturity ({rate}% p.a.)"
        now_ts = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

        try:
            db.execute("""
                INSERT INTO financial_events
                   (user_id, investment_id, event_type, event_date, amount, status, description, created_at)
                VALUES (?, ?, 'FD_MATURITY', ?, ?, 'PENDING', ?, ?)
            """, (uid, inv_id, mat_date_str, maturity_cents, desc, now_ts))
        except Exception:
            pass

    # 2. Sync Bonds
    bond_rows = db.execute("""
        SELECT i.investment_id, i.instrument_id,
               b.face_value, b.coupon_rate, b.coupon_frequency,
               b.issue_date, b.maturity_date, b.issuer_name, b.bond_type
        FROM investments i
        JOIN instruments inst ON i.instrument_id = inst.instrument_id
        JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
        JOIN bonds b ON b.investment_id = i.investment_id
        WHERE i.user_id = ? AND i.status = 'ACTIVE'
          AND it.type_code = 'BOND'
          AND b.maturity_date IS NOT NULL
    """, (uid,)).fetchall()

    for bond in bond_rows:
        inv_id = bond["investment_id"]
        existing = db.execute(
            "SELECT event_id FROM financial_events WHERE investment_id = ? AND event_type = 'BOND_MATURITY' AND user_id = ?",
            (inv_id, uid)
        ).fetchone()
        if existing:
            continue

        mat_date_str = (bond["maturity_date"] or "").strip()[:10]
        if not mat_date_str:
            continue

        # Get total quantity owned from transactions
        qty_row = db.execute(
            "SELECT COALESCE(SUM(CASE WHEN transaction_type = 'BUY' THEN quantity ELSE -quantity END), 1) FROM transactions WHERE investment_id = ?",
            (inv_id,)
        ).fetchone()
        qty = float(qty_row[0]) if (qty_row and qty_row[0]) else 1.0
        face_val = bond["face_value"] or 100000  # cents

        redemption_cents = int(round(face_val * qty))
        issuer = bond["issuer_name"] or "Government/Corporate Bond"
        coupon = bond["coupon_rate"] or 0.0
        desc = f"{issuer} Bond Redemption ({coupon}% Coupon)"
        now_ts = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

        try:
            db.execute("""
                INSERT INTO financial_events
                   (user_id, investment_id, event_type, event_date, amount, status, description, created_at)
                VALUES (?, ?, 'BOND_MATURITY', ?, ?, 'PENDING', ?, ?)
            """, (uid, inv_id, mat_date_str, redemption_cents, desc, now_ts))
        except Exception:
            pass

    try:
        db.commit()
    except Exception:
        pass


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


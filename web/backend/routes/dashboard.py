"""
InvestSphere Dashboard Route
GET /api/dashboard
Returns portfolio summary + recent activity for the authenticated user.
"""

from flask import Blueprint, g
from app import get_db
from helpers import require_auth, success
import datetime

dashboard_bp = Blueprint("dashboard", __name__)


@dashboard_bp.route("", methods=["GET"])
@require_auth
def get_dashboard():
    db = get_db()
    uid = g.user_id

    # --- Portfolio totals from vw_active_holdings view ---
    holdings = db.execute(
        """SELECT asset_type, symbol, instrument_name,
                  remaining_quantity, net_invested_amount, holding_status
           FROM vw_active_holdings
           WHERE user_id = ? AND holding_status = 'ACTIVE'""",
        (uid,)
    ).fetchall()

    total_invested = 0
    total_current = 0
    category_map = {}

    for h in holdings:
        cost = h["net_invested_amount"] or 0
        total_invested += cost
        cat = h["asset_type"]
        if cat not in category_map:
            category_map[cat] = {"invested": 0, "current": 0, "count": 0}
        category_map[cat]["invested"] += cost
        category_map[cat]["count"] += 1
        # Current value = invested (no live price available without market data)
        category_map[cat]["current"] += cost
        total_current += cost

    # Latest market prices to refine current values
    prices = db.execute(
        "SELECT instrument_id, latest_price FROM vw_latest_market_prices"
    ).fetchall()
    price_map = {r["instrument_id"]: r["latest_price"] for r in prices}

    navs = db.execute(
        "SELECT instrument_id, latest_nav FROM vw_latest_mutual_fund_nav"
    ).fetchall()
    nav_map = {r["instrument_id"]: r["latest_nav"] for r in navs}

    # Recalculate with live prices
    investments_with_prices = db.execute(
        """SELECT i.investment_id, i.instrument_id, i.status,
                  inst.symbol, it.type_code,
                  COALESCE(ah.remaining_quantity, 0) AS qty,
                  COALESCE(ah.net_invested_amount, 0) AS cost
           FROM investments i
           JOIN instruments inst ON i.instrument_id = inst.instrument_id
           JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           LEFT JOIN vw_active_holdings ah ON ah.investment_id = i.investment_id
           WHERE i.user_id = ? AND i.status = 'ACTIVE'""",
        (uid,)
    ).fetchall()

    total_current_refined = 0
    for inv in investments_with_prices:
        inst_id = inv["instrument_id"]
        qty = inv["qty"] or 0
        cost = inv["cost"] or 0
        tc = inv["type_code"]

        if tc in ("STOCK", "GOLD") and inst_id in price_map and qty > 0:
            cur_val = int(qty * price_map[inst_id])
        elif tc == "MUTUAL_FUND" and inst_id in nav_map and qty > 0:
            cur_val = int(qty * nav_map[inst_id] * 100)
        elif tc == "FIXED_DEPOSIT":
            fd = db.execute(
                "SELECT estimated_maturity_amount FROM fixed_deposits WHERE investment_id=?",
                (inv["investment_id"],)
            ).fetchone()
            cur_val = fd["estimated_maturity_amount"] if fd else cost
        elif tc == "BOND":
            bd = db.execute(
                "SELECT face_value FROM bonds WHERE investment_id=?",
                (inv["investment_id"],)
            ).fetchone()
            cur_val = int(qty * bd["face_value"]) if bd else cost
        else:
            cur_val = cost

        total_current_refined += cur_val

    total_pnl = total_current_refined - total_invested
    pnl_pct = (total_pnl / total_invested * 100) if total_invested > 0 else 0.0

    # Active holdings count
    active_count = db.execute(
        "SELECT COUNT(*) FROM investments WHERE user_id=? AND status='ACTIVE'", (uid,)
    ).fetchone()[0]

    # Recent transactions (last 10)
    recent_tx = db.execute(
        """SELECT t.transaction_id, t.transaction_type, t.transaction_date,
                  t.net_amount, t.quantity, inst.symbol, inst.instrument_name
           FROM transactions t
           LEFT JOIN investments i ON t.investment_id = i.investment_id
           LEFT JOIN instruments inst ON i.instrument_id = inst.instrument_id
           WHERE t.user_id = ?
           ORDER BY t.transaction_date DESC, t.transaction_id DESC
           LIMIT 10""",
        (uid,)
    ).fetchall()
    recent_tx_list = []
    for r in recent_tx:
        d = dict(r)
        d["net_amount"] = round((d["net_amount"] or 0) / 100, 2)
        recent_tx_list.append(d)

    # Upcoming maturities (next 90 days)
    upcoming = db.execute(
        """SELECT fe.event_id, fe.event_type, fe.event_date, fe.amount,
                  fe.description, fe.status,
                  CAST((julianday(fe.event_date) - julianday(date('now'))) AS INTEGER) AS days_remaining
           FROM financial_events fe
           WHERE fe.user_id = ? AND fe.status = 'PENDING'
             AND julianday(fe.event_date) >= julianday(date('now'))
             AND julianday(fe.event_date) <= julianday(date('now')) + 90
           ORDER BY fe.event_date ASC
           LIMIT 5""",
        (uid,)
    ).fetchall()
    upcoming_list = []
    for r in upcoming:
        d = dict(r)
        d["maturity_date"] = d["event_date"]
        d["amount"] = round((d["amount"] or 0) / 100, 2)
        d["maturity_amount"] = d["amount"]
        d["institution_name"] = d["description"]
        upcoming_list.append(d)

    # Category allocation
    allocation = []
    for cat, vals in category_map.items():
        pct = (vals["invested"] / total_invested * 100) if total_invested > 0 else 0
        allocation.append({
            "category": cat,
            "category_name": cat.replace("_", " ").title(),
            "invested_cents": vals["invested"],
            "current_value": round(vals["current"] / 100, 2),
            "percentage": round(pct, 2),
            "count": vals["count"],
            "allocation_percent": round(pct, 2),
        })
    allocation.sort(key=lambda x: x["invested_cents"], reverse=True)

    # SIP plans count
    active_sips = db.execute(
        "SELECT COUNT(*) FROM sip_plans WHERE user_id=? AND status='ACTIVE'", (uid,)
    ).fetchone()[0]

    return success({
        "summary": {
            "total_invested_cents": total_invested,
            "total_invested": round(total_invested / 100, 2),
            "total_current_value_cents": total_current_refined,
            "total_current_value": round(total_current_refined / 100, 2),
            "total_pnl_cents": total_pnl,
            "total_gain_loss": round(total_pnl / 100, 2),
            "total_gain_loss_pct": round(pnl_pct, 2),
            "total_pnl_percent": round(pnl_pct, 2),
            "total_realized_gain": 0.0,
            "active_holdings_count": active_count,
            "active_investments_count": active_count,
            "active_sip_plans": active_sips,
        },
        "allocation": allocation,
        "recent_transactions": recent_tx_list,
        "upcoming_maturities": upcoming_list,
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    })


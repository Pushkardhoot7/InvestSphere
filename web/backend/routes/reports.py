"""
InvestSphere Reports Route
GET /api/reports/holdings      — portfolio holdings CSV export
GET /api/reports/transactions  — transaction ledger CSV export
GET /api/reports/pnl           — P&L summary
"""

import csv
import io
import datetime
from flask import Blueprint, request, g, Response
from app import get_db
from helpers import require_auth, success, error

reports_bp = Blueprint("reports", __name__)


@reports_bp.route("/holdings", methods=["GET"])
@require_auth
def holdings_report():
    db = get_db()
    uid = g.user_id
    fmt = request.args.get("format", "json")

    rows = db.execute(
        """SELECT i.investment_id, inst.symbol, inst.instrument_name, it.type_code AS category,
                  i.portfolio_tag, i.status,
                  COALESCE(ah.remaining_quantity, 0) AS qty,
                  COALESCE(ah.net_invested_amount, 0) AS cost_cents,
                  i.created_at
           FROM investments i
           JOIN instruments inst ON i.instrument_id = inst.instrument_id
           JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           LEFT JOIN vw_active_holdings ah ON ah.investment_id = i.investment_id
           WHERE i.user_id = ?
           ORDER BY it.type_code, inst.symbol""",
        (uid,)
    ).fetchall()

    data = [dict(r) for r in rows]

    if fmt == "csv":
        output = io.StringIO()
        writer = csv.writer(output)
        writer.writerow([
            "Investment ID", "Symbol", "Name", "Category", "Portfolio Tag",
            "Status", "Quantity", "Invested Amount (₹)", "Added Date"
        ])
        for r in data:
            writer.writerow([
                r["investment_id"], r["symbol"], r["instrument_name"],
                r["category"], r["portfolio_tag"], r["status"],
                r["qty"], f"{r['cost_cents']/100:.2f}", r["created_at"]
            ])
        response = Response(output.getvalue(), mimetype="text/csv")
        fname = f"investsphere_holdings_{datetime.date.today()}.csv"
        response.headers["Content-Disposition"] = f'attachment; filename="{fname}"'
        return response

    for r in data:
        r["current_value"] = round((r.get("cost_cents") or 0) / 100, 2)
        r["name"] = r["instrument_name"]
        r["quantity"] = r["qty"]
        r["cost"] = round((r.get("cost_cents") or 0) / 100, 2)

    return success(data)


@reports_bp.route("/transactions", methods=["GET"])
@require_auth
def transactions_report():
    db = get_db()
    uid = g.user_id
    fmt = request.args.get("format", "json")
    start_date = request.args.get("start_date", "")
    end_date = request.args.get("end_date", "")
    tx_type = request.args.get("type", "")

    q = """SELECT t.transaction_id, t.transaction_date, t.transaction_type,
                  inst.symbol, inst.instrument_name, it.type_code AS category,
                  t.quantity, t.unit_price, t.gross_amount, t.fees, t.taxes,
                  t.net_amount, t.notes
           FROM transactions t
           LEFT JOIN investments i ON t.investment_id = i.investment_id
           LEFT JOIN instruments inst ON i.instrument_id = inst.instrument_id
           LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           WHERE t.user_id = ?"""
    params = [uid]
    if start_date:
        q += " AND t.transaction_date >= ?"
        params.append(start_date)
    if end_date:
        q += " AND t.transaction_date <= ?"
        params.append(end_date + " 23:59:59")
    if tx_type:
        q += " AND t.transaction_type = ?"
        params.append(tx_type)
    q += " ORDER BY t.transaction_date DESC, t.transaction_id DESC"

    rows = db.execute(q, params).fetchall()
    data = [dict(r) for r in rows]

    if fmt == "csv":
        output = io.StringIO()
        writer = csv.writer(output)
        writer.writerow([
            "ID", "Date", "Type", "Symbol", "Name", "Category",
            "Quantity", "Unit Price (₹)", "Gross (₹)", "Fees (₹)",
            "Taxes (₹)", "Net Amount (₹)", "Notes"
        ])
        for r in data:
            def fmt_cents(v): return f"{(v or 0)/100:.2f}"
            writer.writerow([
                r["transaction_id"], r["transaction_date"], r["transaction_type"],
                r.get("symbol", ""), r.get("instrument_name", ""),
                r.get("category", ""), r.get("quantity", ""),
                fmt_cents(r.get("unit_price")), fmt_cents(r.get("gross_amount")),
                fmt_cents(r.get("fees")), fmt_cents(r.get("taxes")),
                fmt_cents(r.get("net_amount")), r.get("notes", "")
            ])
        response = Response(output.getvalue(), mimetype="text/csv")
        fname = f"investsphere_transactions_{datetime.date.today()}.csv"
        response.headers["Content-Disposition"] = f'attachment; filename="{fname}"'
        return response

    for r in data:
        r["date"] = r["transaction_date"]
        r["type"] = r["transaction_type"]
        r["name"] = r.get("instrument_name") or r.get("symbol")
        r["net_amount"] = round((r.get("net_amount") or 0) / 100, 2)
        r["gross_amount"] = round((r.get("gross_amount") or 0) / 100, 2)

    return success(data)


@reports_bp.route("/pnl", methods=["GET"])
@require_auth
def pnl_report():
    db = get_db()
    uid = g.user_id

    # Realized P&L from sell/redemption transactions
    realized_rows = db.execute(
        """SELECT inst.symbol, inst.instrument_name, it.type_code AS category,
                  SUM(CASE WHEN t.transaction_type IN ('SELL','REDEMPTION') THEN t.net_amount ELSE 0 END) AS proceeds,
                  SUM(CASE WHEN t.transaction_type IN ('BUY','DEPOSIT') THEN t.net_amount ELSE 0 END) AS cost,
                  SUM(CASE WHEN t.transaction_type = 'DIVIDEND' THEN t.net_amount ELSE 0 END) AS dividends,
                  SUM(CASE WHEN t.transaction_type = 'INTEREST' THEN t.net_amount ELSE 0 END) AS interest,
                  COUNT(*) AS tx_count
           FROM transactions t
           LEFT JOIN investments i ON t.investment_id = i.investment_id
           LEFT JOIN instruments inst ON i.instrument_id = inst.instrument_id
           LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           WHERE t.user_id = ?
           GROUP BY i.investment_id, inst.symbol, inst.instrument_name, it.type_code
           HAVING proceeds > 0 OR dividends > 0 OR interest > 0
           ORDER BY proceeds DESC""",
        (uid,)
    ).fetchall()

    pnl_items = []
    total_proceeds = 0
    total_realized_pnl = 0
    total_income = 0
    for r in realized_rows:
        pnl = r["proceeds"] - r["cost"]
        total_proceeds += r["proceeds"]
        total_realized_pnl += pnl
        total_income += (r["dividends"] or 0) + (r["interest"] or 0)
        pnl_items.append({
            "symbol": r["symbol"],
            "name": r["instrument_name"],
            "category": r["category"],
            "proceeds_cents": r["proceeds"],
            "cost_cents": r["cost"],
            "realized_pnl_cents": pnl,
            "dividends_cents": r["dividends"] or 0,
            "interest_cents": r["interest"] or 0,
            "transaction_count": r["tx_count"],
        })

    # Active positions with unrealized P&L
    active_rows = db.execute(
        """SELECT i.investment_id, inst.symbol, inst.instrument_name, it.type_code AS category,
                  COALESCE(ah.remaining_quantity, 0) AS qty,
                  COALESCE(ah.net_invested_amount, 0) AS cost_cents
           FROM investments i
           JOIN instruments inst ON i.instrument_id = inst.instrument_id
           JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           LEFT JOIN vw_active_holdings ah ON ah.investment_id = i.investment_id
           WHERE i.user_id = ? AND i.status = 'ACTIVE'""",
        (uid,)
    ).fetchall()

    positions = []
    for ar in active_rows:
        cost = ar["cost_cents"]
        cur_val = cost # Default to cost basis if no live quote
        positions.append({
            "symbol": ar["symbol"],
            "name": ar["instrument_name"],
            "category": ar["category"],
            "cost_basis": round(cost / 100, 2),
            "current_value": round(cur_val / 100, 2),
            "unrealized_gain_loss": 0.0,
            "cost_basis_cents": cost,
            "current_value_cents": cur_val,
        })

    return success({
        "pnl_items": pnl_items,
        "positions": positions,
        "totals": {
            "total_proceeds_cents": total_proceeds,
            "total_realized_pnl_cents": total_realized_pnl,
            "total_income_cents": total_income,
        },
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    })


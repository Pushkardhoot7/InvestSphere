"""
InvestSphere Transactions Routes
Full CRUD for unified transaction ledger
"""

from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success, error
import datetime

transactions_bp = Blueprint("transactions", __name__)

VALID_TX_TYPES = {"BUY", "SELL", "DEPOSIT", "WITHDRAWAL", "DIVIDEND", "INTEREST", "FEE", "REDEMPTION"}


@transactions_bp.route("", methods=["GET"])
@require_auth
def list_transactions():
    db = get_db()
    inv_id = request.args.get("investment_id")
    tx_type = request.args.get("type")
    start_date = request.args.get("start_date")
    end_date = request.args.get("end_date")

    query = """
        SELECT t.*, inst.symbol, inst.instrument_name, it.type_code AS asset_type
        FROM transactions t
        LEFT JOIN investments i ON t.investment_id = i.investment_id
        LEFT JOIN instruments inst ON i.instrument_id = inst.instrument_id
        LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
        WHERE t.user_id = ?
    """
    params = [g.user_id]

    if inv_id:
        # Verify ownership
        row = db.execute(
            "SELECT investment_id FROM investments WHERE investment_id=? AND user_id=?",
            (int(inv_id), g.user_id)
        ).fetchone()
        if not row:
            return error("Investment not found or access denied.", 404)
        query += " AND t.investment_id = ?"
        params.append(int(inv_id))
    if tx_type and tx_type in VALID_TX_TYPES:
        query += " AND t.transaction_type = ?"
        params.append(tx_type)
    if start_date:
        query += " AND t.transaction_date >= ?"
        params.append(start_date)
    if end_date:
        query += " AND t.transaction_date <= ?"
        params.append(end_date + " 23:59:59")

    query += " ORDER BY t.transaction_date DESC, t.transaction_id DESC"
    rows = db.execute(query, params).fetchall()
    results = []
    for r in rows:
        d = dict(r)
        d["tx_type"] = d["transaction_type"]
        d["tx_date"] = d["transaction_date"]
        d["gross_amount_cents"] = d["gross_amount"]
        d["net_amount_cents"] = d["net_amount"]
        d["fees_cents"] = d["fees"]
        d["taxes_cents"] = d["taxes"]
        d["gross_amount"] = round((d["gross_amount"] or 0) / 100, 2)
        d["net_amount"] = round((d["net_amount"] or 0) / 100, 2)
        d["fees"] = round((d["fees"] or 0) / 100, 2)
        d["taxes"] = round((d["taxes"] or 0) / 100, 2)
        if d.get("unit_price"):
            d["unit_price"] = round(d["unit_price"] / 100, 2)
        results.append(d)
    return success(results)


@transactions_bp.route("/<int:tx_id>", methods=["GET"])
@require_auth
def get_transaction(tx_id):
    db = get_db()
    row = db.execute(
        "SELECT * FROM transactions WHERE transaction_id=? AND user_id=?",
        (tx_id, g.user_id)
    ).fetchone()
    if not row:
        return error("Transaction not found.", 404)
    return success(dict(row))


@transactions_bp.route("", methods=["POST"])
@require_auth
def add_transaction():
    data = request.get_json(silent=True) or {}
    err = _validate_tx(data, g.user_id)
    if err:
        return error(err)

    db = get_db()
    inv_id = int(data["investment_id"])
    tx_type = (data.get("transaction_type") or data.get("tx_type") or "").strip().upper()
    tx_date = (data.get("transaction_date") or data.get("tx_date") or "").strip()
    qty = float(data.get("quantity") or 0)

    if "unit_price" in data:
        unit_price = int(round(float(data["unit_price"]) * 100))
    else:
        unit_price = int(data.get("unit_price_cents") or 0)

    if "gross_amount" in data:
        gross = int(round(float(data["gross_amount"]) * 100))
    elif "gross_amount_cents" in data:
        gross = int(data["gross_amount_cents"])
    else:
        gross = int(round(qty * unit_price))

    fees = int(round(float(data["fees"]) * 100)) if "fees" in data else int(data.get("fees_cents") or 0)
    taxes = int(round(float(data["taxes"]) * 100)) if "taxes" in data else int(data.get("taxes_cents") or 0)

    if "net_amount" in data:
        net = int(round(float(data["net_amount"]) * 100))
    elif "net_amount_cents" in data:
        net = int(data["net_amount_cents"])
    else:
        if tx_type in ("BUY", "DEPOSIT"):
            net = gross + fees + taxes
        else:
            net = max(0, gross - fees - taxes)

    notes = (data.get("notes") or "").strip()

    try:
        cur = db.execute(
            """INSERT INTO transactions
               (user_id, investment_id, transaction_type, transaction_date,
                quantity, unit_price, gross_amount, fees, taxes, net_amount, notes)
               VALUES (?,?,?,?,?,?,?,?,?,?,?)""",
            (g.user_id, inv_id, tx_type, tx_date, qty, unit_price, gross, fees, taxes, net, notes)
        )
        db.commit()
        tx_id = cur.lastrowid
        return success({"transaction_id": tx_id}, "Transaction recorded successfully.", 201)
    except Exception as e:
        db.rollback()
        if "Integrity Error" in str(e) or "ABORT" in str(e):
            return error(str(e), 409)
        return error(f"Failed to record transaction: {str(e)}", 500)


@transactions_bp.route("/<int:tx_id>", methods=["DELETE"])
@require_auth
def delete_transaction(tx_id):
    db = get_db()
    row = db.execute(
        "SELECT transaction_id FROM transactions WHERE transaction_id=? AND user_id=?",
        (tx_id, g.user_id)
    ).fetchone()
    if not row:
        return error("Transaction not found.", 404)

    try:
        db.execute("DELETE FROM transactions WHERE transaction_id=? AND user_id=?", (tx_id, g.user_id))
        db.commit()
        return success(message="Transaction deleted.")
    except Exception as e:
        db.rollback()
        return error(str(e), 500)


def _validate_tx(data, user_id) -> str | None:
    inv_id = data.get("investment_id")
    if not inv_id:
        return "investment_id is required."
    try:
        inv_id = int(inv_id)
    except (ValueError, TypeError):
        return "investment_id must be an integer."

    # Verify ownership
    db = get_db()
    row = db.execute(
        "SELECT investment_id FROM investments WHERE investment_id=? AND user_id=?",
        (inv_id, user_id)
    ).fetchone()
    if not row:
        return "Investment not found or access denied."

    tx_type = (data.get("transaction_type") or data.get("tx_type") or "").strip().upper()
    if tx_type not in VALID_TX_TYPES:
        return f"Invalid transaction type. Must be one of: {', '.join(VALID_TX_TYPES)}"

    tx_date = data.get("transaction_date") or data.get("tx_date")
    if not tx_date:
        return "Transaction date is required."

    gross = float(data.get("gross_amount") or data.get("gross_amount_cents") or 0)
    if gross < 0:
        return "Gross amount cannot be negative."

    return None


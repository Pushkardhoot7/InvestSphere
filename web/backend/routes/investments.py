"""
InvestSphere Investments Routes
Full CRUD for multi-asset investment holdings
"""

from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success, error
import datetime
import math

investments_bp = Blueprint("investments", __name__)

VALID_CATEGORIES = {"STOCK", "MUTUAL_FUND", "GOLD", "FIXED_DEPOSIT", "BOND", "CASH"}
TYPE_ID_MAP = {
    "STOCK": 1, "MUTUAL_FUND": 2, "GOLD": 3,
    "FIXED_DEPOSIT": 4, "BOND": 5, "CASH": 6
}
CATEGORY_NAMES = {
    "STOCK": "Equity Stocks", "MUTUAL_FUND": "Mutual Funds",
    "GOLD": "Gold & Precious Metals", "FIXED_DEPOSIT": "Fixed Deposits",
    "BOND": "Bonds & Debentures", "CASH": "Cash & Savings"
}


@investments_bp.route("", methods=["GET"])
@require_auth
def list_investments():
    db = get_db()
    category = request.args.get("category", "ALL")
    status_filter = request.args.get("status", "ACTIVE")
    search = (request.args.get("search") or "").strip().lower()

    # Base query - user-scoped
    query = """
        SELECT i.investment_id, i.instrument_id, i.portfolio_tag, i.notes,
               i.status, i.created_at,
               inst.symbol, inst.instrument_name, inst.currency,
               it.type_code AS asset_type_code
        FROM investments i
        JOIN instruments inst ON i.instrument_id = inst.instrument_id
        JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
        WHERE i.user_id = ?
    """
    params = [g.user_id]

    if status_filter and status_filter != "ALL":
        query += " AND i.status = ?"
        params.append(status_filter)
    if category and category != "ALL" and category in VALID_CATEGORIES:
        query += " AND it.type_code = ?"
        params.append(category)

    query += " ORDER BY i.created_at DESC"
    rows = db.execute(query, params).fetchall()

    results = []
    for row in rows:
        inv = dict(row)
        inv_id = inv["investment_id"]
        inst_id = inv["instrument_id"]

        # Apply search filter
        if search:
            haystack = (inv["symbol"] + " " + inv["instrument_name"] + " " + (inv["notes"] or "")).lower()
            if search not in haystack:
                continue

        # Cost basis from transactions
        txs = db.execute(
            "SELECT transaction_type, quantity, net_amount FROM transactions WHERE investment_id = ?",
            (inv_id,)
        ).fetchall()
        qty, cost = _compute_cost_basis_simple(txs)

        item = {
            "investment_id": inv_id,
            "instrument_id": inst_id,
            "symbol": inv["symbol"],
            "name": inv["instrument_name"],
            "category_code": inv["asset_type_code"],
            "type_code": inv["asset_type_code"],
            "category_name": CATEGORY_NAMES.get(inv["asset_type_code"], inv["asset_type_code"]),
            "status": inv["status"],
            "portfolio_tag": inv["portfolio_tag"],
            "notes": inv["notes"] or "",
            "quantity": qty,
            "invested_amount_cents": cost,
            "total_invested": round(cost / 100, 2),
            "currency": inv["currency"],
            "created_at": inv["created_at"],
        }

        # Valuation
        val = _get_valuation(db, inv["asset_type_code"], inv_id, inst_id, qty, cost, symbol=inv["symbol"])
        item.update(val)
        if not item.get("current_value"):
            item["current_value"] = item["total_invested"]
        if "unrealized_gain_loss" not in item:
            item["unrealized_gain_loss"] = round(item.get("unrealized_pnl_cents", 0) / 100, 2)
        results.append(item)

    return success(results)


@investments_bp.route("/<int:inv_id>", methods=["GET"])
@require_auth
def get_investment(inv_id):
    db = get_db()
    row = db.execute(
        """SELECT i.investment_id, i.instrument_id, i.portfolio_tag, i.notes, i.status, i.created_at,
                  inst.symbol, inst.instrument_name, inst.currency, inst.exchange,
                  it.type_code AS asset_type_code
           FROM investments i
           JOIN instruments inst ON i.instrument_id = inst.instrument_id
           JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
           WHERE i.investment_id = ? AND i.user_id = ?""",
        (inv_id, g.user_id)
    ).fetchone()
    if not row:
        return error("Investment not found or access denied.", 404)

    inv = dict(row)
    txs_rows = db.execute(
        "SELECT * FROM transactions WHERE investment_id = ? ORDER BY transaction_date",
        (inv_id,)
    ).fetchall()
    txs = [dict(t) for t in txs_rows]
    qty, cost = _compute_cost_basis_full(txs)

    detail = {
        "investment_id": inv_id,
        "instrument_id": inv["instrument_id"],
        "symbol": inv["symbol"],
        "name": inv["instrument_name"],
        "exchange": inv["exchange"] or "",
        "currency": inv["currency"],
        "category_code": inv["asset_type_code"],
        "category_name": CATEGORY_NAMES.get(inv["asset_type_code"], ""),
        "status": inv["status"],
        "portfolio_tag": inv["portfolio_tag"],
        "notes": inv["notes"] or "",
        "quantity": qty,
        "invested_amount_cents": cost,
        "created_at": inv["created_at"],
        "transactions": txs,
    }

    # Specializations
    cat = inv["asset_type_code"]
    if cat in ("STOCK",):
        sd = db.execute("SELECT * FROM stocks WHERE instrument_id = ?", (inv["instrument_id"],)).fetchone()
        if sd:
            detail["stock_details"] = dict(sd)
    elif cat == "MUTUAL_FUND":
        mfd = db.execute("SELECT * FROM mutual_funds WHERE instrument_id = ?", (inv["instrument_id"],)).fetchone()
        if mfd:
            detail["mutual_fund_details"] = dict(mfd)
    elif cat == "GOLD":
        gd = db.execute("SELECT * FROM gold_holdings WHERE investment_id = ?", (inv_id,)).fetchone()
        if gd:
            detail["gold_details"] = dict(gd)
    elif cat == "FIXED_DEPOSIT":
        fd = db.execute("SELECT * FROM fixed_deposits WHERE investment_id = ?", (inv_id,)).fetchone()
        if fd:
            detail["fd_details"] = dict(fd)
    elif cat == "BOND":
        bd = db.execute("SELECT * FROM bonds WHERE investment_id = ?", (inv_id,)).fetchone()
        if bd:
            detail["bond_details"] = dict(bd)

    detail.update(_get_valuation(db, cat, inv_id, inv["instrument_id"], qty, cost, symbol=inv["symbol"]))
    return success(detail)


@investments_bp.route("", methods=["POST"])
@require_auth
def add_investment():
    data = request.get_json(silent=True) or {}
    err = _validate_investment(data)
    if err:
        return error(err)

    db = get_db()
    cat = (data.get("category_code") or data.get("category") or "").strip().upper()
    symbol = (data.get("symbol") or "").strip().upper()
    inst_name = (data.get("instrument_name") or data.get("name") or "").strip()
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

    try:
        # 1. Resolve or create instrument
        existing = db.execute(
            "SELECT instrument_id FROM instruments WHERE symbol = ? COLLATE NOCASE", (symbol,)
        ).fetchone()

        if existing:
            inst_id = existing["instrument_id"]
        else:
            cur = db.execute(
                "INSERT INTO instruments (investment_type_id, symbol, instrument_name, currency, exchange, is_active, created_at) "
                "VALUES (?, ?, ?, ?, ?, 1, ?)",
                (TYPE_ID_MAP[cat], symbol, inst_name, data.get("currency", "INR"),
                 data.get("exchange", ""), now)
            )
            inst_id = cur.lastrowid

            # Specialization tables
            if cat == "STOCK":
                db.execute(
                    "INSERT OR IGNORE INTO stocks (instrument_id, isin, sector, industry, market_cap_category) VALUES (?,?,?,?,?)",
                    (inst_id, data.get("isin", ""), data.get("sector", ""),
                     data.get("industry", ""), data.get("market_cap_category", "LARGE_CAP"))
                )
            elif cat == "MUTUAL_FUND":
                db.execute(
                    "INSERT OR IGNORE INTO mutual_funds (instrument_id, amc_name, fund_category, plan_type, expense_ratio) VALUES (?,?,?,?,?)",
                    (inst_id, data.get("amc_name", ""), data.get("fund_category", "EQUITY"),
                     data.get("plan_type", "GROWTH"), data.get("expense_ratio", 0.0))
                )

        # 2. Resolve or create investment holding
        p_tag = (data.get("portfolio_tag") or "DEFAULT").strip() or "DEFAULT"
        existing_inv = db.execute(
            "SELECT investment_id, status FROM investments WHERE user_id = ? AND instrument_id = ? AND portfolio_tag = ?",
            (g.user_id, inst_id, p_tag)
        ).fetchone()

        if existing_inv:
            inv_id = existing_inv["investment_id"]
            if existing_inv["status"] != "ACTIVE":
                db.execute("UPDATE investments SET status = 'ACTIVE', updated_at = ? WHERE investment_id = ?", (now, inv_id))
        else:
            cur = db.execute(
                "INSERT INTO investments (user_id, instrument_id, portfolio_tag, notes, status, created_at, updated_at) "
                "VALUES (?, ?, ?, ?, 'ACTIVE', ?, ?)",
                (g.user_id, inst_id, p_tag, data.get("notes", ""), now, now)
            )
            inv_id = cur.lastrowid

        # 3. Holding specializations
        if cat == "GOLD":
            db.execute(
                "INSERT OR REPLACE INTO gold_holdings (investment_id, gold_form, purity_karat, storage_location) VALUES (?,?,?,?)",
                (inv_id, data.get("gold_form", "PHYSICAL_COIN"),
                 float(data.get("purity_karat", 24)), data.get("storage_location", ""))
            )
        elif cat == "FIXED_DEPOSIT":
            institution = (data.get("institution_name") or data.get("institution") or "").strip()
            principal = int(round(float(data.get("principal") or data.get("principal_amount")) * 100)) if ("principal" in data or "principal_amount" in data) else int(data.get("principal_amount_cents", 0))
            int_rate = float(data.get("annual_interest_rate") or data.get("interest_rate") or 0.0)
            comp_freq = data.get("compounding_frequency", "QUARTERLY")
            payout = data.get("payout_type", "CUMULATIVE")
            mat_date = data.get("maturity_date") or (datetime.date.today() + datetime.timedelta(days=365)).isoformat()

            # Estimate maturity amount if not given
            if "estimated_maturity_amount" in data and float(data["estimated_maturity_amount"]) > 0:
                maturity_amt = int(round(float(data["estimated_maturity_amount"]) * 100))
            elif "estimated_maturity_amount_cents" in data and int(data["estimated_maturity_amount_cents"]) > 0:
                maturity_amt = int(data["estimated_maturity_amount_cents"])
            else:
                # 1-year default tenure estimate for compound interest
                rate_decimal = int_rate / 100.0
                n = 4 if comp_freq == "QUARTERLY" else (12 if comp_freq == "MONTHLY" else 1)
                maturity_amt = int(round(principal * math.pow(1 + rate_decimal / n, n * 1)))

            db.execute(
                """INSERT OR REPLACE INTO fixed_deposits
                   (investment_id, institution_name, account_number, principal_amount,
                    annual_interest_rate, compounding_frequency, payout_type,
                    start_date, maturity_date, estimated_maturity_amount, status)
                   VALUES (?,?,?,?,?,?,?,?,?,?,?)""",
                (inv_id, institution, data.get("account_number", f"FD-{symbol}"),
                 principal, int_rate, comp_freq, payout,
                 data.get("start_date", now[:10]), mat_date,
                 maturity_amt, "ACTIVE")
            )
            # Register maturity event
            if mat_date:
                _register_event(db, g.user_id, inv_id, "FD_MATURITY",
                                mat_date, maturity_amt,
                                f"{institution} Fixed Deposit Maturity")
        elif cat == "BOND":
            face_val_raw = float(data.get("face_value") or data.get("unit_price") or data.get("price") or 1000)
            face_val = int(round(face_val_raw * 100)) if face_val_raw > 0 else int(data.get("face_value_cents", 100000))
            qty = float(data.get("quantity") or data.get("qty") or 1)
            mat_date = data.get("maturity_date") or (datetime.date.today() + datetime.timedelta(days=365*5)).isoformat()
            db.execute(
                """INSERT OR REPLACE INTO bonds
                   (investment_id, issuer_name, bond_type, face_value, coupon_rate,
                    coupon_frequency, issue_date, maturity_date, credit_rating, status)
                   VALUES (?,?,?,?,?,?,?,?,?,?)""",
                (inv_id, data.get("issuer_name", inst_name), data.get("bond_type", "GOVERNMENT"),
                 face_val, float(data.get("coupon_rate", 0)),
                 data.get("coupon_frequency", "SEMI_ANNUALLY"),
                 data.get("issue_date", now[:10]), mat_date,
                 data.get("credit_rating", "AAA"), "ACTIVE")
            )
            _register_event(db, g.user_id, inv_id, "BOND_MATURITY",
                            mat_date, int(face_val * qty),
                            f"{inst_name} Bond Redemption/Maturity")

        # 4. Record initial purchase transaction
        purchase_date = (data.get("purchase_date") or data.get("date") or now[:10]).strip()
        if cat == "FIXED_DEPOSIT":
            principal = int(round(float(data.get("principal") or data.get("principal_amount")) * 100)) if ("principal" in data or "principal_amount" in data) else int(data.get("principal_amount_cents", 0))
            inst_label = (data.get("institution_name") or data.get("institution") or inst_name)
            db.execute(
                """INSERT INTO transactions
                   (user_id, investment_id, transaction_type, transaction_date,
                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes)
                   VALUES (?,?,?,?,?,?,?,?,?,?,?)""",
                (g.user_id, inv_id, "DEPOSIT", purchase_date, 1.0, principal,
                 principal, 0, 0, principal, f"Initial FD deposit: {inst_label}")
            )
        else:
            qty = float(data.get("quantity") or data.get("qty") or 0)
            if "unit_price" in data:
                unit_price = int(round(float(data["unit_price"]) * 100))
            else:
                unit_price = int(data.get("unit_price_cents", 0))

            if "total_amount" in data:
                gross = int(round(float(data["total_amount"]) * 100))
            elif "total_amount_cents" in data:
                gross = int(data["total_amount_cents"])
            else:
                gross = int(round(qty * unit_price))

            db.execute(
                """INSERT INTO transactions
                   (user_id, investment_id, transaction_type, transaction_date,
                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes)
                   VALUES (?,?,?,?,?,?,?,?,?,?,?)""",
                (g.user_id, inv_id, "BUY", purchase_date, qty, unit_price,
                 gross, 0, 0, gross, "Initial purchase acquisition")
            )

        db.commit()
        return success({"investment_id": inv_id}, "Investment added successfully.", 201)

    except Exception as e:
        db.rollback()
        return error(f"Failed to add investment: {str(e)}", 500)


@investments_bp.route("/<int:inv_id>", methods=["PUT"])
@require_auth
def update_investment(inv_id):
    # Verify ownership
    row = db_check_ownership(inv_id)
    if row is None:
        return error("Investment not found or access denied.", 404)

    data = request.get_json(silent=True) or {}
    db = get_db()

    notes = data.get("notes", row["notes"] or "")
    status = data.get("status", row["status"])
    portfolio_tag = data.get("portfolio_tag", row["portfolio_tag"])

    if status not in ("ACTIVE", "CLOSED", "WATCHLIST"):
        return error("Invalid status value.")

    db.execute(
        "UPDATE investments SET notes=?, status=?, portfolio_tag=? WHERE investment_id=? AND user_id=?",
        (notes, status, portfolio_tag, inv_id, g.user_id)
    )
    db.commit()
    return success(message="Investment updated successfully.")


@investments_bp.route("/<int:inv_id>", methods=["DELETE"])
@require_auth
def delete_investment(inv_id):
    row = db_check_ownership(inv_id)
    if row is None:
        return error("Investment not found or access denied.", 404)

    db = get_db()
    try:
        # Cleanly delete dependent records first to avoid FOREIGN KEY RESTRICT constraint violation
        db.execute("DELETE FROM transactions WHERE investment_id = ?", (inv_id,))
        db.execute("DELETE FROM gold_holdings WHERE investment_id = ?", (inv_id,))
        db.execute("DELETE FROM fixed_deposits WHERE investment_id = ?", (inv_id,))
        db.execute("DELETE FROM bonds WHERE investment_id = ?", (inv_id,))
        db.execute("DELETE FROM financial_events WHERE investment_id = ?", (inv_id,))
        db.execute("DELETE FROM investments WHERE investment_id = ? AND user_id = ?", (inv_id, g.user_id))
        db.commit()
        return success(message="Investment deleted successfully.")
    except Exception as e:
        db.rollback()
        return error(f"Delete failed: {str(e)}", 500)


# ─────────────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────────────

def db_check_ownership(inv_id):
    db = get_db()
    return db.execute(
        "SELECT investment_id, notes, status, portfolio_tag FROM investments WHERE investment_id=? AND user_id=?",
        (inv_id, g.user_id)
    ).fetchone()


def _compute_cost_basis_simple(txs):
    return _compute_cost_basis_full(txs)


def _compute_cost_basis_full(txs):
    qty = 0.0
    cost = 0
    for tx in txs:
        if tx["transaction_type"] in ("BUY", "DEPOSIT"):
            qty += tx["quantity"] or 0
            cost += tx["net_amount"] or 0
        elif tx["transaction_type"] in ("SELL", "REDEMPTION"):
            sell_qty = tx["quantity"] or 0
            if qty > 0:
                avg_cost = cost / qty
                qty -= sell_qty
                cost -= int(round(sell_qty * avg_cost))
                if qty <= 0:
                    qty = 0
                    cost = 0
    return round(qty, 6), max(0, cost)


def _get_valuation(db, cat, inv_id, inst_id, qty, cost, symbol=""):
    result = {
        "has_market_price": False,
        "latest_price_cents": 0,
        "latest_price": 0.0,
        "has_current_value": False,
        "current_value_cents": cost,
        "current_value": round(cost / 100, 2),
        "has_pnl": False,
        "unrealized_pnl_cents": 0,
        "unrealized_gain_loss": 0.0,
        "unrealized_pnl_percent": 0.0,
        "day_change": 0.0,
        "day_change_pct": 0.0,
    }
    if cat == "STOCK":
        # 1. Real-time market valuation via live quote
        if symbol:
            try:
                from services.market_data_service import get_stock_quote
                quote = get_stock_quote(symbol)
                if quote and quote.get("price") and quote["price"] > 0:
                    live_p = float(quote["price"])
                    price_cents = int(round(live_p * 100))
                    cur_val = int(round(qty * price_cents))
                    result["has_market_price"] = True
                    result["latest_price_cents"] = price_cents
                    result["latest_price"] = round(live_p, 2)
                    result["has_current_value"] = True
                    result["current_value_cents"] = cur_val
                    result["current_value"] = round(cur_val / 100, 2)
                    result["has_pnl"] = True
                    result["unrealized_pnl_cents"] = cur_val - cost
                    result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
                    result["unrealized_pnl_percent"] = (
                        round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
                    )
                    result["day_change"] = quote.get("change", 0.0)
                    result["day_change_pct"] = quote.get("change_percent", 0.0)
                    result["live_source"] = quote.get("data_source", "Yahoo Finance")
                    return result
            except Exception:
                pass

        # Fallback to market_prices table
        price_row = db.execute(
            "SELECT price FROM market_prices WHERE instrument_id=? ORDER BY observation_timestamp DESC LIMIT 1",
            (inst_id,)
        ).fetchone()
        if price_row and price_row[0] > 0 and qty > 0:
            price = price_row[0]
            cur_val = int(round(qty * price))
            result["has_market_price"] = True
            result["latest_price_cents"] = price
            result["latest_price"] = round(price / 100, 2)
            result["has_current_value"] = True
            result["current_value_cents"] = cur_val
            result["current_value"] = round(cur_val / 100, 2)
            result["has_pnl"] = True
            result["unrealized_pnl_cents"] = cur_val - cost
            result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
            result["unrealized_pnl_percent"] = (
                round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
            )

    elif cat == "GOLD":
        try:
            from services.precious_metals_service import get_oropocket_rates
            rates = get_oropocket_rates()
            gold_p = rates.get("gold", {}).get("sell_inr_per_gram", 0)
            if gold_p > 0 and qty > 0:
                cur_val = int(round(qty * gold_p * 100))
                result["has_market_price"] = True
                result["latest_price_cents"] = int(round(gold_p * 100))
                result["latest_price"] = round(gold_p, 2)
                result["has_current_value"] = True
                result["current_value_cents"] = cur_val
                result["current_value"] = round(cur_val / 100, 2)
                result["has_pnl"] = True
                result["unrealized_pnl_cents"] = cur_val - cost
                result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
                result["unrealized_pnl_percent"] = (
                    round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
                )
                return result
        except Exception:
            pass

        price_row = db.execute(
            "SELECT price FROM market_prices WHERE instrument_id=? ORDER BY observation_timestamp DESC LIMIT 1",
            (inst_id,)
        ).fetchone()
        if price_row and price_row[0] > 0 and qty > 0:
            price = price_row[0]
            cur_val = int(round(qty * price))
            result["has_market_price"] = True
            result["latest_price_cents"] = price
            result["latest_price"] = round(price / 100, 2)
            result["has_current_value"] = True
            result["current_value_cents"] = cur_val
            result["current_value"] = round(cur_val / 100, 2)
            result["has_pnl"] = True
            result["unrealized_pnl_cents"] = cur_val - cost
            result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
            result["unrealized_pnl_percent"] = (
                round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
            )
    elif cat == "MUTUAL_FUND":
        nav_row = db.execute(
            "SELECT nav_value FROM mutual_fund_nav WHERE instrument_id=? ORDER BY observation_date DESC LIMIT 1",
            (inst_id,)
        ).fetchone()
        if nav_row and nav_row[0] > 0 and qty > 0:
            nav = nav_row[0]
            cur_val = int(round(qty * nav * 100))
            result["has_market_price"] = True
            result["latest_price_cents"] = int(nav * 100)
            result["has_current_value"] = True
            result["current_value_cents"] = cur_val
            result["has_pnl"] = True
            result["unrealized_pnl_cents"] = cur_val - cost
            result["unrealized_pnl_percent"] = (
                ((cur_val - cost) / cost * 100) if cost > 0 else 0.0
            )
    elif cat == "FIXED_DEPOSIT":
        fd = db.execute("SELECT principal_amount, estimated_maturity_amount FROM fixed_deposits WHERE investment_id=?", (inv_id,)).fetchone()
        if fd:
            cur_val = fd["estimated_maturity_amount"] or fd["principal_amount"]
            p_cents = fd["principal_amount"]
            result["has_market_price"] = True
            result["latest_price_cents"] = p_cents
            result["latest_price"] = round(p_cents / 100, 2)
            result["has_current_value"] = True
            result["current_value_cents"] = cur_val
            result["current_value"] = round(cur_val / 100, 2)
            result["has_pnl"] = True
            result["unrealized_pnl_cents"] = cur_val - cost
            result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
            result["unrealized_pnl_percent"] = (
                round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
            )
    elif cat == "BOND":
        bd = db.execute("SELECT face_value FROM bonds WHERE investment_id=?", (inv_id,)).fetchone()
        face_p = bd["face_value"] if (bd and bd["face_value"]) else cost
        if qty > 0:
            cur_val = int(round(face_p * qty))
            result["has_market_price"] = True
            result["latest_price_cents"] = face_p
            result["latest_price"] = round(face_p / 100, 2)
            result["has_current_value"] = True
            result["current_value_cents"] = cur_val
            result["current_value"] = round(cur_val / 100, 2)
            result["has_pnl"] = True
            result["unrealized_pnl_cents"] = cur_val - cost
            result["unrealized_gain_loss"] = round((cur_val - cost) / 100, 2)
            result["unrealized_pnl_percent"] = (
                round(((cur_val - cost) / cost * 100), 2) if cost > 0 else 0.0
            )
    return result


def _validate_investment(data) -> str | None:
    cat = (data.get("category_code") or data.get("category") or "").strip().upper()
    if cat not in VALID_CATEGORIES:
        return f"Invalid investment category. Must be one of: {', '.join(VALID_CATEGORIES)}"
    name = (data.get("instrument_name") or data.get("name") or "").strip()
    if not name:
        return "Investment name cannot be blank."
    if not (data.get("symbol") or "").strip():
        return "Symbol/Scheme identifier cannot be blank."

    if cat in ("STOCK", "MUTUAL_FUND", "GOLD"):
        qty = float(data.get("quantity") or data.get("qty") or 0)
        if qty <= 0:
            return "Quantity must be greater than zero."
    if cat == "FIXED_DEPOSIT":
        principal = float(data.get("principal") or data.get("principal_amount") or data.get("principal_amount_cents") or 0)
        if principal <= 0:
            return "Fixed Deposit principal amount must be positive."
        if not (data.get("institution_name") or data.get("institution")):
            return "Institution name is required for Fixed Deposits."
    if cat == "BOND":
        face_val = float(data.get("face_value") or data.get("unit_price") or data.get("price") or data.get("face_value_cents") or 0)
        if face_val <= 0:
            return "Bond face value or unit price must be positive."
    return None


def _register_event(db, user_id, inv_id, event_type, event_date, amount, description):
    try:
        db.execute(
            """INSERT OR IGNORE INTO financial_events
               (user_id, investment_id, event_type, event_date, amount, status, description)
               VALUES (?,?,?,?,?,?,?)""",
            (user_id, inv_id, event_type, event_date, amount, "PENDING", description)
        )
    except Exception:
        pass


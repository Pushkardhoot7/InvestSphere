"""
InvestSphere Market Watch Routes
Integrates Centralized Market Data Service (yfinance) for Indian stock quotes,
benchmarks, historical charts, stock search, and personal watchlist management.
"""

import datetime
from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success, error
from services.market_data_service import (
    get_stock_quote, get_market_overview, get_stock_history,
    search_stocks, POPULAR_INDIAN_STOCKS, normalize_symbol, get_market_status
)

market_bp = Blueprint("market", __name__)


@market_bp.route("/quotes", methods=["GET"])
@require_auth
def get_quotes():
    """Retrieve market quotes with yfinance integration and fallback resilience."""
    db = get_db()
    uid = g.user_id
    symbols_arg = request.args.get("symbols", "")

    if symbols_arg:
        sym_list = [s.strip().upper() for s in symbols_arg.split(",") if s.strip()]
    else:
        # Return quotes for instruments the user holds, or default top Indian stocks
        rows = db.execute(
            """SELECT DISTINCT inst.symbol
               FROM investments i
               JOIN instruments inst ON i.instrument_id = inst.instrument_id
               WHERE i.user_id = ? AND i.status = 'ACTIVE'""",
            (uid,)
        ).fetchall()
        sym_list = [r["symbol"] for r in rows]
        if not sym_list:
            sym_list = [s["display_symbol"] for s in POPULAR_INDIAN_STOCKS[:12]]

    quotes = []
    for sym in sym_list:
        q = get_stock_quote(sym)
        quotes.append(q)

    return success(quotes)


@market_bp.route("/overview", methods=["GET"])
@require_auth
def market_overview():
    """Retrieve market overview indices and top movers."""
    overview = get_market_overview()
    return success(overview)


@market_bp.route("/detail/<symbol>", methods=["GET"])
@require_auth
def stock_detail(symbol):
    """Retrieve detailed quote for a specific stock."""
    q = get_stock_quote(symbol)
    return success(q)


@market_bp.route("/history/<symbol>", methods=["GET"])
@require_auth
def stock_history(symbol):
    """Retrieve historical price series for charts (period: 7d, 1mo, 3mo, 1y)."""
    period = request.args.get("period", "1mo")
    history = get_stock_history(symbol, period=period)
    return success({
        "symbol": symbol.upper(),
        "period": period,
        "history": history
    })


@market_bp.route("/search", methods=["GET"])
@require_auth
def search():
    """Search stock catalog by ticker, company name, or sector."""
    q = request.args.get("q", "")
    results = search_stocks(q)
    return success(results)


@market_bp.route("/watchlist", methods=["GET"])
@require_auth
def get_watchlist():
    """Retrieve user's personal watchlist with live quotes."""
    db = get_db()
    uid = g.user_id
    rows = db.execute(
        """SELECT symbol, instrument_name, asset_type, added_at
           FROM user_watchlists
           WHERE user_id = ?
           ORDER BY added_at DESC""",
        (uid,)
    ).fetchall()

    items = []
    for r in rows:
        sym = r["symbol"]
        q = get_stock_quote(sym)
        items.append({
            "symbol": sym,
            "instrument_name": r["instrument_name"] or q.get("name", sym),
            "asset_type": r["asset_type"],
            "added_at": r["added_at"],
            "quote": q
        })
    return success(items)


@market_bp.route("/watchlist", methods=["POST"])
@require_auth
def add_to_watchlist():
    """Add a stock or index to user's watchlist."""
    data = request.get_json(silent=True) or {}
    symbol = (data.get("symbol") or "").strip().upper()
    name = (data.get("name") or data.get("instrument_name") or symbol).strip()
    asset_type = (data.get("asset_type") or "STOCK").strip().upper()

    if not symbol:
        return error("Symbol is required.")

    db = get_db()
    uid = g.user_id
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

    try:
        db.execute(
            """INSERT OR IGNORE INTO user_watchlists (user_id, symbol, instrument_name, asset_type, added_at)
               VALUES (?, ?, ?, ?, ?)""",
            (uid, symbol, name, asset_type, now)
        )
        db.commit()
        return success({"symbol": symbol}, f"{symbol} added to watchlist.", 201)
    except Exception as e:
        db.rollback()
        return error(f"Failed to add to watchlist: {str(e)}", 500)


@market_bp.route("/watchlist/<symbol>", methods=["DELETE"])
@require_auth
def remove_from_watchlist(symbol):
    """Remove a symbol from user's watchlist."""
    sym = symbol.strip().upper()
    db = get_db()
    uid = g.user_id
    db.execute(
        "DELETE FROM user_watchlists WHERE user_id = ? AND symbol = ? COLLATE NOCASE",
        (uid, sym)
    )
    db.commit()
    return success({"symbol": sym}, f"{sym} removed from watchlist.")


@market_bp.route("/prices", methods=["POST"])
@require_auth
def record_price():
    """Record a manual market price observation for an instrument."""
    data = request.get_json(silent=True) or {}
    symbol = (data.get("symbol") or "").strip().upper()
    price_cents = data.get("price_cents")
    timestamp = data.get("timestamp") or datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")

    if not symbol:
        return error("Symbol is required.")
    if price_cents is None or int(price_cents) < 0:
        return error("Price in cents (paise) must be a non-negative integer.")

    db = get_db()
    inst = db.execute(
        "SELECT instrument_id FROM instruments WHERE symbol = ? COLLATE NOCASE", (symbol,)
    ).fetchone()
    if not inst:
        return error(f"Instrument '{symbol}' not found in catalog.", 404)

    try:
        db.execute(
            """INSERT OR REPLACE INTO market_prices
               (instrument_id, price, observation_timestamp, source, currency)
               VALUES (?, ?, ?, ?, ?)""",
            (inst["instrument_id"], int(price_cents), timestamp, "MANUAL", "INR")
        )
        db.commit()
        return success(message=f"Price recorded for {symbol}.")
    except Exception as e:
        db.rollback()
        return error(str(e), 500)

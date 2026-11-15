"""
InvestSphere Analytics Route
GET /api/analytics  — portfolio analytics & performance
"""

from flask import Blueprint, request, g
from app import get_db
from helpers import require_auth, success
import datetime

analytics_bp = Blueprint("analytics", __name__)


@analytics_bp.route("", methods=["GET"])
@require_auth
def get_analytics():
    try:
        db = get_db()
        uid = g.user_id
        category_filter = request.args.get("category", "ALL")
        start_date = request.args.get("start_date", "")
        end_date = request.args.get("end_date", "")

        # --- Holdings positions ---
        q = """
            SELECT i.investment_id, i.instrument_id, i.status, i.portfolio_tag,
                   inst.symbol, inst.instrument_name,
                   it.type_code AS cat,
                   COALESCE(ah.remaining_quantity, 0) AS qty,
                   COALESCE(ah.net_invested_amount, 0) AS cost
            FROM investments i
            JOIN instruments inst ON i.instrument_id = inst.instrument_id
            JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
            LEFT JOIN vw_active_holdings ah ON ah.investment_id = i.investment_id
            WHERE i.user_id = ? AND i.status = 'ACTIVE'
        """
        params = [uid]
        if category_filter != "ALL":
            q += " AND it.type_code = ?"
            params.append(category_filter)
        rows = db.execute(q, params).fetchall()

        # Safely query view columns
        price_map = {}
        try:
            price_rows = db.execute("SELECT instrument_id, latest_price FROM vw_latest_market_prices").fetchall()
            price_map = {r["instrument_id"]: r["latest_price"] for r in price_rows}
        except Exception:
            pass

        nav_map = {}
        try:
            nav_rows = db.execute("SELECT instrument_id, latest_nav FROM vw_latest_mutual_fund_nav").fetchall()
            nav_map = {r["instrument_id"]: r["latest_nav"] for r in nav_rows}
        except Exception:
            pass

        # Import live stock quote helper for real-time market valuation
        get_stock_quote = None
        try:
            from services.market_data_service import get_stock_quote
        except ImportError:
            pass

        total_invested = 0
        total_current = 0
        total_realized_pnl = 0
        positions = []

        for row in rows:
            inv_id = row["investment_id"]
            inst_id = row["instrument_id"]
            cat = row["cat"]
            qty = row["qty"] or 0
            cost = row["cost"] or 0
            total_invested += cost

            # Realized P&L from sell/redemption transactions
            realized = 0
            try:
                realized_row = db.execute(
                    """SELECT COALESCE(SUM(net_amount), 0) FROM transactions
                       WHERE investment_id=? AND transaction_type IN ('SELL','REDEMPTION')""",
                    (inv_id,)
                ).fetchone()
                if realized_row:
                    realized = realized_row[0]
            except Exception:
                pass
            total_realized_pnl += realized

            # Current value with live quote support
            cur_val = cost
            live_price = None

            if cat == "STOCK" and qty > 0:
                if get_stock_quote:
                    try:
                        q_data = get_stock_quote(row["symbol"])
                        if q_data and q_data.get("price") and q_data["price"] > 0:
                            live_price = float(q_data["price"])
                            cur_val = int(round(qty * live_price * 100))
                    except Exception:
                        pass
                if live_price is None and inst_id in price_map and price_map[inst_id]:
                    cur_val = int(round(qty * price_map[inst_id]))
            elif cat == "GOLD" and qty > 0:
                if inst_id in price_map and price_map[inst_id]:
                    cur_val = int(round(qty * price_map[inst_id]))
                else:
                    try:
                        from services.precious_metals_service import get_oropocket_rates
                        gold_rate = get_oropocket_rates().get("gold", {}).get("sell_inr_per_gram", 0)
                        if gold_rate > 0:
                            cur_val = int(round(qty * gold_rate * 100))
                    except Exception:
                        pass
            elif cat == "MUTUAL_FUND" and inst_id in nav_map and qty > 0:
                cur_val = int(round(qty * nav_map[inst_id] * 100))
            elif cat == "FIXED_DEPOSIT":
                try:
                    fd = db.execute(
                        "SELECT estimated_maturity_amount FROM fixed_deposits WHERE investment_id=?", (inv_id,)
                    ).fetchone()
                    cur_val = fd[0] if fd and fd[0] else cost
                except Exception:
                    cur_val = cost
            elif cat == "BOND":
                try:
                    bd = db.execute("SELECT face_value FROM bonds WHERE investment_id=?", (inv_id,)).fetchone()
                    cur_val = int(round(qty * bd[0])) if bd and qty > 0 else cost
                except Exception:
                    cur_val = cost

            total_current += cur_val
            pnl = cur_val - cost
            pnl_pct = (pnl / cost * 100) if cost > 0 else 0.0

            positions.append({
                "investment_id": inv_id,
                "symbol": row["symbol"],
                "name": row["instrument_name"],
                "category": cat,
                "type_code": cat,
                "quantity": qty,
                "cost_basis_cents": cost,
                "cost_basis": round(cost / 100, 2),
                "total_invested": round(cost / 100, 2),
                "current_value_cents": cur_val,
                "current_value": round(cur_val / 100, 2),
                "unrealized_pnl_cents": pnl,
                "unrealized_gain_loss": round(pnl / 100, 2),
                "unrealized_pnl_percent": round(pnl_pct, 2),
                "return_pct": round(pnl_pct, 2),
                "portfolio_tag": row["portfolio_tag"],
            })

        # Add allocation percentages to positions
        for p in positions:
            p["allocation_pct"] = round((p["cost_basis_cents"] / total_invested * 100) if total_invested > 0 else 0, 2)

        # --- Category breakdown ---
        cat_map = {}
        for p in positions:
            cat = p["category"]
            if cat not in cat_map:
                cat_map[cat] = {"cost": 0, "current": 0, "count": 0}
            cat_map[cat]["cost"] += p["cost_basis_cents"]
            cat_map[cat]["current"] += p["current_value_cents"]
            cat_map[cat]["count"] += 1

        categories = []
        for cat, vals in cat_map.items():
            alloc_pct = (vals["cost"] / total_invested * 100) if total_invested > 0 else 0
            pnl = vals["current"] - vals["cost"]
            pnl_pct = (pnl / vals["cost"] * 100) if vals["cost"] > 0 else 0
            categories.append({
                "category": cat,
                "count": vals["count"],
                "cost_basis_cents": vals["cost"],
                "current_value_cents": vals["current"],
                "allocation_percent": round(alloc_pct, 2),
                "pnl_cents": pnl,
                "pnl_percent": round(pnl_pct, 2),
            })
        categories.sort(key=lambda x: x["cost_basis_cents"], reverse=True)

        # --- Top/Worst performers ---
        scored = [p for p in positions if p["cost_basis_cents"] > 0]
        scored.sort(key=lambda x: x["unrealized_pnl_percent"], reverse=True)
        top = scored[:5]
        worst = scored[-5:][::-1] if len(scored) > 5 else []

        # --- Performance snapshots ---
        snapshots = []
        try:
            snap_q = """SELECT snapshot_timestamp, total_invested, estimated_portfolio_value,
                               realized_profit_loss, unrealized_profit_loss
                        FROM portfolio_snapshots WHERE user_id=?"""
            snap_params = [uid]
            if start_date:
                snap_q += " AND snapshot_timestamp >= ?"
                snap_params.append(start_date)
            if end_date:
                snap_q += " AND snapshot_timestamp <= ?"
                snap_params.append(end_date + " 23:59:59")
            snap_q += " ORDER BY snapshot_timestamp ASC LIMIT 365"
            snapshots = [dict(r) for r in db.execute(snap_q, snap_params).fetchall()]
        except Exception:
            pass

        total_pnl = total_current - total_invested
        pnl_pct_overall = (total_pnl / total_invested * 100) if total_invested > 0 else 0.0

        return success({
            "summary": {
                "total_invested_cents": total_invested,
                "total_invested": round(total_invested / 100, 2),
                "total_current_value_cents": total_current,
                "total_current_value": round(total_current / 100, 2),
                "total_unrealized_pnl_cents": total_pnl,
                "total_gain_loss": round(total_pnl / 100, 2),
                "total_unrealized_pnl_percent": round(pnl_pct_overall, 2),
                "total_gain_loss_pct": round(pnl_pct_overall, 2),
                "total_realized_pnl_cents": total_realized_pnl,
                "positions_count": len(positions),
            },
            "total_invested": round(total_invested / 100, 2),
            "total_current_value": round(total_current / 100, 2),
            "total_gain_loss": round(total_pnl / 100, 2),
            "total_gain_loss_pct": round(pnl_pct_overall, 2),
            "categories": categories,
            "allocation": categories,
            "positions": positions,
            "top_performers": top,
            "worst_performers": worst,
            "performance_history": snapshots,
            "is_empty": len(positions) == 0,
            "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        })
    except Exception as e:
        return success({
            "summary": {
                "total_invested": 0, "total_current_value": 0,
                "total_gain_loss": 0, "total_gain_loss_pct": 0,
                "positions_count": 0
            },
            "total_invested": 0, "total_current_value": 0,
            "total_gain_loss": 0, "total_gain_loss_pct": 0,
            "categories": [], "allocation": [], "positions": [],
            "top_performers": [], "worst_performers": [],
            "performance_history": [],
            "is_empty": True,
            "error_note": str(e)
        })


"""
InvestSphere IPO Routes
Endpoints for Indian Mainboard and SME IPOs.
"""

from flask import Blueprint, request
from helpers import success, error
from services.ipo_service import list_ipos, get_ipo_detail, get_ipo_summary

ipo_bp = Blueprint("ipo", __name__)


@ipo_bp.route("", methods=["GET"])
def get_ipos():
    """List and filter Indian IPOs with live ipoalerts & NSE/BSE verification."""
    status = request.args.get("status", "ALL")
    ipo_type = request.args.get("type", "ALL")
    search = request.args.get("search", "")
    sort_by = request.args.get("sort", request.args.get("sort_by", "date"))
    sort_order = request.args.get("order", request.args.get("sort_order", "desc"))
    force = request.args.get("refresh", "false").lower() in ("true", "1", "yes")
    page = int(request.args.get("page", 1))
    limit = int(request.args.get("limit", 20))

    data = list_ipos(
        status=status,
        ipo_type=ipo_type,
        search=search,
        sort_by=sort_by,
        sort_order=sort_order,
        page=page,
        limit=limit,
        force_refresh=force
    )
    return success(data)


@ipo_bp.route("/upcoming", methods=["GET"])
def get_upcoming_ipos():
    """Direct convenience endpoint for upcoming IPOs."""
    data = list_ipos(status="UPCOMING", page=1, limit=20)
    return success(data)


@ipo_bp.route("/summary", methods=["GET"])
def ipo_summary():
    """Retrieve quick count statistics across IPO categories."""
    summary = get_ipo_summary()
    return success(summary)


@ipo_bp.route("/<identifier>", methods=["GET"])
def ipo_detail(identifier):
    """Retrieve details for a specific IPO."""
    ipo = get_ipo_detail(identifier)
    if not ipo:
        return error("IPO not found", 404)
    return success(ipo)


"""
InvestSphere Precious Metals Routes
Endpoints for 24K, 22K, 18K Gold and Silver rate tracking in India and Maharashtra.
Includes city selection, historical trend charts, and retail jewellery cost calculator.
"""

from flask import Blueprint, request
from helpers import success, error
from services.precious_metals_service import (
    get_precious_metals_rates, get_supported_cities,
    get_historical_trends, calculate_retail_jewellery_price
)

precious_metals_bp = Blueprint("precious_metals", __name__)


@precious_metals_bp.route("/rates", methods=["GET"])
def get_rates():
    """Retrieve Gold and Silver rates for selected city (default: Mumbai / Pune)."""
    city = request.args.get("city", "pune")
    rates = get_precious_metals_rates(city)
    return success(rates)


@precious_metals_bp.route("/cities", methods=["GET"])
def get_cities():
    """Retrieve list of supported Maharashtra and Indian cities."""
    cities = get_supported_cities()
    return success(cities)


@precious_metals_bp.route("/history", methods=["GET"])
def get_history():
    """Retrieve historical price points (metal: gold/silver, period: 7d/30d/90d/1y)."""
    metal = request.args.get("metal", "gold")
    period = request.args.get("period", "30d")
    history = get_historical_trends(metal=metal, period=period)
    return success(history)


@precious_metals_bp.route("/calculate", methods=["POST"])
def calculate_jewellery():
    """Calculate retail jewellery billing price including metal, making charges, and 3% GST."""
    data = request.get_json(silent=True) or {}
    try:
        purity = int(data.get("purity_karat", 22))
        weight = float(data.get("weight_grams", 10.0))
        making = float(data.get("making_charge_pct", 12.0))
    except (ValueError, TypeError):
        return error("Invalid input values. Please provide numeric purity, weight, and making charge percentage.")

    if weight <= 0:
        return error("Weight in grams must be greater than zero.")
    if purity not in (18, 22, 24):
        return error("Purity karat must be 18, 22, or 24.")

    breakdown = calculate_retail_jewellery_price(purity, weight, making)
    return success(breakdown)


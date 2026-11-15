"""
InvestSphere Financial News Routes
Endpoints for categorized Indian and global market articles.
"""

from flask import Blueprint, request
from helpers import success, error
from services.news_service import list_news, get_categories

news_bp = Blueprint("news", __name__)


@news_bp.route("", methods=["GET"])
def get_articles():
    """Retrieve categorized financial news."""
    category = request.args.get("category", "ALL")
    search = request.args.get("search", "")
    page = int(request.args.get("page", 1))
    limit = int(request.args.get("limit", 12))

    data = list_news(category=category, search=search, page=page, limit=limit)
    return success(data)


@news_bp.route("/categories", methods=["GET"])
def news_categories():
    """Retrieve news categories with counts."""
    cats = get_categories()
    return success(cats)


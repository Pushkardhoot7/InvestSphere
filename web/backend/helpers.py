"""
InvestSphere Route Helpers & Auth Middleware
"""

from functools import wraps
from flask import request, jsonify, g
from security import verify_token


def require_auth(f):
    """Decorator: requires a valid Bearer token in Authorization header."""
    @wraps(f)
    def decorated(*args, **kwargs):
        auth_header = request.headers.get("Authorization", "")
        if not auth_header.startswith("Bearer "):
            return jsonify({"error": "Authentication required."}), 401

        token = auth_header[7:]
        payload = verify_token(token)
        if not payload:
            return jsonify({"error": "Invalid or expired session token. Please log in again."}), 401

        g.user_id = payload["uid"]
        g.user_email = payload["email"]
        return f(*args, **kwargs)

    return decorated


def success(data=None, message="", status=200):
    resp = {"success": True}
    if message:
        resp["message"] = message
    if data is not None:
        resp["data"] = data
    return jsonify(resp), status


def error(message="An error occurred.", status=400):
    return jsonify({"success": False, "error": message}), status


def paginate(items: list, page: int, per_page: int = 50) -> dict:
    total = len(items)
    start = (page - 1) * per_page
    end = start + per_page
    return {
        "items": items[start:end],
        "total": total,
        "page": page,
        "per_page": per_page,
        "pages": max(1, math.ceil(total / per_page)),
    }


import math


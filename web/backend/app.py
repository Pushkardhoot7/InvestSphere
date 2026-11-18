"""
InvestSphere Web Backend
Flask REST API — Python 3.12+
Mirrors the C++ desktop application's business logic
"""

import os
import re
import math
import sqlite3
import secrets
import hashlib
import hmac
import struct
import datetime
import csv
import io
import json
from functools import wraps
from flask import Flask, request, jsonify, g, make_response, send_from_directory
from flask_cors import CORS

# ─────────────────────────────────────────────────────────────────────────────
# App Factory
# ─────────────────────────────────────────────────────────────────────────────

def create_app():
    app = Flask(__name__)

    # ── Security: SECRET_KEY must be a fixed value set via environment variable.
    # If not set, use a stable production secret key so tokens survive server restarts
    # and stay synchronized across all Gunicorn workers.
    _secret = os.environ.get("SECRET_KEY", "").strip() or "investsphere-secret-key-stable-2026-production-fix-permanent"
    app.config["SECRET_KEY"] = _secret

    # ── Database: Prefer persistent disk path set via DATABASE_URL env var.
    # Render ephemeral disk is in /opt/render/project/src — wiped on deploy.
    # Persistent disk is mounted at /opt/render/project/data or similar.
    _db_url = os.environ.get("DATABASE_URL", "").strip()
    if not _db_url:
        _db_url = os.path.join(os.path.dirname(__file__), "investsphere_web.db")

    # Ensure the directory exists (for persistent disk mount paths)
    _db_dir = os.path.dirname(_db_url)
    if _db_dir and not os.path.exists(_db_dir):
        try:
            os.makedirs(_db_dir, exist_ok=True)
            print(f"[INFO] Created database directory: {_db_dir}", flush=True)
        except OSError as e:
            print(f"[WARNING] Could not create DB directory {_db_dir}: {e}", flush=True)

    app.config["DATABASE"] = _db_url
    print(f"[INFO] Database path: {_db_url}", flush=True)

    app.config["MAX_CONTENT_LENGTH"] = 1 * 1024 * 1024  # 1 MB

    # CORS — restrict in production via environment variable
    allowed_origins = os.environ.get("ALLOWED_ORIGINS", "*").split(",")
    CORS(app, resources={r"/api/*": {"origins": allowed_origins}},
         supports_credentials=True)

    # Register blueprints
    from routes.auth import auth_bp
    from routes.investments import investments_bp
    from routes.transactions import transactions_bp
    from routes.calculators import calculators_bp
    from routes.analytics import analytics_bp
    from routes.reports import reports_bp
    from routes.market import market_bp
    from routes.maturity import maturity_bp
    from routes.dashboard import dashboard_bp
    from routes.ipo import ipo_bp
    from routes.news import news_bp
    from routes.precious_metals import precious_metals_bp
    from routes.mutual_funds import mutual_funds_bp

    app.register_blueprint(auth_bp, url_prefix="/api/auth")
    app.register_blueprint(investments_bp, url_prefix="/api/investments")
    app.register_blueprint(transactions_bp, url_prefix="/api/transactions")
    app.register_blueprint(calculators_bp, url_prefix="/api/calculators")
    app.register_blueprint(analytics_bp, url_prefix="/api/analytics")
    app.register_blueprint(reports_bp, url_prefix="/api/reports")
    app.register_blueprint(market_bp, url_prefix="/api/market")
    app.register_blueprint(maturity_bp, url_prefix="/api/maturity")
    app.register_blueprint(dashboard_bp, url_prefix="/api/dashboard")
    app.register_blueprint(ipo_bp, url_prefix="/api/ipo")
    app.register_blueprint(news_bp, url_prefix="/api/news")
    app.register_blueprint(news_bp, url_prefix="/api/market-news", name="market_news")  # frontend alias
    app.register_blueprint(precious_metals_bp, url_prefix="/api/precious-metals")
    app.register_blueprint(mutual_funds_bp, url_prefix="/api/mutual-funds")
    app.register_blueprint(mutual_funds_bp, url_prefix="/api/sip", name="sip_alias")  # frontend alias

    with app.app_context():
        init_db()

    # Health check
    @app.route("/api/health")
    def health():
        return jsonify({
            "status": "ok",
            "version": "1.0.0",
            "app": "InvestSphere Web",
            "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat()
        })

    # Serve Frontend Single Page App and static assets
    frontend_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "frontend"))

    @app.route("/", defaults={"path": ""})
    @app.route("/<path:path>")
    def serve_frontend(path):
        if path.startswith("api/"):
            return jsonify({"error": "Endpoint not found"}), 404
        target_path = os.path.join(frontend_dir, path)
        if path and os.path.exists(target_path) and os.path.isfile(target_path):
            return send_from_directory(frontend_dir, path)
        return send_from_directory(frontend_dir, "index.html")

    @app.teardown_appcontext
    def close_db(error):
        db = g.pop("db", None)
        if db is not None:
            db.close()

    @app.errorhandler(404)
    def not_found(e):
        if request.path.startswith("/api/"):
            return jsonify({"error": "API route not found"}), 404
        return send_from_directory(frontend_dir, "index.html")

    @app.errorhandler(405)
    def method_not_allowed(e):
        return jsonify({"error": "Method not allowed"}), 405

    @app.errorhandler(413)
    def request_too_large(e):
        return jsonify({"error": "Request too large"}), 413

    return app


# ─────────────────────────────────────────────────────────────────────────────
# Database
# ─────────────────────────────────────────────────────────────────────────────

def get_db():
    if "db" not in g:
        db_path = current_app_config("DATABASE")
        conn = sqlite3.connect(db_path, check_same_thread=False)
        conn.row_factory = sqlite3.Row
        conn.execute("PRAGMA foreign_keys = ON")
        conn.execute("PRAGMA journal_mode = WAL")
        conn.execute("PRAGMA synchronous = NORMAL")
        conn.execute("PRAGMA busy_timeout = 5000")
        g.db = conn
    return g.db


def current_app_config(key):
    from flask import current_app
    return current_app.config[key]


def init_db():
    """Initialize schema from SQL files."""
    db_path = current_app_config("DATABASE")
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")

    sql_dir = os.path.join(os.path.dirname(__file__), "..", "..", "sql")

    for fname in ["schema.sql", "views.sql", "triggers.sql"]:
        fpath = os.path.join(sql_dir, fname)
        if os.path.exists(fpath):
            with open(fpath, "r", encoding="utf-8") as f:
                sql = f.read()
            try:
                conn.executescript(sql)
            except sqlite3.OperationalError as e:
                if "already exists" not in str(e).lower():
                    print(f"[init_db] Warning ({fname}): {e}")

    # Seed investment_types if empty
    cur = conn.execute("SELECT COUNT(*) FROM investment_types")
    if cur.fetchone()[0] == 0:
        seed_fpath = os.path.join(sql_dir, "seed_data.sql")
        if os.path.exists(seed_fpath):
            with open(seed_fpath, "r", encoding="utf-8") as f:
                seed_sql = f.read()
            for stmt in _split_sql(seed_sql):
                try:
                    conn.execute(stmt)
                except sqlite3.OperationalError:
                    pass
        else:
            # Inline minimal seed if file not found
            _seed_investment_types(conn)

    # Run column migrations for extended fields
    for col, col_type in [
        ("mobile_number", "TEXT"),
        ("country", "TEXT DEFAULT 'India'"),
        ("state", "TEXT DEFAULT 'Maharashtra'"),
        ("pan_masked", "TEXT"),
        ("date_of_birth", "TEXT")
    ]:
        try:
            conn.execute(f"ALTER TABLE users ADD COLUMN {col} {col_type}")
        except sqlite3.OperationalError:
            pass

    try:
        conn.execute("ALTER TABLE sip_plans ADD COLUMN step_up_percent REAL NOT NULL DEFAULT 0.0")
    except sqlite3.OperationalError:
        pass

    # Ensure base user accounts always exist across ephemeral container redeploys.
    # Password for all default accounts: Pushkar@2026  (user can reset via the Reset Password page)
    # We use INSERT OR IGNORE so user-changed passwords are never overwritten on redeploy.
    # If the stored hash is the old broken demo hash, we repair it to the known default.
    _BROKEN_DEMO_HASH = "$pbkdf2-sha256$i=100000,l=32$e696d7adab4b8f4c2f45a88be6fba6e33e56588bc182709000b14e68ebe00c27$f7652bcc0c48eeec9c98199f617609b670a22530d480b160e4c8fc2bc0b0906a"
    _DEFAULT_PW_HASH  = "$pbkdf2-sha256$i=100000,l=32$9837833e323c558719774b8866c964951a19603d390d2d5324be7dade625bdba$67b859ad5c21d0b3626c5362131e6592330578dbb36359494fb66168b6e4df01"
    default_users = [
        ("Pushkar Dhoot",  "pushkardhoot2121@gmail.com", _DEFAULT_PW_HASH, "India", "Maharashtra"),
        ("Aditya Sharma",  "aditya@investsphere.local",  _DEFAULT_PW_HASH, "India", "Maharashtra"),
        ("Elena Rostova",  "elena@investsphere.local",   _DEFAULT_PW_HASH, "India", "Maharashtra"),
    ]
    for full_name, email, pw_hash, country, state in default_users:
        # Create account if it doesn't exist yet
        conn.execute(
            """INSERT OR IGNORE INTO users (full_name, email, password_hash, account_status, country, state)
               VALUES (?, ?, ?, 'ACTIVE', ?, ?)""",
            (full_name, email, pw_hash, country, state)
        )
        # If it already existed with the broken demo hash, repair it to the known default
        conn.execute(
            """UPDATE users SET password_hash = ?, account_status = 'ACTIVE'
               WHERE email = ? AND password_hash = ?""",
            (pw_hash, email, _BROKEN_DEMO_HASH)
        )

    conn.commit()
    conn.close()


def _seed_investment_types(conn):
    types = [
        (1, "STOCK",          "Equity Stocks",           "Direct equity shares and ETFs"),
        (2, "MUTUAL_FUND",    "Mutual Funds",            "Pooled investment fund schemes"),
        (3, "GOLD",           "Gold & Precious Metals",  "Physical gold and gold bonds"),
        (4, "FIXED_DEPOSIT",  "Fixed Deposits",          "Bank term deposits"),
        (5, "BOND",           "Bonds & Debentures",      "Government and corporate bonds"),
        (6, "CASH",           "Cash & Savings",          "Liquid cash savings"),
    ]
    conn.executemany(
        "INSERT OR IGNORE INTO investment_types (investment_type_id, type_code, type_name, description) VALUES (?,?,?,?)",
        types
    )


def _split_sql(sql_content: str) -> list:
    """Split multi-statement SQL on semicolons, skipping comments."""
    statements = []
    current = []
    for line in sql_content.splitlines():
        stripped = line.strip()
        if stripped.startswith("--") or not stripped:
            continue
        current.append(line)
        if stripped.endswith(";"):
            stmt = " ".join(current).strip().rstrip(";").strip()
            if stmt:
                statements.append(stmt)
            current = []
    if current:
        stmt = " ".join(current).strip().rstrip(";").strip()
        if stmt:
            statements.append(stmt)
    return statements


# ─────────────────────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = create_app()
    port = int(os.environ.get("PORT", 5000))
    debug = os.environ.get("FLASK_ENV", "production") == "development"
    app.run(host="0.0.0.0", port=port, debug=debug)


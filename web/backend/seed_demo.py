"""
InvestSphere Demo Account Seeder
Creates a fictional demonstration account with sample holdings and transactions
for academic evaluation and portfolio demonstration.

Safety guarantees:
- Fictional financial instruments and mock figures only
- Standard non-privileged user account (no admin rights)
- Strict user-isolation: only accesses user_id associated with demo account
- Idempotent: can be run repeatedly without duplicating holdings
"""

import os
import sys
import sqlite3
import datetime

# Ensure backend root is on sys.path
sys.path.insert(0, os.path.dirname(__file__))

from security import hash_password

DEMO_EMAIL = os.environ.get("DEMO_USER_EMAIL", "demo@investsphere.demo")
DEMO_PASS = os.environ.get("DEMO_USER_PASSWORD", "DemoInvestor123!")
DEMO_NAME = "Demo Investor"

def init_schema(conn):
    sql_dir = os.path.join(os.path.dirname(__file__), "..", "..", "sql")
    for fname in ["schema.sql", "views.sql", "triggers.sql"]:
        fpath = os.path.join(sql_dir, fname)
        if os.path.exists(fpath):
            with open(fpath, "r", encoding="utf-8") as f:
                conn.executescript(f.read())

    # Seed investment types if empty
    cur = conn.execute("SELECT COUNT(*) FROM investment_types")
    if cur.fetchone()[0] == 0:
        seed_fpath = os.path.join(sql_dir, "seed_data.sql")
        if os.path.exists(seed_fpath):
            with open(seed_fpath, "r", encoding="utf-8") as f:
                conn.executescript(f.read())

def seed_demo_data(db_path=None):
    if not db_path:
        db_path = os.environ.get("DATABASE_URL", os.path.join(os.path.dirname(__file__), "investsphere_web.db"))

    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    init_schema(conn)

    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
    today = datetime.date.today().isoformat()
    maturity_date = (datetime.date.today() + datetime.timedelta(days=45)).isoformat()

    # 1. Create or fetch demo user
    cur = conn.execute("SELECT user_id FROM users WHERE email = ?", (DEMO_EMAIL,))
    user = cur.fetchone()

    if user:
        user_id = user["user_id"]
        print(f"[Seed] Demo user exists (user_id={user_id}). Resetting demo holdings...")
        # Clean existing demo holdings in FK dependency order
        conn.execute("DELETE FROM financial_events WHERE user_id = ?", (user_id,))
        conn.execute("DELETE FROM transactions WHERE user_id = ?", (user_id,))
        conn.execute("DELETE FROM fixed_deposits WHERE investment_id IN (SELECT investment_id FROM investments WHERE user_id = ?)", (user_id,))
        conn.execute("DELETE FROM investments WHERE user_id = ?", (user_id,))
    else:
        pw_hash = hash_password(DEMO_PASS)
        cur = conn.execute(
            """INSERT INTO users (full_name, email, password_hash, account_status, created_at, updated_at)
               VALUES (?, ?, ?, 'ACTIVE', ?, ?)""",
            (DEMO_NAME, DEMO_EMAIL, pw_hash, now, now)
        )
        user_id = cur.lastrowid
        print(f"[Seed] Created demo user (user_id={user_id})")

    # Helper to insert investment & transactions
    def add_investment(cat_id, symbol, name, cur_price_cents):
        # Instrument
        cur = conn.execute("SELECT instrument_id FROM instruments WHERE symbol = ?", (symbol,))
        inst = cur.fetchone()
        if inst:
            inst_id = inst["instrument_id"]
        else:
            cur = conn.execute(
                """INSERT INTO instruments (investment_type_id, symbol, instrument_name, is_active, created_at)
                   VALUES (?, ?, ?, 1, ?)""",
                (cat_id, symbol, name, now)
            )
            inst_id = cur.lastrowid

        # Record latest price
        conn.execute(
            """INSERT INTO market_prices (instrument_id, price, observation_timestamp, source)
               VALUES (?, ?, ?, 'DEMO_SEED')""",
            (inst_id, cur_price_cents, now)
        )
        return inst_id

    # 1. Equity: RELIANCE (20 shares @ purchase 2,850.00, current 2,980.00)
    inst_rel = add_investment(1, "RELIANCE", "Reliance Industries Ltd", 298000)
    cur = conn.execute(
        """INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at)
           VALUES (?, ?, 'Growth', 'ACTIVE', ?)""",
        (user_id, inst_rel, now)
    )
    inv_rel = cur.lastrowid
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
                                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes, created_at)
           VALUES (?, ?, 'BUY', '2026-01-15', 20.0, 285000, 5700000, 5000, 1000, 5706000, 'Initial purchase via SIP', ?)""",
        (user_id, inv_rel, now)
    )

    # 2. Equity: TCS (10 shares @ purchase 3,800.00, current 4,120.00)
    inst_tcs = add_investment(1, "TCS", "Tata Consultancy Services Ltd", 412000)
    cur = conn.execute(
        """INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at)
           VALUES (?, ?, 'Tech Core', 'ACTIVE', ?)""",
        (user_id, inst_tcs, now)
    )
    inv_tcs = cur.lastrowid
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
                                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes, created_at)
           VALUES (?, ?, 'BUY', '2026-02-01', 10.0, 380000, 3800000, 3500, 700, 3804200, 'Bluechip portfolio allocation', ?)""",
        (user_id, inv_tcs, now)
    )

    # 3. Mutual Fund: NIFTYBEES (250 units @ purchase 240.00, current 258.50)
    inst_mf = add_investment(2, "NIFTYBEES", "Nippon India Nifty 50 ETF", 25850)
    cur = conn.execute(
        """INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at)
           VALUES (?, ?, 'Passive Index', 'ACTIVE', ?)""",
        (user_id, inst_mf, now)
    )
    inv_mf = cur.lastrowid
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
                                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes, created_at)
           VALUES (?, ?, 'BUY', '2026-02-10', 250.0, 24000, 6000000, 2000, 400, 6002400, 'Index fund core accumulation', ?)""",
        (user_id, inv_mf, now)
    )

    # 4. Gold: GOLDBEES (20 units @ purchase 6,100.00, current 6,450.00)
    inst_gold = add_investment(3, "GOLDBEES", "Nippon India Gold ETF", 645000)
    cur = conn.execute(
        """INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at)
           VALUES (?, ?, 'Hedge', 'ACTIVE', ?)""",
        (user_id, inst_gold, now)
    )
    inv_gold = cur.lastrowid
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
                                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes, created_at)
           VALUES (?, ?, 'BUY', '2026-01-20', 20.0, 610000, 12200000, 3000, 600, 12203600, 'Gold hedge allocation', ?)""",
        (user_id, inv_gold, now)
    )

    # 5. Fixed Deposit: SBI 1-Year FD (Principal ₹1,50,000 @ 7.1%, Maturing in 45 days)
    inst_fd = add_investment(4, "FD-SBI-2026", "SBI Special Term Deposit", 15000000)
    cur = conn.execute(
        """INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at)
           VALUES (?, ?, 'Fixed Income', 'ACTIVE', ?)""",
        (user_id, inst_fd, now)
    )
    inv_fd = cur.lastrowid
    conn.execute(
        """INSERT INTO fixed_deposits (investment_id, institution_name, account_number, principal_amount,
                                      annual_interest_rate, compounding_frequency, start_date, maturity_date,
                                      estimated_maturity_amount, payout_type)
           VALUES (?, 'State Bank of India', 'FD-SBI-2025-9981', 15000000, 7.10, 'QUARTERLY', '2025-03-28', ?, 16093500, 'CUMULATIVE')""",
        (inv_fd, maturity_date)
    )
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
                                    quantity, unit_price, gross_amount, fees, taxes, net_amount, notes, created_at)
           VALUES (?, ?, 'DEPOSIT', '2025-03-28', 1.0, 15000000, 15000000, 0, 0, 15000000, 'Term deposit booked at branch', ?)""",
        (user_id, inv_fd, now)
    )
    conn.execute(
        """INSERT INTO financial_events (user_id, investment_id, event_type, event_date, amount, status, description, created_at)
           VALUES (?, ?, 'FD_MATURITY', ?, 16093500, 'PENDING', 'SBI Term Deposit Payout', ?)""",
        (user_id, inv_fd, maturity_date, now)
    )

    conn.commit()
    conn.close()

    print("\n" + "=" * 55)
    print("DEMO ACCOUNT SUCCESSFULLY SEEDED!")
    print("=" * 55)
    print(f"Email:    {DEMO_EMAIL}")
    print(f"Password: {DEMO_PASS}")
    print("Holdings: 5 (RELIANCE, TCS, NIFTYBEES, GOLDBEES, SBI FD)")
    print("=" * 55 + "\n")

if __name__ == "__main__":
    db_file = sys.argv[1] if len(sys.argv) > 1 else None
    seed_demo_data(db_file)

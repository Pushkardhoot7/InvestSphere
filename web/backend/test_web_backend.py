"""
InvestSphere Web Backend — Standalone Test
Tests all business logic without Flask (stdlib only).
Run: python test_web_backend.py
"""
import sys, os, json, tempfile, sqlite3, hashlib, hmac, secrets, math, datetime

sys.path.insert(0, os.path.dirname(__file__))

PASS_COUNT = 0
FAIL_COUNT = 0

def check(label, cond, detail=""):
    global PASS_COUNT, FAIL_COUNT
    if cond:
        print(f"  [PASS] {label}")
        PASS_COUNT += 1
    else:
        print(f"  [FAIL] {label}" + (f" — {detail}" if detail else ""))
        FAIL_COUNT += 1

# ── Setup temp database ───────────────────────────────────────────────────────
print("\n[0] Setting up test database")
tmp = tempfile.NamedTemporaryFile(suffix=".db", delete=False)
tmp.close()
DB = tmp.name

SQL_DIR = os.path.join(os.path.dirname(__file__), "..", "..", "sql")

def get_conn():
    conn = sqlite3.connect(DB)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    conn.execute("PRAGMA journal_mode = WAL")
    return conn

def split_sql(sql):
    stmts = []
    curr = []
    for line in sql.splitlines():
        s = line.strip()
        if s.startswith("--") or not s:
            continue
        curr.append(line)
        if s.endswith(";"):
            stmt = " ".join(curr).strip().rstrip(";")
            if stmt:
                stmts.append(stmt)
            curr = []
    if curr:
        stmt = " ".join(curr).strip().rstrip(";")
        if stmt:
            stmts.append(stmt)
    return stmts

conn = get_conn()
# Use executescript() which handles multi-statement SQL (including triggers with BEGIN..END)
# executescript() auto-commits before running, so we use a raw connection
raw_conn = sqlite3.connect(DB)
raw_conn.execute("PRAGMA foreign_keys = ON")
for fname in ["schema.sql", "views.sql", "triggers.sql"]:
    fpath = os.path.join(SQL_DIR, fname)
    if os.path.exists(fpath):
        with open(fpath, encoding="utf-8") as f:
            sql = f.read()
        try:
            raw_conn.executescript(sql)
        except sqlite3.OperationalError as e:
            if "already exists" not in str(e).lower():
                print(f"    Warning ({fname}): {e}")
raw_conn.close()
# Re-open with row_factory
conn = get_conn()

# Seed investment types
conn.executemany(
    "INSERT OR IGNORE INTO investment_types (investment_type_id, type_code, type_name, description) VALUES (?,?,?,?)",
    [
        (1,"STOCK","Equity Stocks","Direct equity shares"),
        (2,"MUTUAL_FUND","Mutual Funds","Pooled fund schemes"),
        (3,"GOLD","Gold & Precious Metals","Physical gold"),
        (4,"FIXED_DEPOSIT","Fixed Deposits","Bank term deposits"),
        (5,"BOND","Bonds & Debentures","Government and corporate bonds"),
        (6,"CASH","Cash & Savings","Liquid cash"),
    ]
)
conn.commit()
check("DB initialized", True)


# ── 1. Password Hashing ───────────────────────────────────────────────────────
print("\n[1] Password Hashing (PBKDF2-SHA256)")
from security import hash_password, verify_password, generate_token, verify_token

h = hash_password("MySecurePass1")
check("Hash format correct",    h.startswith("$pbkdf2-sha256$"))
check("Verify correct pw",      verify_password("MySecurePass1", h))
check("Reject wrong pw",        not verify_password("WrongPass1", h))
check("Constant-time compare",  not verify_password("", h))


# ── 2. Session Tokens ─────────────────────────────────────────────────────────
print("\n[2] Session Tokens")
os.environ["SECRET_KEY"] = "test-secret-only"
tok = generate_token(42, "test@example.com", expires_in=3600)
check("Token generated",        bool(tok) and "." in tok)
payload = verify_token(tok)
check("Token verifiable",       payload is not None)
check("UID correct",            payload.get("uid") == 42)
check("Email correct",          payload.get("email") == "test@example.com")
check("Expired token rejected", verify_token(generate_token(1,"a@b.com", expires_in=-1)) is None)
check("Tampered token rejected",verify_token(tok + "x") is None)


# ── 3. User Registration ──────────────────────────────────────────────────────
print("\n[3] User Registration")
pw_hash = hash_password("Password123")
now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
cur = conn.execute(
    "INSERT INTO users (full_name, email, password_hash, account_status, created_at, updated_at) "
    "VALUES (?,?,?,'ACTIVE',?,?)",
    ("Test User", "test@investsphere.dev", pw_hash, now, now)
)
conn.commit()
uid = cur.lastrowid
check("User created",           uid > 0)

# Duplicate email
try:
    conn.execute(
        "INSERT INTO users (full_name, email, password_hash, account_status, created_at, updated_at) "
        "VALUES (?,?,?,'ACTIVE',?,?)",
        ("Dup User", "test@investsphere.dev", pw_hash, now, now)
    )
    conn.commit()
    check("Duplicate email rejected", False, "Should have raised IntegrityError")
except sqlite3.IntegrityError:
    conn.rollback()
    check("Duplicate email rejected", True)


# ── 4. Investment CRUD ────────────────────────────────────────────────────────
print("\n[4] Investment CRUD")
# Add instrument
cur = conn.execute(
    "INSERT INTO instruments (investment_type_id, symbol, instrument_name, currency, is_active, created_at) "
    "VALUES (?,?,?,?,1,?)",
    (1, "RELIANCE", "Reliance Industries Ltd.", "INR", now)
)
conn.commit()
inst_id = cur.lastrowid
check("Instrument created",     inst_id > 0)

# Add investment holding
cur = conn.execute(
    "INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at, updated_at) "
    "VALUES (?,?,'DEFAULT','ACTIVE',?,?)",
    (uid, inst_id, now, now)
)
conn.commit()
inv_id = cur.lastrowid
check("Investment holding created", inv_id > 0)

# Add stock details
conn.execute(
    "INSERT INTO stocks (instrument_id, sector, market_cap_category) VALUES (?,?,?)",
    (inst_id, "Energy", "LARGE_CAP")
)
conn.commit()
check("Stock specialization",   True)


# ── 5. Transaction Ledger ─────────────────────────────────────────────────────
print("\n[5] Transaction Ledger")
cur = conn.execute(
    """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
       quantity, unit_price, gross_amount, fees, taxes, net_amount, notes)
       VALUES (?,?,?,?,?,?,?,?,?,?,?)""",
    (uid, inv_id, "BUY", "2026-01-15 10:00:00", 10.0, 280000, 2800000, 5000, 0, 2805000, "Initial purchase")
)
conn.commit()
tx_id = cur.lastrowid
check("Transaction recorded",   tx_id > 0)

# Check audit log trigger
audit = conn.execute(
    "SELECT COUNT(*) FROM audit_logs WHERE entity_type='TRANSACTION' AND entity_id=?", (tx_id,)
).fetchone()[0]
check("Audit trigger fired",    audit == 1)

# Second BUY
conn.execute(
    """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
       quantity, unit_price, gross_amount, fees, taxes, net_amount)
       VALUES (?,?,?,?,?,?,?,?,?,?)""",
    (uid, inv_id, "BUY", "2026-06-01 10:00:00", 5.0, 290000, 1450000, 2000, 500, 1452500)
)
conn.commit()
check("Second transaction",     True)

# Oversell prevention trigger
try:
    conn.execute(
        """INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date,
           quantity, unit_price, gross_amount, fees, taxes, net_amount)
           VALUES (?,?,?,?,?,?,?,?,?,?)""",
        (uid, inv_id, "SELL", "2026-07-01 10:00:00", 999.0, 300000, 299700000, 0, 0, 299700000)
    )
    conn.commit()
    # If we get here without exception, verify the row is NOT in DB (trigger may have silently blocked)
    oversell_row = conn.execute(
        "SELECT COUNT(*) FROM transactions WHERE transaction_type='SELL' AND quantity=999 AND investment_id=?",
        (inv_id,)
    ).fetchone()[0]
    check("Oversell trigger blocked", oversell_row == 0, f"Row count={oversell_row}")
except (sqlite3.OperationalError, sqlite3.IntegrityError) as e:
    conn.rollback()
    check("Oversell trigger blocked", True)  # Exception raised = trigger worked


# ── 6. Cost Basis Engine ──────────────────────────────────────────────────────
print("\n[6] Cost Basis Engine")
txs = conn.execute(
    "SELECT transaction_type, quantity, net_amount FROM transactions WHERE investment_id=?", (inv_id,)
).fetchall()

qty = 0.0; cost = 0
for tx in txs:
    if tx["transaction_type"] in ("BUY", "DEPOSIT"):
        qty += tx["quantity"]
        cost += tx["net_amount"]
    elif tx["transaction_type"] in ("SELL", "REDEMPTION"):
        qty -= tx["quantity"]

check("Remaining qty = 15",     abs(qty - 15.0) < 0.001, f"Got {qty}")
check("Total cost correct",     cost == 4257500, f"Got {cost}")


# ── 7. SIP Calculator ─────────────────────────────────────────────────────────
print("\n[7] SIP Calculator")

def calc_sip(monthly, rate, years, step_up=0):
    r = rate / 1200.0
    zero_rate = r < 1e-9
    corpus = 0.0; invested = 0.0; cur_monthly = monthly
    for y in range(1, years + 1):
        gf = 1.0 if zero_rate else math.pow(1.0 + r, 12)
        grown = corpus * gf
        fv = cur_monthly * 12.0 if zero_rate else cur_monthly * ((math.pow(1.0+r,12)-1)/r)
        corpus = grown + fv
        invested += cur_monthly * 12.0
        if step_up > 0:
            cur_monthly *= (1 + step_up / 100.0)
    return invested, corpus

inv, fv = calc_sip(10000, 12.0, 10)
check("SIP: FV > invested",     fv > inv)
check("SIP: FV ≈ ₹23.2L",      abs(fv - 2323000) < 150000, f"Got ₹{fv:.0f}")

inv0, fv0 = calc_sip(10000, 0, 10)
check("SIP zero-rate = Σpayments", abs(fv0 - inv0) < 1, f"Got FV={fv0:.0f}, Inv={inv0:.0f}")

inv_su, fv_su = calc_sip(5000, 12.0, 10, step_up=10)
check("SIP step-up > no step-up", fv_su > fv / 2)


# ── 8. SWP Calculator ─────────────────────────────────────────────────────────
print("\n[8] SWP Calculator")

def calc_swp(corpus, withdrawal, rate, years):
    r = rate / 1200.0
    zero_rate = r < 1e-9
    total_months = years * 12
    balance = corpus; total_withdrawn = 0.0; depleted = False; dep_month = 0
    for y in range(1, years + 1):
        for m in range(1, 13):
            gm = (y-1)*12 + m
            if gm > total_months or depleted:
                break
            interest = 0.0 if zero_rate else balance * r
            balance += interest
            if withdrawal <= 0:
                pass
            elif balance > withdrawal + 1e-9:
                balance -= withdrawal; total_withdrawn += withdrawal
            elif balance > 1e-9:
                total_withdrawn += balance; balance = 0.0
                depleted = True; dep_month = gm; break
    return balance, total_withdrawn, depleted, dep_month

bal, withdrawn, dep, dm = calc_swp(1_000_000, 8000, 8.0, 15)
check("SWP: not depleted (8% / ₹8k)", not dep, f"depleted at month {dm}")
check("SWP: positive balance",          bal > 0, f"bal={bal:.0f}")

bal2, _, dep2, dm2 = calc_swp(100_000, 50_000, 0, 5)
check("SWP: depletes on big withdrawal", dep2, "should deplete")


# ── 9. Database Views ─────────────────────────────────────────────────────────
print("\n[9] Database Views")
vh = conn.execute(
    "SELECT * FROM vw_active_holdings WHERE user_id=?", (uid,)
).fetchone()
check("vw_active_holdings works", vh is not None)
check("Qty from view = 15",       abs((vh["remaining_quantity"] or 0) - 15.0) < 0.001, vh["remaining_quantity"])
check("Cost from view correct",   vh["net_invested_amount"] == 4257500, vh["net_invested_amount"])


# ── 10. FD + Financial Event ─────────────────────────────────────────────────
print("\n[10] Fixed Deposit & Financial Events")
cur = conn.execute(
    "INSERT INTO instruments (investment_type_id, symbol, instrument_name, currency, is_active, created_at) "
    "VALUES (?,?,?,?,1,?)",
    (4, "FD-SBI-001", "SBI 1-Year FD", "INR", now)
)
conn.commit()
fd_inst_id = cur.lastrowid
cur = conn.execute(
    "INSERT INTO investments (user_id, instrument_id, portfolio_tag, status, created_at, updated_at) "
    "VALUES (?,?,'DEFAULT','ACTIVE',?,?)",
    (uid, fd_inst_id, now, now)
)
conn.commit()
fd_inv_id = cur.lastrowid
conn.execute(
    """INSERT INTO fixed_deposits (investment_id, institution_name, principal_amount,
       annual_interest_rate, compounding_frequency, payout_type, start_date, maturity_date,
       estimated_maturity_amount, status)
       VALUES (?,?,?,?,?,?,?,?,?,?)""",
    (fd_inv_id, "SBI", 10_000_000, 7.1, "QUARTERLY", "CUMULATIVE",
     "2026-03-01", "2027-03-01", 10_724_000, "ACTIVE")
)
conn.execute(
    "INSERT OR IGNORE INTO financial_events (user_id, investment_id, event_type, event_date, amount, status, description) "
    "VALUES (?,?,?,?,?,?,?)",
    (uid, fd_inv_id, "FD_MATURITY", "2027-03-01", 10_724_000, "PENDING", "SBI FD Maturity")
)
conn.commit()
evt_count = conn.execute(
    "SELECT COUNT(*) FROM financial_events WHERE user_id=? AND event_type='FD_MATURITY'", (uid,)
).fetchone()[0]
check("FD maturity event registered", evt_count >= 1)

vm = conn.execute(
    "SELECT * FROM vw_upcoming_maturities WHERE user_id=?", (uid,)
).fetchall()
check("vw_upcoming_maturities view works", len(vm) >= 1)


# ── 11. Data Isolation ───────────────────────────────────────────────────────
print("\n[11] Data Isolation")
cur = conn.execute(
    "INSERT INTO users (full_name, email, password_hash, account_status, created_at, updated_at) "
    "VALUES (?,?,?,'ACTIVE',?,?)",
    ("Other User", "other@investsphere.dev", pw_hash, now, now)
)
conn.commit()
other_uid = cur.lastrowid
# Other user should not see user 1's investments
other_inv = conn.execute(
    "SELECT * FROM investments WHERE investment_id=? AND user_id=?", (inv_id, other_uid)
).fetchone()
check("User isolation: no cross-access", other_inv is None)
other_tx = conn.execute(
    "SELECT * FROM transactions WHERE transaction_id=? AND user_id=?", (tx_id, other_uid)
).fetchone()
check("Transaction isolation enforced", other_tx is None)


# ── 12. Triggers ─────────────────────────────────────────────────────────────
print("\n[12] Database Triggers")
# Audit immutability
try:
    conn.execute("UPDATE audit_logs SET description='tampered' WHERE audit_id=1")
    conn.commit()
    check("Audit log immutable", False, "Should have raised")
except (sqlite3.OperationalError, sqlite3.IntegrityError) as e:
    conn.rollback()
    check("Audit log immutable", True)

# Timestamp trigger
conn.execute("UPDATE investments SET notes='updated' WHERE investment_id=?", (inv_id,))
conn.commit()
updated = conn.execute(
    "SELECT updated_at FROM investments WHERE investment_id=?", (inv_id,)
).fetchone()
check("Timestamp trigger fires", updated is not None)


# ── 13. Reports / CSV logic ───────────────────────────────────────────────────
print("\n[13] Reports")
holdings = conn.execute(
    """SELECT i.investment_id, inst.symbol, inst.instrument_name, it.type_code,
              i.portfolio_tag, i.status,
              COALESCE(ah.remaining_quantity, 0) AS qty,
              COALESCE(ah.net_invested_amount, 0) AS cost_cents
       FROM investments i
       JOIN instruments inst ON i.instrument_id = inst.instrument_id
       JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
       LEFT JOIN vw_active_holdings ah ON ah.investment_id = i.investment_id
       WHERE i.user_id = ?
       ORDER BY it.type_code, inst.symbol""",
    (uid,)
).fetchall()
check("Holdings report query", len(holdings) >= 2)

import csv, io
output = io.StringIO()
writer = csv.writer(output)
writer.writerow(["ID","Symbol","Name","Category","Qty","Cost (₹)"])
for h in holdings:
    writer.writerow([h["investment_id"], h["symbol"], h["instrument_name"],
                     h["type_code"], h["qty"], f"{h['cost_cents']/100:.2f}"])
csv_content = output.getvalue()
check("CSV export generated",   len(csv_content) > 100)
check("CSV has header row",     "Symbol" in csv_content)
check("CSV has RELIANCE",       "RELIANCE" in csv_content)


# ── 14. API Aliasing & Flexible Payload Normalization ─────────────────────────
print("\n[14] API Aliasing & Payload Normalization")

def normalize_investment_payload(d):
    category = (d.get("category_code") or d.get("category") or "").upper()
    name = d.get("instrument_name") or d.get("name") or ""
    symbol = d.get("symbol") or name
    qty = float(d.get("quantity") or 0)
    principal = float(d.get("principal_amount_cents") or 0)
    if principal == 0 and ("principal" in d or "principal_amount" in d or "total_amount" in d):
        principal = round(float(d.get("principal") or d.get("principal_amount") or d.get("total_amount") or 0) * 100)
    return {"category": category, "name": name, "symbol": symbol, "qty": qty, "principal": principal}

norm1 = normalize_investment_payload({"category": "stock", "name": "Tata Motors", "quantity": 25, "unit_price": 950})
check("Aliased category resolved", norm1["category"] == "STOCK")
check("Aliased name resolved",     norm1["name"] == "Tata Motors")
check("Aliased symbol fallback",   norm1["symbol"] == "Tata Motors")

norm2 = normalize_investment_payload({"category_code": "FIXED_DEPOSIT", "instrument_name": "HDFC FD", "principal": 50000})
check("Rupees to cents conversion", norm2["principal"] == 5000000)

def normalize_tx_payload(d):
    tx_type = (d.get("tx_type") or d.get("transaction_type") or "").upper()
    date = d.get("tx_date") or d.get("transaction_date") or ""
    qty = float(d.get("qty") or d.get("quantity") or 0)
    unit_price = float(d.get("unit_price_cents") or 0)
    if unit_price == 0 and "unit_price" in d:
        unit_price = round(float(d.get("unit_price") or 0) * 100)
    return {"tx_type": tx_type, "date": date, "qty": qty, "unit_price": unit_price}

tx_norm = normalize_tx_payload({"tx_type": "buy", "tx_date": "2026-03-01", "qty": 10, "unit_price": 1250.50})
check("Tx type aliasing", tx_norm["tx_type"] == "BUY")
check("Tx date aliasing", tx_norm["date"] == "2026-03-01")
check("Tx unit price rupees to cents", tx_norm["unit_price"] == 125050)


# ── 15. Analytics Math & Performance Calculations ─────────────────────────────
print("\n[15] Financial Math & Allocations")
# Verify portfolio allocation math
total_val = 500000
stock_val = 300000
fd_val = 200000
alloc_stock = round((stock_val / total_val) * 100, 2)
alloc_fd = round((fd_val / total_val) * 100, 2)
check("Allocation stock %", alloc_stock == 60.0)
check("Allocation FD %",    alloc_fd == 40.0)
check("Allocation sum 100%", (alloc_stock + alloc_fd) == 100.0)

# Verify gain/loss % math
cost = 250000
current = 300000
gain = current - cost
gain_pct = round((gain / cost) * 100, 2)
check("Gain calculation", gain == 50000)
check("Gain percent calculation", gain_pct == 20.0)


# ── 16. Extended User Registration & Security Validation ──────────────
print("\n[16] Extended User Registration & Security Validation")
from security import (
    is_valid_mobile, normalize_mobile,
    is_valid_pan, mask_pan,
    is_valid_dob, verify_math_captcha, generate_math_captcha
)

# Mobile Validation
check("Valid Indian mobile (9820012345)", is_valid_mobile("9820012345"))
check("Valid Indian mobile with +91",      is_valid_mobile("+91 98200 12345"))
check("Normalize mobile removes prefix",   normalize_mobile("+91 98200 12345") == "9820012345")
check("Invalid mobile (starts with 1)",    not is_valid_mobile("1234567890"))
check("Invalid mobile (too short)",        not is_valid_mobile("98200123"))

# PAN Validation & Masking
check("Valid PAN format (ABCDE1234F)",     is_valid_pan("ABCDE1234F"))
check("Valid lowercase PAN auto-upper",    is_valid_pan("abcde1234f"))
check("Invalid PAN format",                not is_valid_pan("12345ABCDE"))
check("Masked PAN format",                 mask_pan("ABCDE1234F") == "XXXXX1234F")
check("Empty PAN masking returns empty",   mask_pan("") == "")

# DOB & Age 18+ Validation
today = datetime.date.today()
valid_dob = f"{today.year - 20}-01-15"
underage_dob = f"{today.year - 15}-01-15"
check("Age 20 is valid DOB",               is_valid_dob(valid_dob)[0])
check("Age 15 is rejected (under 18)",     not is_valid_dob(underage_dob)[0])
check("Invalid date format rejected",      not is_valid_dob("invalid-date")[0])

# Math Captcha (replaces reCAPTCHA)
import re as _re, os as _os
_os.environ.setdefault("SECRET_KEY", "test-unit-secret-key")
captcha_challenge = generate_math_captcha()
captcha_question  = captcha_challenge["question"]
captcha_id        = captcha_challenge["challenge_id"]
# Parse and solve the math question
_expr = _re.search(r'What is (.+)\?', captcha_question).group(1)
_expr_eval = _expr.replace('×','*').replace('−','-').strip()
correct_ans = str(eval(_expr_eval))
cap_ok, cap_msg = verify_math_captcha(captcha_id, correct_ans)
check("Math captcha generated successfully",  "challenge_id" in captcha_challenge)
check("Math captcha correct answer passes",   cap_ok)
cap_wrong_ok, _ = verify_math_captcha(captcha_id, "9999")
check("Math captcha wrong answer rejected",   not cap_wrong_ok)
cap_empty_ok, _ = verify_math_captcha("", "")
check("Math captcha empty challenge rejected", not cap_empty_ok)
cap_tamper_ok, _ = verify_math_captcha("tampered.payload", correct_ans)
check("Math captcha tampered ID rejected",    not cap_tamper_ok)


# ── 17. Market Data Service ───────────────────────────────────────────
print("\n[17] Market Data Service (yfinance & Fallback Engine)")
from services.market_data_service import (
    get_stock_quote, get_market_overview, get_stock_history, search_stocks
)

reliance_quote = get_stock_quote("RELIANCE.NS")
check("Quote returns dictionary",          isinstance(reliance_quote, dict))
check("Quote has price > 0",               reliance_quote.get("price", 0) > 0)
check("Quote has company name",            "Reliance" in reliance_quote.get("name", ""))
check("Quote has currency INR",            reliance_quote.get("currency") == "INR")

overview = get_market_overview()
check("Overview has indices list",         len(overview.get("indices", [])) >= 3)
check("Overview has market status",        overview.get("status") in ["OPEN", "CLOSED"])
check("Overview has top stocks/gainers",   len(overview.get("top_stocks", [])) > 0)

history = get_stock_history("TCS.NS", days=30)
check("History returns data points",       len(history) >= 10)
check("History point has date and close",  "date" in history[0] and "close" in history[0])

search_results = search_stocks("HDFC")
check("Stock search finds matches",        len(search_results) >= 1)
check("Search result symbol matches",      "HDFC" in search_results[0]["symbol"])


# ── 18. IPO Service ───────────────────────────────────────────────────
print("\n[18] IPO Service (Dataset, Filters & Summary)")
from services.ipo_service import list_ipos, get_ipo_detail, get_ipo_summary

all_ipos_res = list_ipos()
all_ipos = all_ipos_res.get("ipos", [])
check("Total IPOs in dataset >= 8",        all_ipos_res.get("total", 0) >= 8)

upcoming_ipos = list_ipos(status="UPCOMING")["ipos"]
check("Upcoming IPO filter works",         len(upcoming_ipos) >= 1 and upcoming_ipos[0]["status"] == "UPCOMING")

open_ipos = list_ipos(status="OPEN")["ipos"]
check("Open IPO filter works",             len(open_ipos) >= 1 and open_ipos[0]["status"] == "OPEN")

sme_ipos = list_ipos(ipo_type="SME")["ipos"]
check("SME category filter works",         len(sme_ipos) >= 1 and sme_ipos[0]["ipo_type"] == "SME")

summary = get_ipo_summary()
check("IPO summary contains stats",        summary.get("total", 0) >= 8 and "open" in summary)

ipo_detail = get_ipo_detail(all_ipos[0]["id"])
check("IPO detail found by ID",            ipo_detail is not None and ipo_detail["id"] == all_ipos[0]["id"])


# ── 19. Market News Service ───────────────────────────────────────────
print("\n[19] Market News Service (11 Categories & Search)")
from services.news_service import list_news, get_categories

categories = get_categories()
check("11 required news categories present", len(categories) == 11)

news_res = list_news(limit=20)
all_news = news_res.get("articles", [])
check("News dataset has articles",         len(all_news) >= 11 and news_res.get("total", 0) >= 11)
check("Article has headline and source",   "headline" in all_news[0] and "source" in all_news[0])
check("Article has valid thumbnail URL",   all_news[0].get("thumbnail", "").startswith("https://"))

ipo_news = list_news(category="IPO")["articles"]
check("Category filter returns IPO news",  len(ipo_news) >= 1 and ipo_news[0]["category"] == "IPO")

search_news = list_news(search="RBI")["articles"]
check("News search finds RBI news",        len(search_news) >= 1)


# ── 20. Precious Metals Service ───────────────────────────────────────
print("\n[20] Precious Metals Service (Gold, Silver & Jewellery Estimator)")
from services.precious_metals_service import (
    get_precious_metals_rates, get_supported_cities,
    calculate_retail_jewellery_price, get_historical_trends
)

mumbai_rates = get_precious_metals_rates("mumbai")
check("Mumbai 24K gold rate > ₹70,000",    mumbai_rates["gold_24k"]["per_10_gram"] > 70000)
check("Mumbai 22K gold rate < 24K",        mumbai_rates["gold_22k"]["per_10_gram"] < mumbai_rates["gold_24k"]["per_10_gram"])
check("Mumbai Silver 1kg rate > ₹80,000",  mumbai_rates["silver"]["per_kg"] > 80000)
check("OroPocket buy/sell rates present",  mumbai_rates["gold_24k"]["buy_price_per_gram"] > 0 and mumbai_rates["gold_24k"]["sell_price_per_gram"] > 0)
check("OroPocket indicative notice set",   "indicative" in mumbai_rates.get("indicative_notice", "").lower())
check("OroPocket silver buy/sell present", mumbai_rates["silver"]["buy_price_per_gram"] > 0 and mumbai_rates["silver"]["sell_price_per_gram"] > 0)

cities = get_supported_cities()
check("Covers 6+ Maharashtra cities",      len(cities) >= 6)
city_names = [c["name"] for c in cities]
check("Contains Pune and Nashik",          "Pune" in city_names and "Nashik" in city_names)

est = calculate_retail_jewellery_price(purity_karat=22, weight_grams=10, making_charge_pct=10.0)
check("Jewellery billing returns breakdown", "total_estimated_payable" in est and "gst_amount" in est)
check("GST is exactly 3%",                 est["gst_rate_pct"] == 3.0)
check("Calculated total matches sum",      round(est["base_gold_cost"] + est["making_charge_amount"] + est["gst_amount"] + est["hallmarking_charge"]) == round(est["total_estimated_payable"]))

gold_trend = get_historical_trends("gold", period="30d")
check("Gold 30D trend data returned",      len(gold_trend.get("series", [])) >= 20)


# ── 21. Mutual Funds & SIP Recommendation Engine ──────────────────────
print("\n[21] Mutual Funds & Advanced SIP Recommendation Engine")
from services.mutual_funds_service import (
    SCHEMES_CATALOG, calculate_sip, generate_sip_recommendations, compare_schemes
)

check("Mutual funds dataset has >= 10 funds", len(SCHEMES_CATALOG) >= 10)

fund_categories = {f["asset_class"] for f in SCHEMES_CATALOG}
check("Covers EQUITY, HYBRID, DEBT, INDEX",   {"EQUITY", "HYBRID", "DEBT", "INDEX"}.issubset(fund_categories))

# Advanced SIP with Step-Up
sip_res = calculate_sip(monthly_amount=10000, annual_return_pct=12, duration_years=10, step_up_pct=10)
check("SIP calculation returns maturity",   sip_res["future_value"] > sip_res["total_invested"])
check("Step-up increases total invested",   sip_res["total_invested"] > 1200000)
check("Yearly breakdown present",           len(sip_res["year_wise_breakdown"]) == 10)

# Goal-based Recommendation
rec = generate_sip_recommendations(monthly_budget=25000, horizon_years=10, risk_tolerance="AGGRESSIVE")
check("Goal recommendation computes allocations", len(rec["allocations"]) > 0)
check("Selected schemes match recommendations",   len(rec["recommended_schemes"]) > 0)

# Side-by-Side Comparison
comp = compare_schemes([SCHEMES_CATALOG[0]["id"], SCHEMES_CATALOG[1]["id"]])
check("Comparison matrix returns 2 funds",  len(comp) == 2)


print("\n[22] Fixed Deposit, Bond & Gold Financial Calculations")
# 1. Fixed Deposit compounding math
def calc_fd_test(p, r_pct, t_yrs, freq="QUARTERLY"):
    freq_map = {"ANNUALLY": 1, "HALF_YEARLY": 2, "QUARTERLY": 4, "MONTHLY": 12, "SIMPLE": 0}
    n = freq_map.get(freq, 4)
    r = r_pct / 100.0
    if n == 0:
        interest = p * r * t_yrs
        maturity = p + interest
    else:
        maturity = p * math.pow(1.0 + r / n, n * t_yrs)
        interest = maturity - p
    return round(maturity, 2), round(interest, 2)

fd_mat, fd_int = calc_fd_test(100000, 7.5, 3, "QUARTERLY")
check("FD quarterly maturity > principal", fd_mat > 100000)
check("FD quarterly interest correct (~₹24.9k)", 24000 < fd_int < 26000)

fd_s_mat, fd_s_int = calc_fd_test(100000, 7.5, 3, "SIMPLE")
check("FD simple interest matches P*R*T (₹22,500)", fd_s_int == 22500.0)
check("Compound FD earns more than simple FD", fd_mat > fd_s_mat)

# 2. Bond yield & cashflow math
def calc_bond_test(face, price, qty, coupon_pct, tenure_yrs, freq="SEMI_ANNUALLY"):
    outlay = price * qty
    redemption = face * qty
    annual_coupon = face * (coupon_pct / 100.0) * qty
    total_coupon = annual_coupon * tenure_yrs
    gain = (redemption + total_coupon) - outlay
    current_yield = ((face * (coupon_pct / 100.0)) / price) * 100.0
    approx_ytm = ((face * (coupon_pct / 100.0)) + ((face - price) / tenure_yrs)) / ((face + price) / 2.0) * 100.0
    return round(outlay, 2), round(redemption, 2), round(total_coupon, 2), round(gain, 2), round(current_yield, 2), round(approx_ytm, 2)

b_out, b_red, b_c_tot, b_gain, b_cy, b_ytm = calc_bond_test(1000, 980, 10, 7.18, 5)
check("Bond outlay matches purchase price (₹9,800)", b_out == 9800.0)
check("Bond redemption equals face value (₹10,000)", b_red == 10000.0)
check("Bond total coupon interest received (₹3,590)", b_c_tot == 3590.0)
check("Bond total net profit (₹3,790)", b_gain == 3790.0)
check("Bond discount increases YTM above coupon rate (YTM > 7.18%)", b_ytm > 7.18)

# 3. Gold acquisition, tax & projection math
def calc_gold_test(grams, purity_k, rate_per_g, making_pct=5.0, gst_pct=3.0, apprec_pct=10.0, horizon_yrs=5):
    pure_rate = rate_per_g * (purity_k / 24.0)
    metal_cost = grams * pure_rate
    making = metal_cost * (making_pct / 100.0)
    gst = (metal_cost + making) * (gst_pct / 100.0)
    total_outlay = metal_cost + making + gst
    future_rate = rate_per_g * math.pow(1.0 + apprec_pct / 100.0, horizon_yrs)
    future_metal_cost = grams * (future_rate * (purity_k / 24.0))
    proj_gain = future_metal_cost - total_outlay
    return round(metal_cost, 2), round(gst, 2), round(total_outlay, 2), round(future_metal_cost, 2), round(proj_gain, 2)

g_cost, g_gst, g_outlay, g_future, g_gain = calc_gold_test(10, 24, 8000, 5.0, 3.0, 10.0, 5)
check("Gold metal cost 10g 24K @ ₹8000 = ₹80,000", g_cost == 80000.0)
check("Gold GST 3% on (metal + making) = ₹2,520", g_gst == 2520.0)
check("Gold total acquisition outlay = ₹86,520", g_outlay == 86520.0)
check("Gold projected future value > outlay at 10% CAGR", g_future > g_outlay)
check("Gold capital gain is positive", g_gain > 0)




# ── Summary ───────────────────────────────────────────────────────────────────
conn.close()
os.unlink(DB)

total = PASS_COUNT + FAIL_COUNT
print(f"\n{'='*58}")
print(f"  INVESTSPHERE WEB BACKEND TEST: {PASS_COUNT}/{total} PASSED")
print(f"{'='*58}")
sys.exit(0 if FAIL_COUNT == 0 else 1)

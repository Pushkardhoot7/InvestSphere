-- ============================================================================
-- InvestSphere — Smart Investment & Portfolio Management System
-- Schema Definition: SQLite Relational Database (Targeting 3NF)
-- File: sql/schema.sql
-- ============================================================================

-- Enforce SQLite Foreign Key constraints on every execution
PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
-- 1. Schema Versioning & Migrations
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS schema_migrations (
    migration_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    version         INTEGER NOT NULL UNIQUE,
    description     TEXT NOT NULL,
    applied_at      TEXT NOT NULL DEFAULT (datetime('now'))
);

-- ----------------------------------------------------------------------------
-- 2. Users Table
-- Supports multi-user isolation with secure password hashing.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    user_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name       TEXT NOT NULL,
    email           TEXT NOT NULL UNIQUE COLLATE NOCASE,
    password_hash   TEXT NOT NULL,
    account_status  TEXT NOT NULL DEFAULT 'ACTIVE' 
                    CHECK (account_status IN ('ACTIVE', 'SUSPENDED', 'CLOSED')),
    mobile_number   TEXT,
    country         TEXT DEFAULT 'India',
    state           TEXT DEFAULT 'Maharashtra',
    pan_masked      TEXT,
    date_of_birth   TEXT,
    created_at      TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at      TEXT NOT NULL DEFAULT (datetime('now'))
);

-- ----------------------------------------------------------------------------
-- 3. Investment Types Table (Lookup Table)
-- Standard asset class classifications.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS investment_types (
    investment_type_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    type_code           TEXT NOT NULL UNIQUE 
                        CHECK (type_code IN ('STOCK', 'MUTUAL_FUND', 'GOLD', 'FIXED_DEPOSIT', 'BOND', 'CASH')),
    type_name           TEXT NOT NULL UNIQUE,
    description         TEXT,
    is_active           INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1))
);

-- ----------------------------------------------------------------------------
-- 4. Financial Instruments Catalog Table
-- Central 3NF catalog distinguishing market instruments from user holdings.
-- Prevents duplicating company/ticker descriptions across multiple users.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS instruments (
    instrument_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    investment_type_id  INTEGER NOT NULL,
    symbol              TEXT NOT NULL UNIQUE COLLATE NOCASE,
    instrument_name     TEXT NOT NULL,
    currency            TEXT NOT NULL DEFAULT 'USD',
    exchange            TEXT,
    description         TEXT,
    is_active           INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1)),
    created_at          TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (investment_type_id) REFERENCES investment_types(investment_type_id) 
        ON DELETE RESTRICT ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 5. User Investments Table (Holdings)
-- Links a specific user to an instrument with portfolio metadata.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS investments (
    investment_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id             INTEGER NOT NULL,
    instrument_id       INTEGER NOT NULL,
    portfolio_tag       TEXT NOT NULL DEFAULT 'DEFAULT',
    notes               TEXT,
    status              TEXT NOT NULL DEFAULT 'ACTIVE' 
                        CHECK (status IN ('ACTIVE', 'CLOSED', 'WATCHLIST')),
    created_at          TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at          TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE RESTRICT ON UPDATE CASCADE,
    UNIQUE (user_id, instrument_id, portfolio_tag)
);

-- ----------------------------------------------------------------------------
-- 6. Stocks Specialization Table
-- Stores stock-specific attributes extending the instruments table (1-to-1).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS stocks (
    instrument_id       INTEGER PRIMARY KEY,
    isin                TEXT UNIQUE,
    sector              TEXT,
    industry            TEXT,
    market_cap_category TEXT CHECK (market_cap_category IN ('LARGE_CAP', 'MID_CAP', 'SMALL_CAP', 'MICRO_CAP')),
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 7. Mutual Funds Specialization Table
-- Stores mutual fund-specific attributes extending the instruments table (1-to-1).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS mutual_funds (
    instrument_id       INTEGER PRIMARY KEY,
    amc_name            TEXT NOT NULL,
    fund_category       TEXT NOT NULL 
                        CHECK (fund_category IN ('EQUITY', 'DEBT', 'HYBRID', 'INDEX', 'ELSS', 'LIQUID')),
    plan_type           TEXT NOT NULL DEFAULT 'GROWTH' 
                        CHECK (plan_type IN ('GROWTH', 'DIVIDEND_PAYOUT', 'DIVIDEND_REINVEST')),
    benchmark_index     TEXT,
    expense_ratio       REAL CHECK (expense_ratio IS NULL OR (expense_ratio >= 0.0 AND expense_ratio <= 10.0)),
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 8. Gold Holdings Specialization Table
-- Stores gold-specific attributes extending user investments (1-to-1).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS gold_holdings (
    investment_id       INTEGER PRIMARY KEY,
    gold_form           TEXT NOT NULL 
                        CHECK (gold_form IN ('PHYSICAL_COIN', 'PHYSICAL_BAR', 'DIGITAL_GOLD', 'SOVEREIGN_BOND', 'GOLD_ETF')),
    purity_karat        REAL NOT NULL CHECK (purity_karat > 0.0 AND purity_karat <= 24.0),
    storage_location    TEXT,
    FOREIGN KEY (investment_id) REFERENCES investments(investment_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 9. Fixed Deposits Specialization Table
-- Stores bank FD deposit attributes extending user investments (1-to-1).
-- Monetary values stored in minor units (integer cents/paise).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS fixed_deposits (
    investment_id               INTEGER PRIMARY KEY,
    institution_name            TEXT NOT NULL,
    account_number              TEXT,
    principal_amount            INTEGER NOT NULL CHECK (principal_amount > 0),
    annual_interest_rate        REAL NOT NULL CHECK (annual_interest_rate >= 0.0 AND annual_interest_rate <= 100.0),
    compounding_frequency       TEXT NOT NULL 
                                CHECK (compounding_frequency IN ('MONTHLY', 'QUARTERLY', 'HALF_YEARLY', 'ANNUALLY', 'SIMPLE')),
    payout_type                 TEXT NOT NULL 
                                CHECK (payout_type IN ('CUMULATIVE', 'PERIODIC_MONTHLY', 'PERIODIC_QUARTERLY', 'PERIODIC_ANNUALLY')),
    start_date                  TEXT NOT NULL,
    maturity_date               TEXT NOT NULL,
    estimated_maturity_amount   INTEGER NOT NULL CHECK (estimated_maturity_amount >= principal_amount),
    status                      TEXT NOT NULL DEFAULT 'ACTIVE' 
                                CHECK (status IN ('ACTIVE', 'MATURED', 'PREMATURE_WITHDRAWN')),
    FOREIGN KEY (investment_id) REFERENCES investments(investment_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    CHECK (maturity_date > start_date)
);

-- ----------------------------------------------------------------------------
-- 10. Bonds Specialization Table
-- Stores bond-specific attributes extending user investments (1-to-1).
-- Monetary values stored in minor units (integer cents/paise).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS bonds (
    investment_id       INTEGER PRIMARY KEY,
    issuer_name         TEXT NOT NULL,
    bond_type           TEXT NOT NULL 
                        CHECK (bond_type IN ('GOVERNMENT', 'CORPORATE', 'MUNICIPAL', 'SOVEREIGN_GOLD')),
    face_value          INTEGER NOT NULL CHECK (face_value > 0),
    coupon_rate         REAL NOT NULL CHECK (coupon_rate >= 0.0 AND coupon_rate <= 100.0),
    coupon_frequency    TEXT NOT NULL 
                        CHECK (coupon_frequency IN ('MONTHLY', 'QUARTERLY', 'SEMI_ANNUALLY', 'ANNUALLY', 'ZERO_COUPON')),
    issue_date          TEXT NOT NULL,
    maturity_date       TEXT NOT NULL,
    credit_rating       TEXT,
    status              TEXT NOT NULL DEFAULT 'ACTIVE' 
                        CHECK (status IN ('ACTIVE', 'REDEEMED', 'DEFAULTED')),
    FOREIGN KEY (investment_id) REFERENCES investments(investment_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    CHECK (maturity_date > issue_date)
);

-- ----------------------------------------------------------------------------
-- 11. Unified Transactions Ledger
-- Records all asset movements (Buy, Sell, Deposit, Withdrawal, Dividend, etc.).
-- Monetary values stored in minor units (integer cents/paise).
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id             INTEGER NOT NULL,
    investment_id       INTEGER,
    transaction_type    TEXT NOT NULL 
                        CHECK (transaction_type IN ('BUY', 'SELL', 'DEPOSIT', 'WITHDRAWAL', 'DIVIDEND', 'INTEREST', 'FEE', 'REDEMPTION')),
    transaction_date    TEXT NOT NULL,
    quantity            REAL CHECK (quantity IS NULL OR quantity > 0.0),
    unit_price          INTEGER CHECK (unit_price IS NULL OR unit_price >= 0),
    gross_amount        INTEGER NOT NULL CHECK (gross_amount >= 0),
    fees                INTEGER NOT NULL DEFAULT 0 CHECK (fees >= 0),
    taxes               INTEGER NOT NULL DEFAULT 0 CHECK (taxes >= 0),
    net_amount          INTEGER NOT NULL CHECK (net_amount >= 0),
    notes               TEXT,
    created_at          TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (investment_id) REFERENCES investments(investment_id) 
        ON DELETE RESTRICT ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 12. Market Prices History Table
-- Historical spot prices for stocks, ETFs, and commodities.
-- Preserves timestamped observations; prevents duplicate quotes per instrument and time.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS market_prices (
    price_id                INTEGER PRIMARY KEY AUTOINCREMENT,
    instrument_id           INTEGER NOT NULL,
    price                   INTEGER NOT NULL CHECK (price >= 0),
    observation_timestamp   TEXT NOT NULL,
    source                  TEXT NOT NULL DEFAULT 'MANUAL',
    currency                TEXT NOT NULL DEFAULT 'USD',
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (instrument_id, observation_timestamp)
);

-- ----------------------------------------------------------------------------
-- 13. Mutual Fund NAV History Table
-- Historical Net Asset Values for mutual fund schemes.
-- Preserves daily NAV observations; prevents duplicate quotes per scheme and date.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS mutual_fund_nav (
    nav_id              INTEGER PRIMARY KEY AUTOINCREMENT,
    instrument_id       INTEGER NOT NULL,
    nav_value           REAL NOT NULL CHECK (nav_value > 0.0),
    observation_date    TEXT NOT NULL,
    source              TEXT NOT NULL DEFAULT 'AMFI',
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (instrument_id, observation_date)
);

-- ----------------------------------------------------------------------------
-- 14. SIP Plans Table (Systematic Investment Plans)
-- User financial calculation plans and automated savings projections.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS sip_plans (
    sip_plan_id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id                     INTEGER NOT NULL,
    instrument_id               INTEGER,
    plan_name                   TEXT NOT NULL,
    monthly_amount              INTEGER NOT NULL CHECK (monthly_amount > 0),
    expected_annual_return      REAL NOT NULL CHECK (expected_annual_return >= -100.0 AND expected_annual_return <= 100.0),
    duration_months             INTEGER NOT NULL CHECK (duration_months > 0),
    start_date                  TEXT NOT NULL,
    end_date                    TEXT NOT NULL,
    step_up_percent             REAL NOT NULL DEFAULT 0.0 CHECK (step_up_percent >= 0),
    contribution_day_of_month   INTEGER NOT NULL DEFAULT 1 CHECK (contribution_day_of_month BETWEEN 1 AND 31),
    projected_invested_capital  INTEGER NOT NULL CHECK (projected_invested_capital > 0),
    projected_future_value      INTEGER NOT NULL CHECK (projected_future_value >= 0),
    status                      TEXT NOT NULL DEFAULT 'ACTIVE' 
                                CHECK (status IN ('ACTIVE', 'PAUSED', 'COMPLETED', 'CANCELLED')),
    created_at                  TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE SET NULL ON UPDATE CASCADE,
    CHECK (end_date >= start_date)
);

-- ----------------------------------------------------------------------------
-- 15. SWP Plans Table (Systematic Withdrawal Plans)
-- User financial retirement cash flow and withdrawal longevity projections.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS swp_plans (
    swp_plan_id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id                     INTEGER NOT NULL,
    instrument_id               INTEGER,
    plan_name                   TEXT NOT NULL,
    initial_corpus              INTEGER NOT NULL CHECK (initial_corpus > 0),
    monthly_withdrawal          INTEGER NOT NULL CHECK (monthly_withdrawal > 0),
    expected_annual_return      REAL NOT NULL CHECK (expected_annual_return >= -100.0 AND expected_annual_return <= 100.0),
    duration_months             INTEGER NOT NULL CHECK (duration_months > 0),
    start_date                  TEXT NOT NULL,
    withdrawal_day_of_month     INTEGER NOT NULL DEFAULT 1 CHECK (withdrawal_day_of_month BETWEEN 1 AND 31),
    projected_final_balance     INTEGER NOT NULL CHECK (projected_final_balance >= 0),
    status                      TEXT NOT NULL DEFAULT 'ACTIVE' 
                                CHECK (status IN ('ACTIVE', 'PAUSED', 'COMPLETED', 'CANCELLED')),
    created_at                  TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (instrument_id) REFERENCES instruments(instrument_id) 
        ON DELETE SET NULL ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 16. Portfolio Snapshots Table
-- Historical snapshots of portfolio valuation for performance charting.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS portfolio_snapshots (
    snapshot_id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id                     INTEGER NOT NULL,
    snapshot_timestamp          TEXT NOT NULL DEFAULT (datetime('now')),
    total_invested              INTEGER NOT NULL CHECK (total_invested >= 0),
    estimated_portfolio_value   INTEGER NOT NULL CHECK (estimated_portfolio_value >= 0),
    realized_profit_loss        INTEGER NOT NULL,
    unrealized_profit_loss      INTEGER NOT NULL,
    notes                       TEXT,
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 17. Financial Events Table
-- Upcoming maturities, coupon payments, and planned cash flow events.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS financial_events (
    event_id            INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id             INTEGER NOT NULL,
    investment_id       INTEGER,
    event_type          TEXT NOT NULL 
                        CHECK (event_type IN ('FD_MATURITY', 'BOND_MATURITY', 'BOND_COUPON', 'SIP_INSTALLMENT', 'SWP_WITHDRAWAL', 'DIVIDEND_EXPECTED')),
    event_date          TEXT NOT NULL,
    amount              INTEGER CHECK (amount IS NULL OR amount >= 0),
    status              TEXT NOT NULL DEFAULT 'PENDING' 
                        CHECK (status IN ('PENDING', 'PROCESSED', 'CANCELLED', 'OVERDUE')),
    description         TEXT NOT NULL,
    created_at          TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (investment_id) REFERENCES investments(investment_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (user_id, investment_id, event_type, event_date)
);

-- ----------------------------------------------------------------------------
-- 18. Audit Logs Table
-- Immutable audit ledger recording data changes for accountability.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS audit_logs (
    audit_id            INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id             INTEGER,
    action_type         TEXT NOT NULL 
                        CHECK (action_type IN ('INSERT', 'UPDATE', 'DELETE', 'LOGIN', 'LOGOUT', 'EXPORT', 'BACKUP')),
    entity_type         TEXT NOT NULL,
    entity_id           INTEGER,
    action_timestamp    TEXT NOT NULL DEFAULT (datetime('now')),
    description         TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE SET NULL ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 19. User Watchlists Table
-- Tracks custom stocks, mutual funds, or commodities watched by individual users.
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS user_watchlists (
    watchlist_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id             INTEGER NOT NULL,
    symbol              TEXT NOT NULL,
    instrument_name     TEXT,
    asset_type          TEXT NOT NULL DEFAULT 'STOCK',
    added_at            TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE,
    UNIQUE (user_id, symbol)
);

CREATE INDEX IF NOT EXISTS idx_watchlists_user_symbol 
    ON user_watchlists(user_id, symbol);

-- ============================================================================
-- PERFORMANCE INDEXES
-- Indexing high-frequency foreign keys, dates, and query filter columns.
-- ============================================================================

CREATE INDEX IF NOT EXISTS idx_users_email 
    ON users(email);

CREATE INDEX IF NOT EXISTS idx_instruments_type 
    ON instruments(investment_type_id);

CREATE INDEX IF NOT EXISTS idx_instruments_symbol 
    ON instruments(symbol);

CREATE INDEX IF NOT EXISTS idx_investments_user 
    ON investments(user_id);

CREATE INDEX IF NOT EXISTS idx_investments_instrument 
    ON investments(instrument_id);

CREATE INDEX IF NOT EXISTS idx_transactions_user 
    ON transactions(user_id);

CREATE INDEX IF NOT EXISTS idx_transactions_investment 
    ON transactions(investment_id);

CREATE INDEX IF NOT EXISTS idx_transactions_date 
    ON transactions(transaction_date);

CREATE INDEX IF NOT EXISTS idx_transactions_type 
    ON transactions(transaction_type);

CREATE INDEX IF NOT EXISTS idx_market_prices_instrument_time 
    ON market_prices(instrument_id, observation_timestamp DESC);

CREATE INDEX IF NOT EXISTS idx_mutual_fund_nav_instrument_date 
    ON mutual_fund_nav(instrument_id, observation_date DESC);

CREATE INDEX IF NOT EXISTS idx_sip_plans_user 
    ON sip_plans(user_id);

CREATE INDEX IF NOT EXISTS idx_swp_plans_user 
    ON swp_plans(user_id);

CREATE INDEX IF NOT EXISTS idx_portfolio_snapshots_user_time 
    ON portfolio_snapshots(user_id, snapshot_timestamp DESC);

CREATE INDEX IF NOT EXISTS idx_financial_events_user_date 
    ON financial_events(user_id, event_date);

CREATE INDEX IF NOT EXISTS idx_audit_logs_user_time 
    ON audit_logs(user_id, action_timestamp DESC);

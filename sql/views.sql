-- ============================================================================
-- InvestSphere — Smart Investment & Portfolio Management System
-- Database Views: Analytical & Reporting Views
-- File: sql/views.sql
-- ============================================================================

-- ----------------------------------------------------------------------------
-- View 1: vw_latest_market_prices
-- Retrieves the most recent market price observation for every instrument.
-- Uses a correlated subquery on observation_timestamp.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_latest_market_prices;
CREATE VIEW vw_latest_market_prices AS
SELECT 
    mp.instrument_id,
    inst.symbol,
    inst.instrument_name,
    inst.currency,
    mp.price AS latest_price,
    mp.observation_timestamp AS latest_timestamp,
    mp.source
FROM market_prices mp
JOIN instruments inst ON mp.instrument_id = inst.instrument_id
WHERE mp.observation_timestamp = (
    SELECT MAX(sub_mp.observation_timestamp)
    FROM market_prices sub_mp
    WHERE sub_mp.instrument_id = mp.instrument_id
);

-- ----------------------------------------------------------------------------
-- View 2: vw_latest_mutual_fund_nav
-- Retrieves the most recent NAV observation for every mutual fund scheme.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_latest_mutual_fund_nav;
CREATE VIEW vw_latest_mutual_fund_nav AS
SELECT 
    mfn.instrument_id,
    inst.symbol AS scheme_code,
    inst.instrument_name AS scheme_name,
    mfn.nav_value AS latest_nav,
    mfn.observation_date AS latest_nav_date,
    mfn.source
FROM mutual_fund_nav mfn
JOIN instruments inst ON mfn.instrument_id = inst.instrument_id
WHERE mfn.observation_date = (
    SELECT MAX(sub_nav.observation_date)
    FROM mutual_fund_nav sub_nav
    WHERE sub_nav.instrument_id = mfn.instrument_id
);

-- ----------------------------------------------------------------------------
-- View 3: vw_active_holdings
-- Calculates current remaining quantity and net acquisition cost for each
-- active investment holding by aggregating BUY and SELL transactions.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_active_holdings;
CREATE VIEW vw_active_holdings AS
SELECT 
    inv.investment_id,
    inv.user_id,
    u.full_name AS user_name,
    inv.instrument_id,
    inst.symbol,
    inst.instrument_name,
    it.type_code AS asset_type,
    inv.portfolio_tag,
    -- Net remaining quantity: sum of BUYs/DEPOSITs minus sum of SELLs/REDEMPTIONs/WITHDRAWALs
    ROUND(COALESCE(SUM(CASE 
        WHEN t.transaction_type IN ('BUY', 'DEPOSIT') THEN t.quantity 
        WHEN t.transaction_type IN ('SELL', 'REDEMPTION', 'WITHDRAWAL') THEN -t.quantity 
        ELSE 0 
    END), 0), 4) AS remaining_quantity,
    -- Total invested acquisition cost for active holdings (minor units)
    COALESCE(SUM(CASE 
        WHEN t.transaction_type IN ('BUY', 'DEPOSIT') THEN t.net_amount 
        WHEN t.transaction_type IN ('SELL', 'REDEMPTION', 'WITHDRAWAL') THEN -t.net_amount 
        ELSE 0 
    END), 0) AS net_invested_amount,
    -- Cumulative transaction fees
    COALESCE(SUM(t.fees), 0) AS total_fees_paid,
    COUNT(t.transaction_id) AS total_transactions_count,
    inv.status AS holding_status
FROM investments inv
JOIN users u ON inv.user_id = u.user_id
JOIN instruments inst ON inv.instrument_id = inst.instrument_id
JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
LEFT JOIN transactions t ON inv.investment_id = t.investment_id
GROUP BY inv.investment_id, inv.user_id, inv.instrument_id, inst.symbol, inst.instrument_name, it.type_code, inv.portfolio_tag, inv.status;

-- ----------------------------------------------------------------------------
-- View 4: vw_transaction_ledger
-- Human-readable complete ledger combining transactions, instruments, and users.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_transaction_ledger;
CREATE VIEW vw_transaction_ledger AS
SELECT 
    t.transaction_id,
    t.user_id,
    u.full_name AS user_name,
    t.investment_id,
    inst.symbol,
    inst.instrument_name,
    it.type_code AS asset_type,
    t.transaction_type,
    t.transaction_date,
    t.quantity,
    t.unit_price,
    t.gross_amount,
    t.fees,
    t.taxes,
    t.net_amount,
    t.notes,
    t.created_at
FROM transactions t
JOIN users u ON t.user_id = u.user_id
LEFT JOIN investments inv ON t.investment_id = inv.investment_id
LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id
LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id;

-- ----------------------------------------------------------------------------
-- View 5: vw_portfolio_summary
-- Consolidated overview of capital allocation grouped by user and asset category.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_portfolio_summary;
CREATE VIEW vw_portfolio_summary AS
SELECT 
    u.user_id,
    u.full_name AS user_name,
    it.type_code AS asset_type,
    it.type_name AS asset_type_name,
    COUNT(DISTINCT inv.investment_id) AS active_holdings_count,
    COALESCE(SUM(CASE 
        WHEN t.transaction_type IN ('BUY', 'DEPOSIT') THEN t.net_amount 
        WHEN t.transaction_type IN ('SELL', 'REDEMPTION', 'WITHDRAWAL') THEN -t.net_amount 
        ELSE 0 
    END), 0) AS category_invested_amount
FROM users u
CROSS JOIN investment_types it
LEFT JOIN instruments inst ON it.investment_type_id = inst.investment_type_id
LEFT JOIN investments inv ON u.user_id = inv.user_id AND inst.instrument_id = inv.instrument_id AND inv.status = 'ACTIVE'
LEFT JOIN transactions t ON inv.investment_id = t.investment_id
GROUP BY u.user_id, u.full_name, it.type_code, it.type_name;

-- ----------------------------------------------------------------------------
-- View 6: vw_upcoming_maturities
-- Filters pending financial events occurring over the next 90 days.
-- ----------------------------------------------------------------------------
DROP VIEW IF EXISTS vw_upcoming_maturities;
CREATE VIEW vw_upcoming_maturities AS
SELECT 
    fe.event_id,
    fe.user_id,
    u.full_name AS user_name,
    fe.investment_id,
    inst.symbol,
    inst.instrument_name,
    fe.event_type,
    fe.event_date,
    fe.amount,
    fe.status,
    fe.description,
    CAST((julianday(fe.event_date) - julianday(date('now'))) AS INTEGER) AS days_remaining
FROM financial_events fe
JOIN users u ON fe.user_id = u.user_id
LEFT JOIN investments inv ON fe.investment_id = inv.investment_id
LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id
WHERE fe.status = 'PENDING'
ORDER BY fe.event_date ASC;

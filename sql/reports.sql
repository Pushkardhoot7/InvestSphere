-- ============================================================================
-- InvestSphere — Smart Investment & Portfolio Management System
-- Demonstrative SQL Reports & Analytical Queries (Academic DBMS Demonstration)
-- File: sql/reports.sql
-- ============================================================================

-- ============================================================================
-- SECTION A: CORE DBMS SQL OPERATIONS DEMONSTRATION
-- ============================================================================

-- 1. SELECT, WHERE, ORDER BY
-- Retrieves all active users ordered by signup date.
SELECT user_id, full_name, email, account_status, created_at
FROM users
WHERE account_status = 'ACTIVE'
ORDER BY created_at DESC;

-- 2. INSERT (Prepared format demonstration)
-- Adding a new market instrument.
-- INSERT INTO instruments (investment_type_id, symbol, instrument_name, currency, exchange)
-- VALUES (1, 'GOOGL', 'Alphabet Inc. Class A', 'USD', 'NASDAQ');

-- 3. UPDATE with Arithmetic Expression
-- Updating note and timestamp on an investment.
-- UPDATE investments 
-- SET notes = 'Updated core position thesis', updated_at = datetime('now')
-- WHERE investment_id = 1;

-- 4. DELETE with Cascading Integrity Check
-- Attempting deletion of an unused watchlist item.
-- DELETE FROM investments WHERE investment_id = 999 AND status = 'WATCHLIST';

-- 5. INNER JOIN
-- Combines active investments with their underlying instrument specifications.
SELECT 
    inv.investment_id,
    u.full_name AS owner_name,
    inst.symbol,
    inst.instrument_name,
    it.type_name AS asset_category
FROM investments inv
INNER JOIN users u ON inv.user_id = u.user_id
INNER JOIN instruments inst ON inv.instrument_id = inst.instrument_id
INNER JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
WHERE inv.status = 'ACTIVE';

-- 6. LEFT JOIN
-- Shows all instruments and includes their latest market price if available.
SELECT 
    inst.instrument_id,
    inst.symbol,
    inst.instrument_name,
    COALESCE(lmp.latest_price, 0) AS latest_price_cents,
    lmp.latest_timestamp
FROM instruments inst
LEFT JOIN vw_latest_market_prices lmp ON inst.instrument_id = lmp.instrument_id
ORDER BY inst.symbol ASC;

-- 7. GROUP BY, HAVING, and Aggregate Functions (SUM, AVG, COUNT, MIN, MAX)
-- Calculates investment metrics per asset type where total investment exceeds $1,000 (100,000 cents).
SELECT 
    it.type_code AS asset_type,
    COUNT(t.transaction_id) AS total_transactions,
    ROUND(AVG(t.net_amount), 2) AS average_transaction_amount_cents,
    MIN(t.net_amount) AS smallest_transaction_cents,
    MAX(t.net_amount) AS largest_transaction_cents,
    SUM(CASE WHEN t.transaction_type = 'BUY' THEN t.net_amount ELSE 0 END) AS total_capital_deployed_cents
FROM transactions t
JOIN investments inv ON t.investment_id = inv.investment_id
JOIN instruments inst ON inv.instrument_id = inst.instrument_id
JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
GROUP BY it.type_code
HAVING total_capital_deployed_cents > 100000;

-- 8. Subqueries (Correlated and Non-correlated)
-- Finds investments whose total invested amount is higher than the user's average investment.
SELECT 
    inv.investment_id,
    inst.symbol,
    inst.instrument_name,
    sub.invested_amount_cents
FROM investments inv
JOIN instruments inst ON inv.instrument_id = inst.instrument_id
JOIN (
    SELECT investment_id, SUM(net_amount) AS invested_amount_cents
    FROM transactions
    WHERE transaction_type = 'BUY'
    GROUP BY investment_id
) sub ON inv.investment_id = sub.investment_id
WHERE sub.invested_amount_cents > (
    SELECT AVG(total_buy)
    FROM (
        SELECT SUM(net_amount) AS total_buy
        FROM transactions
        WHERE transaction_type = 'BUY'
        GROUP BY investment_id
    )
);

-- 9. Transaction Processing (ACID Guarantees)
-- BEGIN TRANSACTION;
-- INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date, quantity, unit_price, gross_amount, fees, taxes, net_amount, notes)
-- VALUES (1, 1, 'BUY', datetime('now'), 10, 20000, 200000, 500, 0, 200500, 'ACID trade execution');
-- COMMIT;
-- (Or ROLLBACK if validation fails)


-- ============================================================================
-- SECTION B: PRACTICAL FINANCIAL & ACADEMIC COURSE REPORTS
-- ============================================================================

-- ----------------------------------------------------------------------------
-- Report 1: Consolidated Portfolio Valuation & Unrealized P&L
-- Computes true current market value vs acquisition cost for equity & mutual funds.
-- ----------------------------------------------------------------------------
SELECT 
    h.investment_id,
    h.user_id,
    h.symbol,
    h.instrument_name,
    h.asset_type,
    h.remaining_quantity,
    h.net_invested_amount AS total_cost_cents,
    CASE 
        WHEN h.remaining_quantity > 0 THEN ROUND(h.net_invested_amount / h.remaining_quantity, 2)
        ELSE 0
    END AS avg_cost_per_unit_cents,
    COALESCE(lmp.latest_price, CAST(ROUND(COALESCE(nav.latest_nav, 0) * 100) AS INTEGER), 0) AS current_price_cents,
    CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER) AS estimated_current_value_cents,
    -- Unrealized Gain/Loss = Estimated Current Value - Total Cost
    CAST(ROUND((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) AS INTEGER) AS unrealized_pnl_cents,
    -- Unrealized Return Percentage
    CASE 
        WHEN h.net_invested_amount > 0 THEN 
            ROUND((((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) * 100.0) / h.net_invested_amount, 2)
        ELSE 0.0
    END AS unrealized_return_pct
FROM vw_active_holdings h
LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id
LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id
WHERE h.remaining_quantity > 0;

-- ----------------------------------------------------------------------------
-- Report 2: Realized Capital Gains & Closed Trades Analysis
-- Demonstrates Weighted Average Cost basis tracking for closed sales.
-- ----------------------------------------------------------------------------
SELECT 
    t.transaction_id,
    t.user_id,
    inst.symbol,
    inst.instrument_name,
    t.transaction_date AS sale_date,
    t.quantity AS sold_quantity,
    t.unit_price AS sale_price_cents,
    t.gross_amount AS sale_proceeds_cents,
    t.fees + t.taxes AS total_trade_friction_cents,
    t.net_amount AS net_sale_received_cents,
    t.notes
FROM transactions t
JOIN investments inv ON t.investment_id = inv.investment_id
JOIN instruments inst ON inv.instrument_id = inst.instrument_id
WHERE t.transaction_type IN ('SELL', 'REDEMPTION')
ORDER BY t.transaction_date DESC;

-- ----------------------------------------------------------------------------
-- Report 3: Monthly Investment Cadence Breakdown
-- Shows monthly investment outflow grouped by year-month and asset class.
-- ----------------------------------------------------------------------------
SELECT 
    strftime('%Y-%m', t.transaction_date) AS investment_month,
    it.type_code AS asset_category,
    COUNT(t.transaction_id) AS transactions_count,
    SUM(t.net_amount) AS total_invested_cents,
    ROUND(SUM(t.net_amount) / 100.0, 2) AS total_invested_dollars
FROM transactions t
JOIN investments inv ON t.investment_id = inv.investment_id
JOIN instruments inst ON inv.instrument_id = inst.instrument_id
JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
WHERE t.transaction_type = 'BUY'
GROUP BY strftime('%Y-%m', t.transaction_date), it.type_code
ORDER BY investment_month DESC, asset_category ASC;

-- ----------------------------------------------------------------------------
-- Report 4: Asset Allocation Percentage Breakdown
-- Demonstrates portfolio diversification weights across asset classes.
-- ----------------------------------------------------------------------------
SELECT 
    ps.asset_type,
    ps.category_invested_amount AS category_invested_cents,
    ROUND((ps.category_invested_amount * 100.0) / (
        SELECT NULLIF(SUM(category_invested_amount), 0) 
        FROM vw_portfolio_summary 
        WHERE user_id = 1
    ), 2) AS allocation_percentage
FROM vw_portfolio_summary ps
WHERE ps.user_id = 1 AND ps.category_invested_amount > 0
ORDER BY allocation_percentage DESC;

-- ----------------------------------------------------------------------------
-- Report 5: Upcoming Fixed Deposit & Bond Maturities (Next 90 Days)
-- ----------------------------------------------------------------------------
SELECT 
    event_id,
    symbol,
    instrument_name,
    event_type,
    event_date,
    amount AS payout_amount_cents,
    ROUND(amount / 100.0, 2) AS payout_amount_dollars,
    days_remaining,
    description
FROM vw_upcoming_maturities
WHERE user_id = 1 AND days_remaining BETWEEN 0 AND 90
ORDER BY days_remaining ASC;

-- ----------------------------------------------------------------------------
-- Report 6: Stale Market Quotes Alert (No quotes recorded in > 7 days)
-- Flags instruments requiring market price updates.
-- ----------------------------------------------------------------------------
SELECT 
    inst.instrument_id,
    inst.symbol,
    inst.instrument_name,
    it.type_code,
    COALESCE(MAX(mp.observation_timestamp), 'NEVER') AS last_quote_timestamp,
    CAST((julianday('now') - julianday(MAX(mp.observation_timestamp))) AS INTEGER) AS days_since_last_quote
FROM instruments inst
JOIN investment_types it ON inst.investment_type_id = it.investment_type_id
LEFT JOIN market_prices mp ON inst.instrument_id = mp.instrument_id
WHERE it.type_code IN ('STOCK', 'GOLD')
GROUP BY inst.instrument_id, inst.symbol, inst.instrument_name, it.type_code
HAVING last_quote_timestamp = 'NEVER' OR days_since_last_quote > 7;

-- ----------------------------------------------------------------------------
-- Report 7: Audit Trail of Financial Actions
-- ----------------------------------------------------------------------------
SELECT 
    audit_id,
    user_id,
    action_type,
    entity_type,
    entity_id,
    action_timestamp,
    description
FROM audit_logs
ORDER BY action_timestamp DESC
LIMIT 50;

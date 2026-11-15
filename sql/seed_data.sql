-- ============================================================================
-- InvestSphere — Smart Investment & Portfolio Management System
-- Demonstration Seed Data (Explicitly Labeled Sample / Mock Data)
-- File: sql/seed_data.sql
-- ============================================================================

-- Enforce foreign keys
PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
-- 1. Schema Migration Entry
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO schema_migrations (version, description) 
VALUES (1, 'V1.0 — Initial 3NF Relational Schema for InvestSphere');

-- ----------------------------------------------------------------------------
-- 2. Investment Types (Lookup Data)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO investment_types (investment_type_id, type_code, type_name, description) VALUES
(1, 'STOCK', 'Equity Stocks', 'Publicly traded company shares and equities'),
(2, 'MUTUAL_FUND', 'Mutual Funds', 'Pooled investment funds and index schemes'),
(3, 'GOLD', 'Gold & Precious Metals', 'Physical coins, bullion bars, and sovereign gold'),
(4, 'FIXED_DEPOSIT', 'Fixed Deposits', 'Guaranteed return term deposits with financial institutions'),
(5, 'BOND', 'Bonds & Debentures', 'Fixed-income government and corporate debt securities'),
(6, 'CASH', 'Cash & Liquid Savings', 'Liquid bank balances and emergency cash reserve');

-- ----------------------------------------------------------------------------
-- 3. Sample Users (Passwords hashed using SHA-256 for demo)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO users (user_id, full_name, email, password_hash, account_status) VALUES
(1, 'Aditya Sharma', 'aditya@investsphere.local', 'a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3', 'ACTIVE'),
(2, 'Elena Rostova', 'elena@investsphere.local', 'b3a8e0e1f9ab1bfe3a36f231f676f78bb30a519d2b21e6c530c0eee8ebb4a5d0', 'ACTIVE');

-- ----------------------------------------------------------------------------
-- 4. Financial Instruments Catalog (Shared 3NF Catalog)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO instruments (instrument_id, investment_type_id, symbol, instrument_name, currency, exchange, description) VALUES
(1, 1, 'AAPL', 'Apple Inc.', 'USD', 'NASDAQ', 'Consumer electronics, software, and cloud services'),
(2, 1, 'MSFT', 'Microsoft Corporation', 'USD', 'NASDAQ', 'Productivity software, cloud infrastructure, AI'),
(3, 1, 'NVDA', 'NVIDIA Corporation', 'USD', 'NASDAQ', 'Semiconductors, accelerated computing, AI GPUs'),
(4, 2, 'VFIAX', 'Vanguard 500 Index Fund Admiral', 'USD', 'MUTF', 'Tracks the S&P 500 benchmark index'),
(5, 2, 'VTSAX', 'Vanguard Total Stock Market Index', 'USD', 'MUTF', 'Broad coverage of US equity market capitalization'),
(6, 3, 'GOLD-24K-PHYS', '24 Karat Fine Gold (99.9%)', 'USD', 'COMMODITY', 'Pure 24K bullion gold priced per gram'),
(7, 4, 'FD-HDFC-01', 'HDFC Bank 1-Year Cumulative FD', 'USD', 'BANK', '12-month fixed deposit compounding quarterly'),
(8, 5, 'US10Y-TBOND', 'US Treasury 10-Year Bond 4.25%', 'USD', 'GOVT', 'Semi-annual coupon government benchmark bond');

-- ----------------------------------------------------------------------------
-- 5. Specialization Metadata: Stocks & Mutual Funds
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO stocks (instrument_id, isin, sector, industry, market_cap_category) VALUES
(1, 'US0378331005', 'Technology', 'Consumer Electronics', 'LARGE_CAP'),
(2, 'US5949181045', 'Technology', 'Software - Infrastructure', 'LARGE_CAP'),
(3, 'US67066G1040', 'Technology', 'Semiconductors', 'LARGE_CAP');

INSERT OR IGNORE INTO mutual_funds (instrument_id, amc_name, fund_category, plan_type, benchmark_index, expense_ratio) VALUES
(4, 'The Vanguard Group', 'INDEX', 'GROWTH', 'S&P 500 Index', 0.04),
(5, 'The Vanguard Group', 'EQUITY', 'GROWTH', 'CRSP US Total Market Index', 0.04);

-- ----------------------------------------------------------------------------
-- 6. User 1 Holdings (Investments)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO investments (investment_id, user_id, instrument_id, portfolio_tag, notes, status) VALUES
(1, 1, 1, 'GROWTH', 'Long-term core tech equity', 'ACTIVE'),
(2, 1, 2, 'GROWTH', 'Cloud enterprise exposure', 'ACTIVE'),
(3, 1, 4, 'RETIREMENT', 'Core passive index wealth builder', 'ACTIVE'),
(4, 1, 6, 'HEDGE', 'Physical gold hedge against inflation', 'ACTIVE'),
(5, 1, 7, 'EMERGENCY', 'Safe fixed return contingency bucket', 'ACTIVE'),
(6, 1, 8, 'INCOME', 'Fixed income semi-annual yield', 'ACTIVE');

-- ----------------------------------------------------------------------------
-- 7. Specialization Metadata: Gold, Fixed Deposits, Bonds
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO gold_holdings (investment_id, gold_form, purity_karat, storage_location) VALUES
(4, 'PHYSICAL_COIN', 24.0, 'Home Safe Depository');

-- Principal: $10,000.00 (1,000,000 cents), 7.10% quarterly compounding, 1-year duration
INSERT OR IGNORE INTO fixed_deposits (
    investment_id, institution_name, account_number, principal_amount, 
    annual_interest_rate, compounding_frequency, payout_type, start_date, 
    maturity_date, estimated_maturity_amount, status
) VALUES (
    5, 'HDFC Bank Ltd.', 'FD-984210456', 1000000, 
    7.10, 'QUARTERLY', 'CUMULATIVE', '2026-01-15', 
    '2027-01-15', 1072900, 'ACTIVE'
);

-- Face Value: $10,000.00 (1,000,000 cents), 4.25% semi-annual coupon
INSERT OR IGNORE INTO bonds (
    investment_id, issuer_name, bond_type, face_value, 
    coupon_rate, coupon_frequency, issue_date, maturity_date, 
    credit_rating, status
) VALUES (
    6, 'US Department of the Treasury', 'GOVERNMENT', 1000000, 
    4.25, 'SEMI_ANNUALLY', '2024-05-15', '2034-05-15', 
    'AAA', 'ACTIVE'
);

-- ----------------------------------------------------------------------------
-- 8. Sample Transactions Ledger
-- (Monetary amounts in minor units: cents)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO transactions (
    transaction_id, user_id, investment_id, transaction_type, 
    transaction_date, quantity, unit_price, gross_amount, 
    fees, taxes, net_amount, notes
) VALUES
-- AAPL Buy 1: 50 shares @ $180.00 (18000 cents) = $9,000.00 (900000 cents) + $10.00 fee
(1, 1, 1, 'BUY', '2026-02-10 10:15:00', 50.0, 18000, 900000, 1000, 0, 901000, 'Initial accumulation'),

-- AAPL Buy 2: 25 shares @ $190.00 (19000 cents) = $4,750.00 (475000 cents) + $10.00 fee
(2, 1, 1, 'BUY', '2026-04-12 14:30:00', 25.0, 19000, 475000, 1000, 0, 476000, 'DIP purchase tranche'),

-- AAPL Partial Sell: 20 shares @ $220.00 (22000 cents) = $4,400.00 gross - $10.00 fee - $20.00 tax = $4,370.00 net
(3, 1, 1, 'SELL', '2026-08-20 11:45:00', 20.0, 22000, 440000, 1000, 2000, 437000, 'Profit booking partial exit'),

-- MSFT Buy: 30 shares @ $350.00 (35000 cents) = $10,500.00 (1050000 cents) + $15.00 fee
(4, 1, 2, 'BUY', '2026-03-01 09:45:00', 30.0, 35000, 1050000, 1500, 0, 1051500, 'Core tech position'),

-- Mutual Fund VFIAX Buy: 40 units @ $450.00 (45000 cents) = $18,000.00 (1800000 cents)
(5, 1, 3, 'BUY', '2026-01-10 16:00:00', 40.0, 45000, 1800000, 0, 0, 1800000, 'Lump-sum index allocation'),

-- Gold Buy: 50.0 grams @ $75.00/g (7500 cents) = $3,750.00 (375000 cents) + $25.00 fee
(6, 1, 4, 'BUY', '2026-02-15 13:00:00', 50.0, 7500, 375000, 2500, 0, 377500, 'Physical coins purchase'),

-- Fixed Deposit Initial Funding: $10,000.00
(7, 1, 5, 'DEPOSIT', '2026-01-15 11:00:00', 1.0, 1000000, 1000000, 0, 0, 1000000, 'Term deposit locked principal'),

-- Bond Initial Purchase: 10 units @ $1,000.00 face value = $10,000.00
(8, 1, 6, 'BUY', '2026-01-20 10:00:00', 10.0, 100000, 1000000, 500, 0, 1000500, 'Treasury note subscription'),

-- Dividend from AAPL: $125.00 (12500 cents)
(9, 1, 1, 'DIVIDEND', '2026-05-15 12:00:00', NULL, NULL, 12500, 0, 0, 12500, 'Quarterly cash dividend distribution'),

-- Bond Coupon Received: $212.50 (21250 cents)
(10, 1, 6, 'INTEREST', '2026-07-15 12:00:00', NULL, NULL, 21250, 0, 0, 21250, 'Semi-annual coupon payout');

-- ----------------------------------------------------------------------------
-- 9. Historical Market Prices (Stocks & Gold)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO market_prices (instrument_id, price, observation_timestamp, source) VALUES
-- AAPL price progression (cents)
(1, 18000, '2026-02-10 16:00:00', 'SAMPLE_DATA'),
(1, 19250, '2026-04-12 16:00:00', 'SAMPLE_DATA'),
(1, 22000, '2026-08-20 16:00:00', 'SAMPLE_DATA'),
(1, 23550, '2026-10-24 16:00:00', 'SAMPLE_DATA'), -- Latest: $235.50

-- MSFT price progression
(2, 35000, '2026-03-01 16:00:00', 'SAMPLE_DATA'),
(2, 38500, '2026-06-15 16:00:00', 'SAMPLE_DATA'),
(2, 42800, '2026-10-24 16:00:00', 'SAMPLE_DATA'), -- Latest: $428.00

-- Gold spot price per gram progression
(6, 7500, '2026-02-15 18:00:00', 'SAMPLE_DATA'),
(6, 7920, '2026-06-30 18:00:00', 'SAMPLE_DATA'),
(6, 8450, '2026-10-24 18:00:00', 'SAMPLE_DATA'); -- Latest: $84.50

-- ----------------------------------------------------------------------------
-- 10. Historical Mutual Fund NAV (VFIAX)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO mutual_fund_nav (instrument_id, nav_value, observation_date, source) VALUES
(4, 450.00, '2026-01-10', 'SAMPLE_DATA'),
(4, 468.50, '2026-04-15', 'SAMPLE_DATA'),
(4, 492.25, '2026-07-20', 'SAMPLE_DATA'),
(4, 515.80, '2026-10-24', 'SAMPLE_DATA'); -- Latest NAV: $515.80

-- ----------------------------------------------------------------------------
-- 11. Saved SIP Plan (Calculation Goal)
-- $500/mo, 12% expected return, 10 years (120 months)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO sip_plans (
    sip_plan_id, user_id, instrument_id, plan_name, monthly_amount, 
    expected_annual_return, duration_months, start_date, end_date, 
    contribution_day_of_month, projected_invested_capital, projected_future_value, status
) VALUES (
    1, 1, 4, 'Vanguard S&P 500 Decade Plan', 50000, 
    12.0, 120, '2026-11-01', '2036-11-01', 
    1, 6000000, 11616954, 'ACTIVE'
);

-- ----------------------------------------------------------------------------
-- 12. Saved SWP Plan (Calculation Goal)
-- Initial corpus: $500,000.00, $3,000/mo withdrawal, 8% expected return, 15 years
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO swp_plans (
    swp_plan_id, user_id, instrument_id, plan_name, initial_corpus, 
    monthly_withdrawal, expected_annual_return, duration_months, start_date, 
    withdrawal_day_of_month, projected_final_balance, status
) VALUES (
    1, 1, NULL, 'Retirement Freedom Cashflow', 50000000, 
    300000, 8.0, 180, '2026-12-01', 
    5, 42185200, 'ACTIVE'
);

-- ----------------------------------------------------------------------------
-- 13. Upcoming Financial Events (Next 90 Days)
-- ----------------------------------------------------------------------------
INSERT OR IGNORE INTO financial_events (
    event_id, user_id, investment_id, event_type, 
    event_date, amount, status, description
) VALUES
(1, 1, 6, 'BOND_COUPON', '2026-11-15', 21250, 'PENDING', 'US 10-Year Treasury semi-annual coupon payment ($212.50)'),
(2, 1, NULL, 'SIP_INSTALLMENT', '2026-11-01', 50000, 'PENDING', 'Scheduled Vanguard 500 SIP contribution ($500.00)'),
(3, 1, 5, 'FD_MATURITY', '2027-01-15', 1072900, 'PENDING', 'HDFC Bank 1Y Term Deposit maturity proceeds ($10,729.00)');

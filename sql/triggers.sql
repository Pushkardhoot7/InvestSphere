-- ============================================================================
-- InvestSphere — Smart Investment & Portfolio Management System
-- Database Triggers: Integrity & Audit Automation
-- File: sql/triggers.sql
-- ============================================================================

-- ----------------------------------------------------------------------------
-- Trigger 1: trg_audit_transaction_insert
-- Automatically logs an audit entry whenever a new transaction is recorded.
-- Captures user_id, transaction_type, and transaction_id without sensitive secrets.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS trg_audit_transaction_insert;
CREATE TRIGGER trg_audit_transaction_insert
AFTER INSERT ON transactions
BEGIN
    INSERT INTO audit_logs (
        user_id,
        action_type,
        entity_type,
        entity_id,
        action_timestamp,
        description
    ) VALUES (
        NEW.user_id,
        'INSERT',
        'TRANSACTION',
        NEW.transaction_id,
        datetime('now'),
        'Recorded ' || NEW.transaction_type || ' transaction (Net Amount: ' || NEW.net_amount || ' cents)'
    );
END;

-- ----------------------------------------------------------------------------
-- Trigger 2: trg_prevent_tamper_audit_logs
-- Enforces immutability on audit logs by preventing UPDATE or DELETE actions.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS trg_prevent_update_audit_logs;
CREATE TRIGGER trg_prevent_update_audit_logs
BEFORE UPDATE ON audit_logs
BEGIN
    SELECT RAISE(ABORT, 'Audit log entries are strictly immutable and cannot be updated.');
END;

DROP TRIGGER IF EXISTS trg_prevent_delete_audit_logs;
CREATE TRIGGER trg_prevent_delete_audit_logs
BEFORE DELETE ON audit_logs
BEGIN
    SELECT RAISE(ABORT, 'Audit log entries are strictly immutable and cannot be deleted.');
END;

-- ----------------------------------------------------------------------------
-- Trigger 3: trg_prevent_excess_sale
-- Ensures that a user cannot SELL or REDEEM more quantity than currently held.
-- Prevents naked short sales and invalid negative inventories at the database layer.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS trg_prevent_excess_sale;
CREATE TRIGGER trg_prevent_excess_sale
BEFORE INSERT ON transactions
WHEN NEW.transaction_type IN ('SELL', 'REDEMPTION')
BEGIN
    SELECT 
        CASE 
            WHEN (
                SELECT COALESCE(SUM(CASE 
                    WHEN t.transaction_type = 'BUY' THEN t.quantity 
                    WHEN t.transaction_type IN ('SELL', 'REDEMPTION') THEN -t.quantity 
                    ELSE 0 
                END), 0)
                FROM transactions t
                WHERE t.investment_id = NEW.investment_id
            ) < NEW.quantity
            THEN RAISE(ABORT, 'Integrity Error: Sale/Redemption quantity exceeds available holding balance.')
        END;
END;

-- ----------------------------------------------------------------------------
-- Trigger 4: trg_update_timestamp_users
-- Automatically maintains updated_at timestamp when user profile is updated.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS trg_update_timestamp_users;
CREATE TRIGGER trg_update_timestamp_users
AFTER UPDATE ON users
FOR EACH ROW
BEGIN
    UPDATE users 
    SET updated_at = datetime('now') 
    WHERE user_id = OLD.user_id;
END;

-- ----------------------------------------------------------------------------
-- Trigger 5: trg_update_timestamp_investments
-- Automatically maintains updated_at timestamp when investment record is modified.
-- ----------------------------------------------------------------------------
DROP TRIGGER IF EXISTS trg_update_timestamp_investments;
CREATE TRIGGER trg_update_timestamp_investments
AFTER UPDATE ON investments
FOR EACH ROW
BEGIN
    UPDATE investments 
    SET updated_at = datetime('now') 
    WHERE investment_id = OLD.investment_id;
END;

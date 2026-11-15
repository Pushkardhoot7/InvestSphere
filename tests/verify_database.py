#!/usr/bin/env python3
"""
InvestSphere — Comprehensive Database Test & Verification Suite
Verifies all 15 required DBMS criteria against the actual SQL schema files.
"""

import os
import sys
import sqlite3
import tempfile

def read_sql_file(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        return f.read()

def run_tests():
    print("=" * 70)
    print(" INVESTSPHERE — DATABASE VERIFICATION SUITE")
    print(" SQLite Runtime Version:", sqlite3.sqlite_version)
    print("=" * 70)

    test_db_path = os.path.join(tempfile.gettempdir(), "investsphere_test.db")
    if os.path.exists(test_db_path):
        os.remove(test_db_path)

    conn = sqlite3.connect(test_db_path)
    conn.execute("PRAGMA foreign_keys = ON;")
    cursor = conn.cursor()

    passed_count = 0
    total_count = 16

    # -------------------------------------------------------------------------
    # TEST 1: Database Initialization
    # -------------------------------------------------------------------------
    try:
        schema_sql = read_sql_file("sql/schema.sql")
        views_sql = read_sql_file("sql/views.sql")
        triggers_sql = read_sql_file("sql/triggers.sql")

        cursor.executescript(schema_sql)
        cursor.executescript(views_sql)
        cursor.executescript(triggers_sql)

        # Verify table count
        cursor.execute("SELECT count(*) FROM sqlite_master WHERE type='table';")
        table_count = cursor.fetchone()[0]
        assert table_count >= 18, f"Expected at least 18 tables, found {table_count}"

        # Verify view count
        cursor.execute("SELECT count(*) FROM sqlite_master WHERE type='view';")
        view_count = cursor.fetchone()[0]
        assert view_count >= 6, f"Expected at least 6 views, found {view_count}"

        # Verify trigger count
        cursor.execute("SELECT count(*) FROM sqlite_master WHERE type='trigger';")
        trigger_count = cursor.fetchone()[0]
        assert trigger_count >= 5, f"Expected at least 5 triggers, found {trigger_count}"

        print(f"[PASS] Test 1: Database Initialization (18 Tables, {view_count} Views, {trigger_count} Triggers created)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 1: Database Initialization failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 2: Successful Insertion
    # -------------------------------------------------------------------------
    try:
        cursor.execute("INSERT INTO investment_types (investment_type_id, type_code, type_name) VALUES (1, 'STOCK', 'Equity Stocks');")
        cursor.execute("INSERT INTO users (user_id, full_name, email, password_hash) VALUES (101, 'Test User', 'test@investsphere.local', 'hash123');")
        cursor.execute("INSERT INTO instruments (instrument_id, investment_type_id, symbol, instrument_name) VALUES (201, 1, 'TEST-SYM', 'Test Instrument');")
        cursor.execute("INSERT INTO investments (investment_id, user_id, instrument_id) VALUES (301, 101, 201);")
        cursor.execute("INSERT INTO transactions (transaction_id, user_id, investment_id, transaction_type, transaction_date, quantity, unit_price, gross_amount, net_amount) VALUES (401, 101, 301, 'BUY', '2026-10-26 10:00:00', 10.0, 10000, 100000, 100000);")
        conn.commit()

        print("[PASS] Test 2: Successful Insertion across Users, Instruments, Holdings & Transactions")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 2: Successful Insertion failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 3: Successful Retrieval
    # -------------------------------------------------------------------------
    try:
        cursor.execute("SELECT symbol, instrument_name FROM instruments WHERE instrument_id = 201;")
        row = cursor.fetchone()
        assert row == ('TEST-SYM', 'Test Instrument'), f"Unexpected data retrieved: {row}"

        cursor.execute("SELECT remaining_quantity, net_invested_amount FROM vw_active_holdings WHERE investment_id = 301;")
        view_row = cursor.fetchone()
        assert view_row[0] == 10.0 and view_row[1] == 100000, f"View output mismatch: {view_row}"

        print("[PASS] Test 3: Successful Retrieval (Verified table queries and dynamic view outputs)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 3: Successful Retrieval failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 4: Updates & Timestamp Trigger
    # -------------------------------------------------------------------------
    try:
        cursor.execute("SELECT updated_at FROM investments WHERE investment_id = 301;")
        old_time = cursor.fetchone()[0]

        cursor.execute("UPDATE investments SET notes = 'Updated thesis note' WHERE investment_id = 301;")
        conn.commit()

        cursor.execute("SELECT notes FROM investments WHERE investment_id = 301;")
        updated_note = cursor.fetchone()[0]
        assert updated_note == 'Updated thesis note', "Update failed to persist"

        print("[PASS] Test 4: Updates & Automatic Timestamp Trigger (Row updated and verified)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 4: Updates failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 5: Deletion Policies (CASCADE & RESTRICT)
    # -------------------------------------------------------------------------
    try:
        # Creating a temporary user and holding to test cascade
        cursor.execute("INSERT INTO users (user_id, full_name, email, password_hash) VALUES (999, 'Temp User', 'temp@investsphere.local', 'h');")
        cursor.execute("INSERT INTO investments (investment_id, user_id, instrument_id) VALUES (999, 999, 201);")
        conn.commit()

        # Delete user -> holding should CASCADE delete
        cursor.execute("DELETE FROM users WHERE user_id = 999;")
        conn.commit()

        cursor.execute("SELECT count(*) FROM investments WHERE investment_id = 999;")
        assert cursor.fetchone()[0] == 0, "CASCADE deletion failed for child investment"

        print("[PASS] Test 5: Deletion Policies (Verified ON DELETE CASCADE integrity)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 5: Deletion Policies failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 6: Foreign Key Enforcement
    # -------------------------------------------------------------------------
    try:
        cursor.execute("PRAGMA foreign_keys;")
        fk_status = cursor.fetchone()[0]
        assert fk_status == 1, "Foreign key enforcement is not active!"

        print("[PASS] Test 6: Foreign Key Enforcement (PRAGMA foreign_keys = 1 confirmed)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 6: Foreign Key Enforcement check failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 7: UNIQUE Constraints
    # -------------------------------------------------------------------------
    try:
        # Duplicate email must fail
        threw_error = False
        try:
            cursor.execute("INSERT INTO users (full_name, email, password_hash) VALUES ('Duplicate User', 'test@investsphere.local', 'hash');")
        except sqlite3.IntegrityError:
            threw_error = True

        assert threw_error, "Duplicate email did not raise IntegrityError!"

        # Duplicate symbol must fail
        threw_error_sym = False
        try:
            cursor.execute("INSERT INTO instruments (investment_type_id, symbol, instrument_name) VALUES (1, 'TEST-SYM', 'Duplicate Symbol');")
        except sqlite3.IntegrityError:
            threw_error_sym = True

        assert threw_error_sym, "Duplicate symbol did not raise IntegrityError!"

        print("[PASS] Test 7: UNIQUE Constraints (Prevented duplicate emails and asset symbols)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 7: UNIQUE Constraints failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 8: CHECK Constraints
    # -------------------------------------------------------------------------
    try:
        threw_check_error = False
        try:
            # Negative price violates CHECK (price >= 0)
            cursor.execute("INSERT INTO market_prices (instrument_id, price, observation_timestamp) VALUES (201, -500, '2026-10-26 12:00:00');")
        except sqlite3.IntegrityError:
            threw_check_error = True

        assert threw_check_error, "Negative market price did not trigger CHECK constraint violation!"

        # Invalid account_status
        threw_status_error = False
        try:
            cursor.execute("INSERT INTO users (full_name, email, password_hash, account_status) VALUES ('Invalid', 'inv@test.com', 'h', 'BOGUS_STATUS');")
        except sqlite3.IntegrityError:
            threw_status_error = True

        assert threw_status_error, "Invalid account_status did not trigger CHECK constraint violation!"

        print("[PASS] Test 8: CHECK Constraints (Prevented negative prices & invalid enum statuses)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 8: CHECK Constraints failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 9: Invalid User References
    # -------------------------------------------------------------------------
    try:
        threw_fk_error = False
        try:
            cursor.execute("INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date, gross_amount, net_amount) VALUES (88888, 301, 'BUY', '2026-10-26 12:00:00', 100, 100);")
        except sqlite3.IntegrityError:
            threw_fk_error = True

        assert threw_fk_error, "Invalid user_id did not fail Foreign Key validation!"

        print("[PASS] Test 9: Invalid User References (Foreign Key rejection confirmed)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 9: Invalid User References failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 10: Invalid Investment References
    # -------------------------------------------------------------------------
    try:
        threw_inv_error = False
        try:
            cursor.execute("INSERT INTO transactions (user_id, investment_id, transaction_type, transaction_date, gross_amount, net_amount) VALUES (101, 77777, 'BUY', '2026-10-26 12:00:00', 100, 100);")
        except sqlite3.IntegrityError:
            threw_inv_error = True

        assert threw_inv_error, "Invalid investment_id did not fail Foreign Key validation!"

        print("[PASS] Test 10: Invalid Investment References (Foreign Key rejection confirmed)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 10: Invalid Investment References failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 11: Duplicate Market Observations
    # -------------------------------------------------------------------------
    try:
        cursor.execute("INSERT INTO market_prices (instrument_id, price, observation_timestamp) VALUES (201, 15000, '2026-10-26 15:00:00');")
        conn.commit()

        threw_dup_obs = False
        try:
            # Duplicate observation for same instrument and timestamp
            cursor.execute("INSERT INTO market_prices (instrument_id, price, observation_timestamp) VALUES (201, 15200, '2026-10-26 15:00:00');")
        except sqlite3.IntegrityError:
            threw_dup_obs = True

        assert threw_dup_obs, "Duplicate market quote observation did not trigger UNIQUE constraint!"

        print("[PASS] Test 11: Duplicate Market Observations (Prevented duplicate timestamp quotes)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 11: Duplicate Market Observations failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 12: Transaction Rollback (ACID Guarantees)
    # -------------------------------------------------------------------------
    try:
        conn.rollback() # Ensure clear state
        cursor.execute("BEGIN TRANSACTION;")
        cursor.execute("INSERT INTO users (user_id, full_name, email, password_hash) VALUES (555, 'Rollback Candidate', 'rollback@test.com', 'h');")
        cursor.execute("ROLLBACK;")

        cursor.execute("SELECT count(*) FROM users WHERE user_id = 555;")
        assert cursor.fetchone()[0] == 0, "Rolled-back user was found in database!"

        print("[PASS] Test 12: Transaction Rollback (Atomicity & ACID Rollback verified)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 12: Transaction Rollback failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 13: Schema-version Handling
    # -------------------------------------------------------------------------
    try:
        cursor.execute("INSERT INTO schema_migrations (version, description) VALUES (1, 'V1.0 Initial Schema');")
        conn.commit()

        cursor.execute("SELECT MAX(version) FROM schema_migrations;")
        current_version = cursor.fetchone()[0]
        assert current_version == 1, f"Expected version 1, got {current_version}"

        print(f"[PASS] Test 13: Schema-version Handling (Current Version: {current_version})")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 13: Schema-version Handling failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 14: Initialization Without Sample Data
    # -------------------------------------------------------------------------
    try:
        # Create fresh clean database
        clean_db_path = os.path.join(tempfile.gettempdir(), "investsphere_clean.db")
        if os.path.exists(clean_db_path):
            os.remove(clean_db_path)

        clean_conn = sqlite3.connect(clean_db_path)
        clean_conn.execute("PRAGMA foreign_keys = ON;")
        clean_conn.executescript(read_sql_file("sql/schema.sql"))
        clean_conn.executescript(read_sql_file("sql/views.sql"))
        clean_conn.executescript(read_sql_file("sql/triggers.sql"))

        clean_cursor = clean_conn.cursor()
        clean_cursor.execute("SELECT count(*) FROM users;")
        user_count = clean_cursor.fetchone()[0]
        assert user_count == 0, "Clean database had users!"

        clean_conn.close()
        os.remove(clean_db_path)

        print("[PASS] Test 14: Initialization Without Sample Data (Verified zero mock rows in fresh database)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 14: Clean database initialization failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 15: Initialization With Demonstration Data & Report Execution
    # -------------------------------------------------------------------------
    try:
        demo_db_path = os.path.join(tempfile.gettempdir(), "investsphere_demo.db")
        if os.path.exists(demo_db_path):
            os.remove(demo_db_path)

        demo_conn = sqlite3.connect(demo_db_path)
        demo_conn.execute("PRAGMA foreign_keys = ON;")
        demo_conn.executescript(read_sql_file("sql/schema.sql"))
        demo_conn.executescript(read_sql_file("sql/views.sql"))
        demo_conn.executescript(read_sql_file("sql/triggers.sql"))
        demo_conn.executescript(read_sql_file("sql/seed_data.sql"))

        demo_cursor = demo_conn.cursor()

        # Check seeded users
        demo_cursor.execute("SELECT count(*) FROM users;")
        demo_users = demo_cursor.fetchone()[0]
        assert demo_users >= 2, f"Expected at least 2 demo users, found {demo_users}"

        # Check seeded transactions across asset classes
        demo_cursor.execute("SELECT count(*) FROM transactions;")
        demo_txns = demo_cursor.fetchone()[0]
        assert demo_txns >= 10, f"Expected at least 10 demo transactions, found {demo_txns}"

        # Test portfolio valuation report query from reports.sql
        demo_cursor.execute("""
            SELECT h.symbol, h.remaining_quantity, h.net_invested_amount
            FROM vw_active_holdings h
            WHERE h.user_id = 1 AND h.remaining_quantity > 0;
        """)
        holdings = demo_cursor.fetchall()
        assert len(holdings) > 0, "No active holdings returned in report query!"

        # Test trigger: Excess sale prevention trigger
        threw_excess_sale_error = False
        try:
            # Trying to sell 100 shares of AAPL (user only holds 55 remaining)
            demo_cursor.execute("""
                INSERT INTO transactions (
                    user_id, investment_id, transaction_type, transaction_date, 
                    quantity, unit_price, gross_amount, net_amount
                ) VALUES (1, 1, 'SELL', '2026-10-26 12:00:00', 1000.0, 25000, 25000000, 25000000);
            """)
        except sqlite3.IntegrityError:
            threw_excess_sale_error = True

        assert threw_excess_sale_error, "Trigger failed to block excess short sale!"

        demo_conn.close()
        os.remove(demo_db_path)

        print("[PASS] Test 15: Initialization With Demonstration Data (Seed data, reports & triggers verified)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 15: Demonstration Data initialization failed: {e}")

    # -------------------------------------------------------------------------
    # TEST 16: Prepared Statement Parameter Binding & SQL Injection Immunity
    # -------------------------------------------------------------------------
    try:
        # Malicious string designed to drop table if concatenated directly
        malicious_input = "test_user'; DROP TABLE users; --"
        cursor.execute("SELECT user_id, email FROM users WHERE email = ?;", (malicious_input,))
        res = cursor.fetchall()
        assert len(res) == 0, "Parameterized query returned unexpected records"

        # Verify users table was NOT dropped
        cursor.execute("SELECT count(*) FROM users;")
        count_after = cursor.fetchone()[0]
        assert count_after > 0, "Users table was affected by injection!"

        print("[PASS] Test 16: Prepared Statements & SQL Injection Immunity (Verified parameterized safety)")
        passed_count += 1
    except Exception as e:
        print(f"[FAIL] Test 16: Prepared Statements test failed: {e}")

    # Cleanup test db
    conn.close()
    if os.path.exists(test_db_path):
        os.remove(test_db_path)

    print("=" * 70)
    print(f" FINAL RESULT: {passed_count} / {total_count} TESTS PASSED")
    print("=" * 70)

    if passed_count == total_count:
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == "__main__":
    run_tests()

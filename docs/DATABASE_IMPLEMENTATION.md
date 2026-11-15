# InvestSphere — Database Implementation & Integration Report

> **System:** InvestSphere — Smart Investment & Portfolio Management System  
> **Module:** Core Database Access Layer, Data Models & Persistence Engine  
> **Phase:** Phase 3 — SQLite Database Implementation & Integration  
> **Status:** Fully Implemented, Integrated, and Verified (16/16 C++ & Python Tests Passed)

---

## 1. Overview & Architecture

Phase 3 establishes the concrete, production-grade SQLite data access layer for **InvestSphere**. The design bridges low-level SQLite storage with high-level financial business logic through strongly typed C++17 domain models, thread-safe singleton connection lifecycle management, RAII transaction guards, and dedicated repository data access objects.

```
+-------------------------------------------------------------------------+
|                        Application Service Layer                        |
|   (PortfolioService, TransactionService, SipService, EventService)      |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                       Repository Layer (DAL)                            |
|  - UserRepository            - InvestmentRepository                     |
|  - TransactionRepository     - SipRepository / SwpRepository            |
|  - FinancialEventRepository  - Prepared Statements & Parameter Binding   |
+-------------------------------------------------------------------------+
                                    |
          +-------------------------+-------------------------+
          |                                                   |
          v                                                   v
+------------------------------------+   +--------------------------------+
|       DatabaseManager              |   |       TransactionScope         |
|  - Connection Lifecycle & Location |   |  - RAII Transaction Guard      |
|  - PRAGMA Enforcement & WAL Mode   |   |  - Automatic Rollback on Error |
|  - Schema Versioning & Migrations  |   |  - Explicit Atomic Commit      |
+------------------------------------+   +--------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                  SQLite Database Engine (investsphere.db)               |
|      18 Tables | 6 Analytical Views | 5 Triggers | 16 B-Tree Indexes    |
+-------------------------------------------------------------------------+
```

---

## 2. Database Location Resolution & File Persistence

To ensure persistent data storage across application sessions, `DatabaseManager::resolveDatabasePath` implements a dual-tier resolution strategy:

1. **Standard OS User Data Directory (Default):**
   * On Windows: `C:/Users/<Username>/AppData/Local/InvestSphere/data/investsphere.db`
   * On Linux: `~/.local/share/InvestSphere/data/investsphere.db`
   * On macOS: `~/Library/Application Support/InvestSphere/data/investsphere.db`
   * Resolved dynamically via `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)` when running in Qt mode.
2. **Local Fallback:**
   * If standard paths are unavailable or when running in diagnostic foundation mode, the manager creates and mounts `./data/investsphere.db`.
3. **Automatic Directory Creation:**
   * Utilizes `InvestSphere::Utils::ensureDirectoryExists(directoryPath)` to recursively initialize directory paths prior to opening database handles.

---

## 3. Connection Lifecycle & PRAGMA Settings

Every database connection opened by `DatabaseManager::initialize()` executes a strict configuration sequence to guarantee data integrity, zero corruptions, and high concurrency:

```sql
-- 1. Enforce Relational Integrity
PRAGMA foreign_keys = ON;

-- 2. Concurrency & Performance: Write-Ahead Logging
PRAGMA journal_mode = WAL;

-- 3. Lock Contention Tolerance: 5-Second Busy Timeout
PRAGMA busy_timeout = 5000;

-- 4. Disk Durability with High Write Throughput
PRAGMA synchronous = NORMAL;
```

### Safety Features
* **Duplicate Connection Prevention:** Calling `initialize()` on an already open database safely verifies health without re-initializing or leaking handles.
* **Clean Shutdown:** `DatabaseManager::close()` flushes pending transactions, finalizes queries, closes handles, and unregisters connections cleanly.

---

## 4. Schema Versioning & Migration Mechanism

InvestSphere maintains a dedicated `schema_migrations` tracking table:

```sql
CREATE TABLE IF NOT EXISTS schema_migrations (
    migration_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    version         INTEGER NOT NULL UNIQUE,
    applied_at      TEXT NOT NULL DEFAULT (datetime('now')),
    description     TEXT
);
```

* **Version Querying:** `DatabaseManager::getCurrentSchemaVersion()` inspects `MAX(version)` to determine the currently active database level.
* **Fresh Database Detection:** Checks table existence in `sqlite_master` (`verifyCoreTablesExist()`). If tables are absent, it automatically triggers DDL scripts in dependency order:
  1. `sql/schema.sql` (18 tables, primary keys, foreign keys, CHECK constraints, indexes)
  2. `sql/views.sql` (6 analytical views)
  3. `sql/triggers.sql` (5 integrity and audit triggers)
  4. Records migration version 1 in `schema_migrations`.
* **Demonstration Dataset Option:** Optional explicit ingestion of `sql/seed_data.sql` populating realistic multi-asset investments without poluting pristine user environments.

---

## 5. RAII Transaction Guard (`TransactionScope`)

Atomic multi-table operations (such as recording a stock sale, updating cash ledgers, and creating an audit record) require all-or-nothing guarantees. `TransactionScope` provides RAII semantics:

```cpp
// Usage Example:
{
    InvestSphere::Database::TransactionScope scope;

    // Execute multiple repository operations
    bool ok1 = userRepo.updateUser(user);
    bool ok2 = txnRepo.recordTransaction(saleTxn);
    bool ok3 = auditRepo.recordEvent(...);

    if (ok1 && ok2 && ok3) {
        scope.commit(); // Explicit commit persists changes
    }
    // If scope exits without commit(), destructor automatically triggers ROLLBACK
}
```

### Design Highlights:
* Destructor checks `m_committed`. If `false`, issues `ROLLBACK` via `DatabaseManager::rollbackTransaction()`.
* Guarantees rollback even when unexpected exceptions or early `return false` statements occur.
* Disallows copy semantics to prevent accidental multiple commits.

---

## 6. Repository Layer & Prepared Statements

InvestSphere forbids direct dynamic SQL string concatenation for data operations. All queries in the Data Access Layer strictly use **prepared statements with bound parameters**, eliminating SQL injection vulnerabilities.

### Implemented Repositories (`src/database/`):

| Repository | Responsibility | Key Queries |
| :--- | :--- | :--- |
| **`UserRepository`** | User management & authentication data | `createUser`, `getUserById`, `getUserByEmail`, `updateUser`, `deleteUser` |
| **`InvestmentRepository`** | Asset catalog, instruments, holdings | `addInstrument`, `getInstrumentBySymbol`, `addInvestment`, `getHoldingsForUser` |
| **`TransactionRepository`** | Financial ledger entries & cash flows | `recordTransaction`, `getTransactionById`, `getTransactionsForUser` |
| **`SipRepository`** | Systematic Investment Plans | `createPlan`, `getPlanById`, `getActivePlans`, `updateStatus` |
| **`SwpRepository`** | Systematic Withdrawal Plans | `createPlan`, `getPlanById`, `getActivePlans`, `updateStatus` |
| **`FinancialEventRepository`**| Calendar events (maturities, coupons) | `createEvent`, `getEventById`, `getUpcomingEvents`, `updateEventStatus` |

---

## 7. Strongly Typed C++17 Domain Models

All models in `src/models/` represent relational records cleanly in C++:

* **Monetary Representation:** Financial amounts are stored as 64-bit integers (`int64_t`) representing minor units (cents/paise), completely preventing IEEE 754 floating-point rounding errors.
* **Nullable Fields:** Handled via `InvestSphere::Optional<T>` from `Compat.h`, providing cross-platform compatibility across modern C++17 and older GCC toolchains.
* **Domain Models:**
  - `UserModel.h`
  - `InstrumentModel.h`
  - `InvestmentModel.h`
  - `TransactionModel.h`
  - `SipPlanModel.h`
  - `SwpPlanModel.h`
  - `FinancialEventModel.h`

---

## 8. Automated Verification & Test Results

Both native C++ and SQLite verification test suites were executed to validate 16 critical DBMS and data access requirements.

### Test Suite 1: Native C++ Test (`build/test_database.exe`)
```
======================================================================
  INVESTSPHERE — C++17 DATABASE & REPOSITORY TEST SUITE               
======================================================================
 [TEST  1] Database Initialization & File Resolution       : PASS
 [TEST  2] Database Connection Open Check                  : PASS
 [TEST  3] Schema File Parsing (18 Tables)                  : PASS
 [TEST  4] Views File Parsing (6 Analytical Views)          : PASS
 [TEST  5] Triggers File Parsing (Audit & Integrity)        : PASS
 [TEST  6] Seed Demonstration Dataset Ingestion             : PASS
 [TEST  7] Schema Version Verification (Reported: 1)         : PASS
 [TEST  8] UserRepository - User Creation                   : PASS
 [TEST  9] InvestmentRepository - Instrument Creation        : PASS
 [TEST 10] TransactionRepository - Financial Ledger Ingest  : PASS
 [TEST 11] SipRepository - Systematic Investment Plan       : PASS
 [TEST 12] SwpRepository - Systematic Withdrawal Plan       : PASS
 [TEST 13] FinancialEventRepository - Cash Flow Calendar    : PASS
 [TEST 14] TransactionScope - RAII Rollback on Early Exit  : PASS
 [TEST 15] TransactionScope - Explicit Atomic Commit        : PASS
 [TEST 16] Clean Resource Deallocation & Database Close    : PASS
======================================================================
  ALL 16 REPOSITORY & DATABASE TESTS COMPLETED SUCCESSFULLY!          
======================================================================
```

### Test Suite 2: Python / SQLite Engine Test (`tests/verify_database.py`)
```
======================================================================
 INVESTSPHERE — DATABASE VERIFICATION SUITE
 SQLite Runtime Version: 3.49.1
======================================================================
[PASS] Test 1: Database Initialization (18 Tables, 6 Views, 5 Triggers created)
[PASS] Test 2: Successful Insertion across Users, Instruments, Holdings & Transactions
[PASS] Test 3: Successful Retrieval (Verified table queries and dynamic view outputs)
[PASS] Test 4: Updates & Automatic Timestamp Trigger (Row updated and verified)
[PASS] Test 5: Deletion Policies (Verified ON DELETE CASCADE integrity)
[PASS] Test 6: Foreign Key Enforcement (PRAGMA foreign_keys = 1 confirmed)
[PASS] Test 7: UNIQUE Constraints (Prevented duplicate emails and asset symbols)
[PASS] Test 8: CHECK Constraints (Prevented negative prices & invalid enum statuses)
[PASS] Test 9: Invalid User References (Foreign Key rejection confirmed)
[PASS] Test 10: Invalid Investment References (Foreign Key rejection confirmed)
[PASS] Test 11: Duplicate Market Observations (Prevented duplicate timestamp quotes)
[PASS] Test 12: Transaction Rollback (Atomicity & ACID Rollback verified)
[PASS] Test 13: Schema-version Handling (Current Version: 1)
[PASS] Test 14: Initialization Without Sample Data (Verified zero mock rows in fresh database)
[PASS] Test 15: Initialization With Demonstration Data (Seed data, reports & triggers verified)
[PASS] Test 16: Prepared Statements & SQL Injection Immunity (Verified parameterized safety)
======================================================================
 FINAL RESULT: 16 / 16 TESTS PASSED
======================================================================
```

---

## 9. Conclusion

Phase 3 establishes a verified, robust data foundation. Every subsequent financial module, calculation engine, user authentication workflow, and UI view can now persist and query relational data through clean, type-safe repositories with ACID transactional guarantees.

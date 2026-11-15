# InvestSphere — Development Roadmap & Implementation Plan

This roadmap outlines the phased engineering milestones for **InvestSphere — Smart Investment & Portfolio Management System**, tracking development progress from foundational architecture through final evaluation.

---

## Roadmap Overview

```
Phase 1: Project Setup & Architecture Foundation   [COMPLETED]
Phase 2: Relational Schema & 3NF Normalization     [COMPLETED]
Phase 3: Database Access Layer & Core Data Models  [COMPLETED]
Phase 4: Financial Calculation Engine & Math Tests [COMPLETED]
Phase 5: User Authentication & Session Security    [COMPLETED]
Phase 6: Multi-Asset Investment Tracking Modules   [COMPLETED]
Phase 7: Unified Transactions Ledger & Audit Logs  [COMPLETED]
Phase 8: SIP/SWP Calculators & Maturity Tracker    [PLANNED]
Phase 9: Executive Dashboard & SQL Reports         [PLANNED]
Phase 10: Academic DBMS Evaluation & Final Polish  [PLANNED]
```

---

## Phase Breakdown

### Phase 1 — Project Setup, Architecture & Development Foundation
**Status: COMPLETED**
* [x] Environment inspection and toolchain discovery (compilers, CMake, Qt).
* [x] Creation of structured, modular project directory tree.
* [x] Root `CMakeLists.txt` build configuration with C++17 and Qt 6 detection.
* [x] Application entry point (`src/main.cpp`) with dual Qt 6 GUI and diagnostic console modes.
* [x] Minimal Qt 6 `MainWindow` establishing the design system, navy sidebar, and tokenized palette.
* [x] Comprehensive documentation (`README.md`, `PROJECT_OVERVIEW.md`, `SYSTEM_ARCHITECTURE.md`, `DEVELOPMENT_ROADMAP.md`).
* [x] Project `.gitignore` specification.
* [x] Build verification and status reporting.

---

### Phase 2 — Relational Schema, 3NF Normalization & SQL Scripts
**Status: COMPLETED**
* [x] Formal ER diagram design and entity-relationship mapping (`docs/ER_DIAGRAM.md`, `docs/er_diagram.mmd`).
* [x] Normalization proof (1NF, 2NF, 3NF) and functional dependency analysis (`docs/NORMALIZATION.md`).
* [x] Detailed Data Dictionary and relational design specification (`docs/DATABASE_DESIGN.md`).
* [x] Creation of `sql/schema.sql` (18 tables, primary keys, foreign keys, `CHECK` constraints, default values, 16 indexes).
* [x] Creation of `sql/views.sql` (6 analytical views for holdings, portfolio summary, P&L, ledger, upcoming maturities).
* [x] Creation of `sql/triggers.sql` (5 triggers for audit logging, excess sale prevention, timestamp updates, audit immutability).
* [x] Creation of `sql/seed_data.sql` (Fictional demonstration datasets across all 5 asset classes: Stocks, Funds, Gold, FDs, Bonds).
* [x] Creation of `sql/reports.sql` (22 DBMS concepts demonstrated + practical financial analytics).
* [x] C++ `DatabaseManager` singleton component (`src/database/DatabaseManager.h`, `DatabaseManager.cpp`).
* [x] Database test suites: 15/15 DBMS automated tests passed (`tests/verify_database.py`), native C++ test passing (`tests/test_database.cpp`).

---

### Phase 3 — Database Access Layer & Core Data Models
**Status: COMPLETED**
* [x] Implementation of `DatabaseManager` (persistent path resolution, WAL mode, foreign key enforcement, schema versioning).
* [x] Strongly typed C++17 Data Models in `src/models/`:
  - `UserModel`, `InstrumentModel`, `InvestmentModel`, `TransactionModel`, `SipPlanModel`, `SwpPlanModel`, `FinancialEventModel`.
* [x] Implementation of Data Access Layer (Repositories) in `src/database/`:
  - `UserRepository`, `InvestmentRepository`, `TransactionRepository`, `SipRepository`, `SwpRepository`, `FinancialEventRepository`.
* [x] Prepared statements and parameter binding across all repository queries (SQL injection immune).
* [x] RAII Transaction controller guard (`TransactionScope`) with automatic rollback on error.
* [x] Comprehensive automated testing: 16/16 C++ native tests passed (`tests/test_database.cpp`), 16/16 SQLite validation tests passed (`tests/verify_database.py`).
* [x] Detailed architecture and verification documentation (`docs/DATABASE_IMPLEMENTATION.md`).

---

### Phase 4 — Financial Calculation Engine & Math Tests
**Status: COMPLETED**
* [x] `src/finance/CostBasisEngine`: Weighted Average Cost (WAC) algorithm with exact integer minor-unit arithmetic.
* [x] `src/finance/CompoundingEngine`: Compound interest for FDs (Monthly/Quarterly/Half-Yearly/Annual/Simple).
* [x] Unrealized P&L calculation (current market value vs cost basis).
* [x] Realized P&L engine: sale proceeds minus WAC cost basis of units sold.
* [x] Phase 6 financial calculation tests (`tests/test_phase6_investments.cpp`): 15/15 passed.
* [x] Phase 7 calculation tests (`tests/test_phase7_transactions.cpp`): 8 dedicated math tests passed.

---

### Phase 5 — User Authentication & Session Security
**Status: COMPLETED**
* [x] User registration and credential storage with PBKDF2/SHA256 secure hashing.
* [x] Login and registration UI with client/service validation and error feedback.
* [x] User session state management ensuring strict isolation of portfolio records.
* [x] Profile and password update workflows with database persistence.

---

### Phase 6 — Interactive Dashboard & Multi-Asset Investment Management
**Status: COMPLETED**
* [x] **Interactive Dashboard:** 5 summary cards (Total Invested, Current Value, Unrealized P&L, Realized P&L, Active Holdings), real-time SQLite aggregations, vector Donut asset allocation chart, top holdings table, recent transactions ledger, upcoming financial events, and quick actions bar.
* [x] **Stocks UI & Service:** Add stock holding, exchange & sector mapping, WAC cost basis tracking, market price valuation, and unrealized return calculation.
* [x] **Mutual Funds UI & Service:** Scheme code, AMC, expense ratio, NAV tracking, and WAC cost basis.
* [x] **Gold UI & Service:** Karat purity (24K/22K), grams quantity, storage notes, and 24K spot valuation.
* [x] **Fixed Deposits UI & Service:** Bank institution, principal amount, compound interest engine (Monthly/Quarterly/Half-Yearly/Annual/Simple), auto maturity calculations, and financial events scheduling.
* [x] **Bonds UI & Service:** Issuer, coupon rate, payment frequency schedule, credit rating, and maturity tracking.
* [x] **Strict Non-Fabrication Policy:** Unquoted holdings strictly labeled "Unavailable" instead of fabricating prices.
* [x] **Integrity & Scoping:** Strict user ownership isolation (`WHERE user_id = :userId`) and `ON DELETE RESTRICT` transaction policy enforcement.

---

### Phase 7 — Unified Transactions Ledger & Portfolio Calculations
**Status: COMPLETED**
* [x] **TransactionService:** Atomic CRUD operations with `TransactionScope` RAII guards; full validation pipeline (user ownership, asset-class suitability, negative fee guard, net amount consistency, holding sufficiency).
* [x] **PortfolioCalculationService:** Single authoritative source of truth for WAC cost basis, unrealized/realized P&L, income, and portfolio summary aggregation.
* [x] **CostBasisEngine extended:** Separate income buckets for dividends, interest, fees, taxes; cumulative purchase turnover; net external cash flow.
* [x] **TransactionsPage UI:** Master ledger table with summary cards, search bar, type/date-range filters, and direct edit/delete action.
* [x] **TransactionDialog:** New/edit modal with live net-amount calculation and holding-balance hints.
* [x] **TransactionDetailsDialog:** Audit inspector with gross vs net breakdown.
* [x] **Duplicate protection:** Identical type + date + quantity + net on same investment rejected.
* [x] **Oversell prevention:** Delete and update safety checks simulate chronological timeline before committing.
* [x] **Multi-user isolation:** All queries scoped to `userId`; cross-user ownership access denied.
* [x] **Automated tests:** 20/20 Phase 7 tests passed (`tests/test_phase7_transactions.cpp`).
* [x] **Documentation:** `docs/TRANSACTION_MANAGEMENT.md` and `docs/FINANCIAL_CALCULATIONS.md` written.

---

### Phase 8 — SIP Calculator, SWP Calculator & Portfolio Analytics
**Status: COMPLETED**
* [x] **AnnuityEngine:** Pure C++17 financial math engine (`src/finance/AnnuityEngine.h`, `.cpp`) implementing monthly compounding ordinary annuity SIP, step-up SIP schedule, and month-by-month SWP capital depletion model.
* [x] **SipCalculatorPage UI:** Two-way synchronized spinboxes and sliders, reset defaults button, summary cards, principal-vs-gain allocation bar, and 40-year growth projection schedule table.
* [x] **SwpCalculatorPage UI:** Interactive parameters, 5 summary cards, dynamic depletion warning banner with exact month/year exhaustion detection, and year-by-year cash flow table.
* [x] **AnalyticsService & PortfolioSnapshotRepository:** Aggregation service querying `PortfolioCalculationService`, multi-category breakdowns, valuation coverage diagnostics, and daily snapshot deduplication.
* [x] **AnalyticsPage UI:** Executive summary cards, category analysis table, top performers, worst performers (lagging holdings), snapshot history table with empty state, time filters (1M/3M/6M/1Y/All), category filter, and manual snapshot capture.
* [x] **Multi-user isolation:** Strict query scoping to authenticated user; no cross-user leakage.
* [x] **Automated Tests:** 23/23 Phase 8 tests passed (`tests/test_phase8_analytics.cpp`).
* [x] **Zero Regressions:** 100% of test suites passed across all 5 modules via `ctest`.
* [x] **Documentation:** `docs/SIP_CALCULATOR.md`, `docs/SWP_CALCULATOR.md`, `docs/PORTFOLIO_ANALYTICS.md`, and updated `docs/FINANCIAL_CALCULATIONS.md`.

---

### Phase 9 — Executive Dashboard, Analytics & Reporting
**Status: PLANNED**
* [ ] Executive dashboard overview cards (Total Invested, Current Value, Net Realized P&L, Net Unrealized P&L, Return %).
* [ ] Asset allocation pie/donut chart and distribution table.
* [ ] Top performing investments and recent transaction widgets.
* [ ] SQL-driven reporting module generating consolidated PDF/CSV summaries.

---

### Phase 10 — Academic DBMS Evaluation & Final Polish
**Status: PLANNED**
* [ ] Verification of all 22 required DBMS concepts.
* [ ] Preparation of SQL execution log and normalization defense documentation.
* [ ] Complete UI consistency check (color palette, spacing, responsive resizing).
* [ ] End-to-end integration testing and presentation rehearsal guide.

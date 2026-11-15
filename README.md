# InvestSphere — Smart Investment & Portfolio Management System

> **Tagline:** *"One Platform. Every Investment. Smarter Decisions."*

InvestSphere is a native C++ desktop investment management application built with Qt 6 and SQLite. Developed as a comprehensive academic Database Management System (DBMS) course project, InvestSphere solves the problem of fragmented portfolio tracking across disjoint platforms by providing a centralized, relational, and mathematically rigorous management environment.

---

## 1. Project Overview

Many investors hold stocks, mutual funds, gold, fixed deposits, and bonds across separate brokers, banking portals, and offline certificates. This fragmentation introduces major friction:
* Inability to compute true consolidated portfolio value and asset allocation.
* Confusion between total purchase cost and current market valuation.
* Miscalculation of realized vs. unrealized gains and losses.
* Lack of integrated forecasting (SIP compounding, SWP withdrawal sustainability).
* Untracked maturity schedules for fixed deposits and bond coupon dates.

InvestSphere integrates transaction recording, multi-asset portfolio accounting, financial calculators, maturity tracking, and database reporting into a unified desktop interface.

---

## 2. Technology Stack

* **Application Type:** Native Desktop Application
* **Programming Language:** C++17 or newer
* **User Interface:** Qt 6 Widgets (modular navigation, responsive layout)
* **Application Architecture:** Layered Architecture (Presentation, Service, Data Model, Financial Engine, Database Access)
* **Database Engine:** SQLite 3 (Permanent local relational store)
* **Database Connectivity:** Qt SQL module (`QSqlDatabase`, `QSqlQuery`) with prepared statements
* **Database Language:** Structured Query Language (SQL - DDL, DML, DQL)
* **Charts & Analytics:** Qt Charts (when available)
* **Build System:** CMake (3.16+)
* **Version Control:** Git

---

## 3. DBMS Concepts Demonstrated

InvestSphere is engineered specifically to demonstrate core relational database principles during college and course evaluation:

1. **Relational Schema Design & 3NF Normalization:** Zero transitive dependencies, strictly typed domain schemas, and clean surrogate/natural key design.
2. **Referential Integrity:** Mandatory foreign keys (`PRAGMA foreign_keys = ON;`) cascading appropriately on delete/update.
3. **Transaction Management & ACID Guarantees:** Multi-table operations (e.g. stock sales updating cash balances, asset holdings, and audit logs) wrapped in explicit `BEGIN TRANSACTION`, `COMMIT`, and `ROLLBACK` blocks.
4. **Complex SQL Queries:**
   - Multi-table `INNER JOIN` and `LEFT JOIN` operations across holdings, asset categories, and transactions.
   - Aggregate functions (`SUM()`, `AVG()`, `COUNT()`, `MIN()`, `MAX()`).
   - Grouping and filtering with `GROUP BY` and `HAVING`.
   - Correlated and non-correlated subqueries.
5. **Database Views:** Pre-compiled analytical views for consolidated portfolio summaries, asset allocation percentages, and realized P&L.
6. **Triggers:** Automated integrity and audit tracking (e.g. updating timestamp logs and recalculating remaining quantities).
7. **Indexing:** B-Tree indexes on user IDs, transaction timestamps, and asset tickers for high-efficiency lookups.
8. **SQL-Driven Reporting & CSV Exports:** Native SQL query pipelines exporting financial summaries.

---

## 4. System Architecture

InvestSphere follows a strict 6-layer decoupled design:

```
┌─────────────────────────────────────────────────────────────┐
│                   PRESENTATION LAYER (UI)                   │
│   Qt 6 Widgets • MainWindow • Sidebar Navigation • Dialogs  │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                  APPLICATION SERVICE LAYER                  │
│  PortfolioService • TransactionService • UserService etc.   │
└──────────────┬──────────────────────────────┬───────────────┘
               │                              │
┌──────────────▼──────────────┐┌──────────────▼───────────────┐
│      DATA MODEL LAYER       ││  FINANCIAL CALCULATION LAYER │
│  Holding • Transaction etc. ││  Cost Basis • SIP/SWP • P&L  │
└──────────────┬──────────────┘└──────────────┬───────────────┘
               │                              │
┌──────────────▼──────────────────────────────▼───────────────┐
│                    DATABASE ACCESS LAYER                    │
│   DatabaseManager • Prepared SQL Queries • Transactions     │
└──────────────────────────────┬──────────────────────────────┘
                               │
┌──────────────────────────────▼──────────────────────────────┐
│                    STORAGE ENGINE: SQLite                   │
│         Relational Tables • Views • Triggers • 3NF          │
└─────────────────────────────────────────────────────────────┘
```

---

## 5. Project Directory Structure

```
InvestSphere/
├── CMakeLists.txt              # Primary CMake build specification
├── README.md                   # Project overview & documentation
├── .gitignore                  # Git exclusion rules
│
├── docs/                       # Architectural & design documentation
│   ├── PROJECT_OVERVIEW.md     # Detailed course & functional overview
│   ├── SYSTEM_ARCHITECTURE.md  # Layered architecture & design patterns
│   └── DEVELOPMENT_ROADMAP.md  # Phased milestones & task tracking
│
├── resources/                  # Application resources
│   ├── icons/                  # UI navigation icons
│   ├── images/                 # Brand assets & logos
│   └── styles/                 # Qt Style Sheets (QSS)
│
├── sql/                        # Relational database scripts
│   ├── schema.sql              # Table definitions & constraints (3NF)
│   ├── seed_data.sql           # Initial seed data for testing
│   ├── views.sql               # Database views for reporting
│   ├── triggers.sql            # Integrity triggers & audit logs
│   ├── reports.sql             # Complex analytical SQL queries
│   └── migrations/             # Schema migration scripts
│
├── src/                        # C++ Source Code
│   ├── main.cpp                # Application entry point
│   ├── app/                    # Application lifecycle & configuration
│   ├── ui/                     # Presentation layer (Qt 6 Widgets)
│   │   ├── mainwindow/         # Main window, sidebar & header
│   │   ├── dashboard/          # Summary metrics & quick overview
│   │   ├── portfolio/          # Multi-asset portfolio view
│   │   ├── stocks/             # Stock holding & trade management
│   │   ├── mutualfunds/        # Mutual fund scheme & NAV tracking
│   │   ├── gold/               # Gold investments
│   │   ├── fixeddeposits/      # Term deposits & maturity schedules
│   │   ├── bonds/              # Bond holdings & coupon schedules
│   │   ├── transactions/       # Unified transaction ledger
│   │   ├── calculators/        # SIP and SWP planners
│   │   ├── analytics/          # Asset allocation & performance
│   │   ├── market/             # Market watch & price tracker
│   │   ├── maturity/           # Upcoming maturity tracker
│   │   ├── reports/            # Financial reporting & export UI
│   │   └── settings/           # User preferences & configuration
│   ├── models/                 # Pure data transfer objects & entity models
│   ├── database/               # Database manager, DAO, and query runners
│   ├── services/               # Application business logic layer
│   ├── finance/                # Financial math algorithms (P&L, SIP, SWP)
│   └── utils/                  # String, date, and math helpers
│
├── tests/                      # Unit & integration tests
└── build/                      # Build output directory
```

---

## 6. Build & Setup Instructions

### Prerequisites
* **C++ Compiler:** C++17 compatible (GCC 7+, Clang 5+, or MSVC 2019+)
* **CMake:** Version 3.16 or higher
* **Qt 6:** Qt 6.x (Modules: `Qt6Widgets`, `Qt6Sql`, and optionally `Qt6Charts`)
* **SQLite:** SQLite 3 (included with Qt 6 SQL SQLite driver)

### Building with CMake

1. Clone or open the workspace:
   ```bash
   cd c:/InvestSphere
   ```

2. Generate the build configuration:
   ```bash
   # When Qt 6 is in standard path or specified via CMAKE_PREFIX_PATH
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```

3. Build the binary:
   ```bash
   cmake --build build --config Release
   ```

4. Run InvestSphere:
   ```bash
   ./build/InvestSphere
   ```

---

## 7. Development Phases

* **Phase 1 (Completed):** Project Setup, Architecture & Development Foundation.
* **Phase 2 (Completed):** Relational Database Design, 3NF Normalization, Schema & SQL Scripts.
* **Phase 3 (Completed):** SQLite Database Implementation, Repositories, Models & Integration.
* **Phase 4 (Completed):** Financial Calculation Engine & Unit Tests.
* **Phase 5 (Completed):** User Authentication & Session Management.
* **Phase 6 (Completed):** Portfolio & Investment Management Modules (Stocks, Funds, Gold, FDs, Bonds).
* **Phase 7 (Completed):** Transactions Ledger & Auditing.
* **Phase 8 (Completed):** Financial Calculators (SIP/SWP) & Maturity Tracker.
* **Phase 9 (Completed):** Dashboard, Analytics & SQL-Driven Reporting.
* **Phase 10 (Completed):** Complete Code Audit, Error Fixing, Web SPA, and Public Cloud Deployment.

---

## 8. Web Application & Public Cloud Deployment

InvestSphere includes a responsive, zero-install Web Application in [`web/`](file:///C:/InvestSphere/web) that runs in any modern browser on desktop, tablet, and mobile (iOS & Android).

### Architecture
- **Frontend (`web/frontend/`):** Responsive HTML5/CSS3/Vanilla JS Single-Page Application (SPA) with mobile-first sidebar, financial summary cards, dynamic asset allocation bars, interactive modals, and real-time SIP/SWP planning.
- **Backend (`web/backend/`):** Python/Flask REST API server running on Gunicorn WSGI. Directly utilizes the production relational SQLite schema (`sql/schema.sql`, `views.sql`, `triggers.sql`), PBKDF2-SHA256 password hashing, and parameterized query execution.
- **Deployment Platform:** Ready for 1-click deployment on [Render.com](https://render.com) using [`render.yaml`](file:///C:/InvestSphere/render.yaml) or [`Procfile`](file:///C:/InvestSphere/Procfile).

### Public Cloud Access
- **Target URL:** `https://investsphere.onrender.com` *(configurable on Render deployment)*
- **Health Check Endpoint:** `GET /api/health`

### Demonstration Account Credentials
Evaluators and project guides can sign in immediately using fictional demonstration data without manual entry:
- **Email:** `demo@investsphere.demo`
- **Password:** `DemoInvestor123!`
- **Holdings:** 5 diverse asset holdings (RELIANCE stock, TCS stock, NIFTYBEES ETF, GOLDBEES gold, and SBI Fixed Deposit with maturity tracking).

### Running Web Application Locally
```bash
# 1. Install dependencies
pip install -r web/backend/requirements.txt

# 2. Run local development server
python web/backend/app.py

# 3. Open browser
http://localhost:5000
```

### Full Deployment Documentation
For comprehensive step-by-step cloud deployment instructions, environment variable references, and operational considerations, see [`docs/DEPLOYMENT_GUIDE.md`](file:///C:/InvestSphere/docs/DEPLOYMENT_GUIDE.md) and [`docs/FINAL_CODE_AUDIT.md`](file:///C:/InvestSphere/docs/FINAL_CODE_AUDIT.md).

---

## 9. License

Academic DBMS Course Project — Developed for educational demonstration and evaluation.

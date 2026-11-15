# InvestSphere — System Architecture & Technical Design

## 1. Architectural Philosophy

InvestSphere is engineered following a clean, decoupled **Multi-Layer Architecture**. In accordance with modern C++ and Qt software engineering principles, UI components never execute raw SQL queries or perform complex financial calculations directly. Instead, each responsibility is isolated into dedicated layers.

```
+-------------------------------------------------------------------------+
|                         PRESENTATION LAYER                              |
|   Qt 6 Widgets, QMainWindow, SideBarNav, Custom Cards, QTableView, Dialogs   |
+-------------------------------------------------------------------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                      APPLICATION SERVICE LAYER                          |
|   AuthService, PortfolioService, TransactionService, ReportService      |
+-------------------------------------------------------------------------+
                    |                                 |
                    v                                 v
+-----------------------------+     +-------------------------------------+
|      DATA MODEL LAYER       |     |     FINANCIAL CALCULATION LAYER     |
|  User, Holding, Trade,      |     |  ValuationEngine, CostBasisEngine,  |
|  Deposit, Bond, Plan DTOs   |     |  SIPCalculator, SWPCalculator       |
+-----------------------------+     +-------------------------------------+
                    |                                 |
                    +----------------+----------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                       DATABASE ACCESS LAYER                             |
|   DatabaseManager, DAOs (HoldingDao, TransactionDao, UserDao), Prepared |
|   SQL Statements, Connection Configuration, ACID Transaction Controllers|
+-------------------------------------------------------------------------+
                                     |
                                     v
+-------------------------------------------------------------------------+
|                         PERSISTENCE ENGINE                              |
|         SQLite 3 (WAL Mode, Foreign Key Constraints, 3NF Schema)        |
+-------------------------------------------------------------------------+
```

---

## 2. Detailed Layer Specifications

### Layer A: Presentation Layer (UI)
* **Technology:** Qt 6 Widgets (`QWidget`, `QMainWindow`, `QDialog`, `QTableView`, `QLayout`).
* **Responsibilities:**
  - Rendering responsive, visually premium user interface components.
  - Capturing user inputs and triggering validation.
  - Forwarding user actions to the Application Service Layer.
  - Displaying formatted financial data and error notifications.
* **Key Design Rules:**
  - No database connectivity or SQL query strings permitted within widgets.
  - No direct calculation of compounding or tax formulas inside UI event handlers.
  - Fully dynamic resizing using Qt layout managers (`QVBoxLayout`, `QHBoxLayout`, `QGridLayout`).

### Layer B: Application Service Layer
* **Responsibilities:**
  - Orchestrates business workflows across multiple entities.
  - Handles transaction safety (coordinates transactions across DAOs).
  - Validates business rules (e.g. verifying that a user cannot sell more shares than currently held).
  - Bridges the presentation layer and data access layer.
* **Key Components:**
  - `AuthService`: Registration, session state, password verification.
  - `PortfolioService`: Aggregates multi-asset balances, calculates category weights.
  - `TransactionService`: Executes buys, sells, deposits, and income entries atomically.
  - `ReportService`: Orchestrates analytical queries and serializes reports to CSV.

### Layer C: Data Model Layer
* **Responsibilities:**
  - Provides strongly-typed C++ Data Transfer Objects (DTOs) representing domain entities.
  - Value types with immutable IDs, encapsulation, and type safety.
* **Key Entities:**
  - `User`: ID, username, email, password hash, created date.
  - `Investment`: ID, user ID, asset type (Stock, MF, Gold, FD, Bond), symbol, name.
  - `Holding`: Asset details, remaining quantity, average cost basis, total invested.
  - `Transaction`: ID, holding ID, type (BUY, SELL, DIVIDEND, INTEREST), quantity, price, fees, timestamp.
  - `FixedDeposit`: ID, user ID, bank name, principal, rate, start date, maturity date, maturity value.
  - `Bond`: ID, user ID, issuer, face value, coupon rate, payment frequency, maturity date.
  - `FinancialPlan`: ID, user ID, plan type (SIP, SWP), monthly amount, expected rate, tenure.

### Layer D: Financial Calculation Layer
* **Responsibilities:**
  - Pure mathematical and financial functions, completely decoupled from Qt and SQLite.
  - Guaranteed deterministic results with strict rounding and precision management.
* **Key Algorithms:**
  - `CostBasisEngine`: Computes Weighted Average Cost (WAC) on incremental purchases and updates remaining cost pool on partial sales.
  - `ProfitLossEngine`: Segregates Realized Gains/Losses from Unrealized Gains/Losses.
  - `CompoundInterestEngine`: Implements compound interest formulas for FDs across annual, semi-annual, quarterly, and monthly frequencies:
    $$A = P \left(1 + \frac{r}{n}\right)^{nt}$$
  - `AnnuityEngine`: Computes SIP future wealth accumulation and SWP capital longevity curves.

### Layer E: Database Access Layer (DAL)
* **Responsibilities:**
  - Encapsulates all interactions with the SQLite database.
  - Manages database lifecycles, connections, migrations, and schema verification.
  - Implements the Data Access Object (DAO) pattern.
  - Uses prepared statements (`QSqlQuery::prepare()`) exclusively to prevent SQL injection vulnerabilities.
* **Connection & Safety Protocol:**
  - **Foreign Key Enforcement:** Explicitly executes `PRAGMA foreign_keys = ON;` immediately upon opening every connection.
  - **Concurrency & Reliability:** Configures Write-Ahead Logging (`PRAGMA journal_mode = WAL;`) and synchronous normal mode for atomic reliability without performance degradation.
  - **ACID Transaction Boundaries:** Uses `db.transaction()`, `db.commit()`, and `db.rollback()` around all multi-statement operations.

---

## 3. Design Patterns Utilized

1. **Repository / DAO Pattern:**
   Data access logic is encapsulated inside dedicated DAO classes (`HoldingDao`, `TransactionDao`, `UserDao`), keeping SQL queries isolated and testable.
2. **Service Layer Pattern:**
   Centralizes high-level business rules, ensuring presentation logic remains lightweight and decoupled.
3. **Singleton / Central Manager Pattern:**
   `DatabaseManager` provides a controlled, single point of configuration and lifecycle management for the SQLite database connection.
4. **Model-View Architecture (Qt):**
   Qt's model/view framework (`QAbstractTableModel`) is utilized for high-performance rendering of tabular transaction ledgers and holdings grids.

---

## 4. UI/UX Visual Design Tokens

| Token | Hex Value | Application |
| :--- | :--- | :--- |
| **Sidebar Background** | `#0F172A` | Fixed navigation container (Navy Blue) |
| **Active Nav Item** | `#2563EB` | Active page indicator (Accent Blue) |
| **Nav Hover State** | `#1E293B` | Interactive hover feedback |
| **Main Canvas** | `#F8FAFC` | Primary workspace canvas background |
| **Card Background** | `#FFFFFF` | Metric, list, and form card surfaces |
| **Card Border** | `#E2E8F0` | Subtle clean boundary stroke |
| **Primary Text** | `#0F172A` | Headings, active values, high emphasis |
| **Secondary Text** | `#64748B` | Labels, descriptions, dates |
| **Positive Metric** | `#10B981` | Positive returns, profits, inflows (Green) |
| **Negative Metric** | `#EF4444` | Losses, negative returns, fees (Red) |
| **Warning/Pending** | `#F59E0B` | Upcoming maturities, alerts (Amber) |

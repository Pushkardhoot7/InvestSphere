# InvestSphere — Project Overview & Requirements Specification

## 1. Executive Summary

**InvestSphere** is a centralized, native desktop investment and portfolio management platform designed to solve the challenges of modern multi-asset wealth management. Developed as a flagship academic Database Management System (DBMS) course project, InvestSphere demonstrates the practical application of relational database modeling, normalization (1NF, 2NF, 3NF), ACID transaction processing, analytical SQL queries, and clean layered software architecture in C++17 and Qt 6.

* **Project Title:** InvestSphere — Smart Investment & Portfolio Management System
* **Tagline:** *"One Platform. Every Investment. Smarter Decisions."*
* **Core Technology:** C++17, Qt 6 Widgets, SQLite 3, CMake

---

## 2. Problem Statement & Motivation

Modern individual investors distribute their savings across multiple diverse asset classes:
1. **Equity Stocks:** Active market equities requiring share tracking, average cost calculations, and realized P&L computation on partial exits.
2. **Mutual Funds:** Systematic Investment Plans (SIP), NAV valuation, and unit allocation.
3. **Precious Metals (Gold):** Physical or digital gold holdings measured in grams with fluctuating spot valuations.
4. **Fixed Deposits (FD):** Fixed-term banking instruments requiring compounding interest calculations and maturity tracking.
5. **Government & Corporate Bonds:** Fixed-income securities with face values, periodic coupon payment dates, and maturity horizons.

Currently, investors face critical pain points:
* **Information Fragmentation:** Managing multiple brokerage portals, banking apps, and physical documents leads to a disjointed view of net worth.
* **Cost vs. Value Confusion:** Investors frequently confuse total capital deployed (acquisition cost) with present market valuation, misinterpreting their true financial health.
* **Conflation of Realized and Unrealized Gains:** Mixing closed trades with fluctuating open positions distorts tax and performance accounting.
* **Missed Maturities & Cash Flows:** Fixed deposit maturities and bond coupon payments often go unmonitored, resulting in idle funds.
* **Lack of Data Ownership:** Cloud portals expose private financial data to third-party tracking; a local, secure SQLite-based application guarantees user privacy and offline availability.

InvestSphere directly addresses these issues with a single, unified, offline-first relational software system.

---

## 3. Core System Modules

The system is organized into 16 discrete, high-cohesion functional modules:

### Module 1: Authentication & User Management
* Multi-user support with isolated portfolio partitions.
* Secure credential hashing and salting.
* Session tracking and logout lifecycle.

### Module 2: Executive Dashboard
* Consolidated Total Invested Capital vs. Current Portfolio Valuation.
* Segregated Realized P&L and Unrealized P&L metrics.
* Overall Portfolio Return Percentage (Absolute and XIRR/CAGR foundations).
* High-level asset allocation breakdown.
* Activity feed of recent transactions and upcoming maturity alerts.

### Module 3: My Portfolio (Consolidated)
* Multi-asset portfolio grid grouping investments by asset class.
* Filtering by asset type, holding status (active vs. exited), and search by ticker/name.
* Detailed holding drill-down displaying purchase history and cost basis.

### Module 4: Stocks Module
* Buy and sell transaction logging with share quantity, unit price, and brokerage fees.
* Weighted average cost basis tracking.
* Dynamic remaining share quantity calculation.
* Realized gain/loss calculation on sales and unrealized gain/loss against latest market quotes.

### Module 5: Mutual Funds Module
* Fund schemes, category classification (Equity, Debt, Hybrid), and folio numbers.
* NAV recording with historical valuation tracking.
* Lump-sum and automated SIP installment linking.

### Module 6: Gold Holdings Module
* Gold purchases recorded by weight (grams), purity (24K, 22K), and acquisition price.
* Spot market price benchmarking for estimated valuation.

### Module 7: Fixed Deposits (FD) Module
* Principal deposit amount, issuing financial institution, interest rate, and compounding frequency (Monthly, Quarterly, Annually, Simple).
* Automated maturity date and maturity payout calculation.
* Maturity alert tracking.

### Module 8: Bonds Module
* Issuer details, face value, coupon interest rate, and payment frequency (Annual, Semi-annual).
* Scheduled coupon payment timeline and maturity redemption tracking.

### Module 9: Unified Transactions Ledger
* Immutable transaction history (Buy, Sell, Deposit, Withdrawal, Dividend, Interest).
* Filtering by date range, transaction type, and asset category.
* CSV export capabilities.

### Module 10: SIP Calculator (Systematic Investment Plan)
* Mathematical future value projection using compound annuity formulas:
  $$FV = P \times \frac{(1 + r)^n - 1}{r} \times (1 + r)$$
* Visual comparison between Total Invested Capital and Estimated Wealth Gain.
* Saving custom SIP calculation plans directly to the database.

### Module 11: SWP Calculator (Systematic Withdrawal Plan)
* Mathematical balance depletion modeling for retirement and systematic cash flows.
* Monthly withdrawal schedule projection with remaining capital trajectory.
* Plan persistence for future reference.

### Module 12: Portfolio Analytics
* Asset allocation diversification analysis (Equity vs. Debt vs. Gold vs. Fixed Income).
* Category concentration risk metrics.
* Monthly investment cadence tracking.

### Module 13: Market Watch
* Local catalog of market quotes and mutual fund NAVs.
* Watchlist management for tracking potential opportunities.
* Explicit identification of quote timestamps (no false claims of live data).

### Module 14: Maturity Tracker
* Centralized timeline of upcoming FD maturities, bond coupon dates, bond redemptions, and scheduled SIP/SWP dates.
* Status classification (Pending, Matured, Collected).

### Module 15: Financial Reports & SQL Analytics
* Structured SQL reporting engine generating:
  - Portfolio Valuation Summary Report
  - Capital Gains & Tax-prep Report (Realized P&L)
  - Annual Dividend & Interest Income Report
  - Category Performance Breakdown
* Native CSV export functionality for all reports.

### Module 16: System Settings & Preferences
* Currency formatting preferences (e.g., USD $, INR ₹, EUR €).
* Date format conventions (YYYY-MM-DD, DD/MM/YYYY).
* Local database backup and restore facilities.

---

## 4. Financial Calculation Standards

To guarantee mathematical consistency and financial accuracy:

1. **Monetary Precision:** Stored monetary values use exact integer representations (cents/paise) or bounded double precision with explicit round-to-nearest-cent logic to eliminate IEEE 754 floating-point drift.
2. **Cost Basis Standard:** Weighted Average Cost (WAC) method is utilized as the primary baseline for equity and mutual funds, with clear tracking of partial dispositions.
3. **Separation of Concerns in P&L:**
   - **Realized P&L:** Locked upon sale transaction execution:
     $$\text{Realized P\&L} = (\text{Sale Price} - \text{Cost Basis}) \times \text{Sold Quantity} - \text{Transaction Fees}$$
   - **Unrealized P&L:** Dynamic paper gain/loss based on active holding quantity:
     $$\text{Unrealized P\&L} = (\text{Current Market Price} - \text{Cost Basis}) \times \text{Remaining Quantity}$$
4. **Dividends and Interest:** Recorded as separate income transactions. Never deducted from purchase costs or double-counted as portfolio appreciation.
5. **Cash Deposits:** Treated strictly as capital injections, never distorted as investment yield.

---

## 5. DBMS Academic Course Alignment

InvestSphere explicitly fulfills the curriculum requirements of an advanced university Database Management Systems course:

| DBMS Concept | Practical Implementation in InvestSphere |
| :--- | :--- |
| **Relational Modeling** | Normalized schema spanning Users, Assets, Holdings, Transactions, FDs, Bonds, Plans |
| **Normalization (1NF - 3NF)** | Elimination of repeating groups, partial dependencies, and transitive attributes |
| **Referential Integrity** | SQLite Foreign Key cascades, restrictive deletion rules, and not-null constraints |
| **ACID Transactions** | Atomic commit/rollback on multi-row trade settlement and account ledger updates |
| **Complex Joins** | Multi-table `INNER JOIN` across Assets and Quotes; `LEFT JOIN` for partial holdings |
| **Aggregations & Grouping** | `SUM()`, `AVG()` with `GROUP BY asset_category` and `HAVING total_value > 0` |
| **Subqueries** | Correlated subqueries fetching the latest quote per ticker for current valuation |
| **Views & Triggers** | `vw_portfolio_summary`, `vw_realized_gains`, and audit triggers for transaction updates |
| **Indexing Strategy** | B-Tree indexes on `user_id`, `holding_id`, `transaction_date`, and `symbol` |

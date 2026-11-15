# Portfolio Analytics & Historical Snapshots Documentation

InvestSphere provides an enterprise-grade analytics engine and interactive UI dashboard for portfolio diagnostics, multi-asset allocation, performance ranking, and snapshot-based historical tracking.

---

## 1. System Architecture

```
┌────────────────────────────────────────────────────────┐
│             InvestSphere UI Layer                      │
│   AnalyticsPage (Overview, Categories, Performers,     │
│                  Snapshots Table, Filters)             │
└──────────────────────────┬─────────────────────────────┘
                           │ delegates
┌──────────────────────────▼─────────────────────────────┐
│          InvestSphere::Services::AnalyticsService      │
│   - computeAnalytics(userId, filterCat, startDate, ...) │
│   - recordSnapshotIfValuationAvailable(userId)        │
│   - getSnapshotHistory(userId, startDate, endDate)    │
└──────────────┬──────────────────────────┬──────────────┘
               │                          │
┌──────────────▼─────────────┐ ┌──────────▼──────────────┐
│ PortfolioCalculationService│ │PortfolioSnapshotRepository
│ (Holdings, WAC, Realized/  │ │ (portfolio_snapshots    │
│  Unrealized P&L, Quotes)   │ │  table, deduplication)  │
└────────────────────────────┘ └─────────────────────────┘
```

The analytics module does **not** duplicate mathematical formulas. It aggregates and queries through `PortfolioCalculationService` (established in Phase 7) and persists valuation history through `PortfolioSnapshotRepository`.

---

## 2. Core Capabilities

### A. Portfolio Overview
Provides instant summary metrics across active positions:
- **Total Cost Basis**: Cumulative capital invested across all active holdings.
- **Estimated Portfolio Value**: Real-time market valuation of all holdings with live market quotes.
- **Unrealized Profit / Loss**: Dollar value and percentage return against quoted cost basis.
- **Realized Profit / Loss**: Cumulative net capital gain/loss from executed sale transactions.
- **Income (Dividends & Interest)**: Total cash distributions collected from equities, mutual funds, and fixed deposits.
- **Active Holdings & Valuation Coverage**: Distinguishes between quoted and unquoted holdings, clearly reporting the percentage of the portfolio that has verified market pricing.

### B. Asset Allocation by Category
Categorizes holdings into standard asset classes:
- Stocks (`STOCK`)
- Mutual Funds (`MUTUAL_FUND`)
- Gold & Precious Metals (`GOLD`)
- Fixed Deposits (`FIXED_DEPOSIT`)
- Bonds & Debentures (`BOND`)
- Other Supported Assets

Calculates category-level cost basis, market value, unrealized gain, realized gain, and **allocation percentage** ($M_{\text{cat}} / \sum M_{\text{quoted}} \times 100$).

### C. Best and Worst Performers
Only holdings with verified market prices and positive cost basis are ranked:
$$\text{Unrealized Return } \% = \frac{\text{Market Value} - \text{Cost Basis}}{\text{Cost Basis}} \times 100$$
- **Top Performers**: Sorted by return percentage descending.
- **Lagging Holdings**: Sorted ascending (worst return first).
- Positions lacking market valuation quotes are strictly excluded from performance rankings to prevent distorted return diagnostics.

### D. Historical Snapshots (`portfolio_snapshots`)
- Captures point-in-time valuation records: `snapshot_timestamp`, `total_invested`, `estimated_portfolio_value`, `realized_profit_loss`, and `unrealized_profit_loss`.
- **Automatic Deduplication**: If multiple snapshots are captured on the same calendar day for the same user, the existing record is updated in-place rather than creating redundant rows.
- **Non-Fabrication Guarantee**: Snapshots are never written or backfilled for dates prior to the entry of real valuation data.
- **Time Filters**: Supports filtering history by All Time, Last 1 Month, Last 3 Months, Last 6 Months, and Last 1 Year.

---

## 3. Database Schema

```sql
CREATE TABLE IF NOT EXISTS portfolio_snapshots (
    snapshot_id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id                     INTEGER NOT NULL,
    snapshot_timestamp          TEXT NOT NULL DEFAULT (datetime('now')),
    total_invested              INTEGER NOT NULL CHECK (total_invested >= 0),
    estimated_portfolio_value   INTEGER NOT NULL CHECK (estimated_portfolio_value >= 0),
    realized_profit_loss        INTEGER NOT NULL,
    unrealized_profit_loss      INTEGER NOT NULL,
    notes                       TEXT,
    FOREIGN KEY (user_id) REFERENCES users(user_id) 
        ON DELETE CASCADE ON UPDATE CASCADE
);
```

---

## 4. Multi-User Isolation & Security

1. **User Scoping**: Every query to `PortfolioSnapshotRepository` and `AnalyticsService` includes `userId` parameter sourced from the authenticated `SessionManager`.
2. **Database Queries**: Prepared SQL statements prevent injection vulnerabilities.
3. **Data Protection**: User A cannot view, query, or overwrite User B's portfolio valuations or historical snapshots.

---

## 5. Automated Verification Results

All 23 test scenarios in `tests/test_phase8_analytics.cpp` pass without errors:
- Multi-category aggregation: Stocks and Mutual Funds aggregated into respective categories with correct allocation percentages.
- Valuation coverage: Unquoted holdings are segregated and reported accurately in coverage metrics without corrupting valuation calculations.
- Deduplication: Same-day snapshot updates properly overwrite without creating redundant rows.
- Multi-user isolation: Separate portfolios under distinct user accounts remain isolated and independent.

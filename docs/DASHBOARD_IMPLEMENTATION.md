# InvestSphere — Dashboard Implementation & Analytics Architecture

## 1. Executive Dashboard Overview
The **InvestSphere Interactive Dashboard** serves as the central command cockpit for the investor, loaded immediately upon successful authentication. It consolidates high-level portfolio performance metrics, asset allocation breakdowns, top holdings, recent ledger activity, and upcoming cash flows directly from the SQLite database.

---

## 2. Executive Summary Cards (5 Metrics)

The dashboard presents five primary performance cards at the top:

| Summary Card | Calculation Source | Behavior When Quotes Missing | Trend Badge |
|---|---|---|---|
| **Total Invested** | Cumulative cost of active positions: $\sum (\text{Remaining Qty} \times \text{WAC})$ | Strictly computed from purchase costs | Displays `"Initial Cost"` (Neutral) |
| **Current Estimated Value** | $\sum (\text{Remaining Qty} \times \text{CMP})$ | Displays `"Unavailable"` if any active asset lacks a quote | Overall portfolio gain/loss % or `"Pending"` |
| **Unrealized Profit/Loss** | $\text{Current Value} - \text{Total Invested}$ | Displays `"N/A"` if valuation is incomplete | Positive (Green) / Negative (Red) delta % |
| **Realized Profit/Loss** | $\sum_{\text{closed sales}} (\text{Sell Price} - \text{WAC}) \times \text{Qty}$ | Computed from historical sales ledger | Net Gain (Green) / Net Loss (Red) |
| **Active Holdings** | Count of holdings where $\text{status} = \text{'ACTIVE'}$ | Real-time count across all 5 asset classes | Subtitle indicates unquoted holding count |

---

## 3. Visualizations: Vector Donut Chart Fallback

To support environments where Qt Charts (`Qt6Charts`) is not available or bundled, InvestSphere includes a custom vector-rendered anti-aliased Donut chart in `ChartPlaceholder`:
- **Render Engine**: QPainter with `Antialiasing` and `SmoothPixmapTransform` flags.
- **Visual Design**:
  - Center hole radius (56% of outer diameter) displaying central portfolio context.
  - Slice arc rendering proportional to category allocation percentages.
  - Distinct asset class color palette:
    - **Stocks**: Royal Blue (`#2563EB`)
    - **Mutual Funds**: Emerald Green (`#059669`)
    - **Gold**: Amber Gold (`#D97706`)
    - **Fixed Deposits**: Violet Purple (`#7C3AED`)
    - **Bonds**: Cyan Blue (`#0891B2`)
  - Integrated interactive legend on the right showing category names, allocation percentages, and invested amounts.
  - Graceful empty-state ring and helper message when no active holdings exist.

---

## 4. Top Holdings & Activity Tables

### 4.1. Top Holdings Section
- Lists top 5 portfolio holdings ranked by invested capital and market valuation.
- Displays Asset/Symbol, Category badge, Remaining Quantity, Invested Capital, Current Value (or "Unavailable"), and Unrealized P&L badge.

### 4.2. Recent Transactions Activity
- Displays the 5 most recent ledger events across all asset classes.
- Columns: Date, Instrument Symbol/Name, Transaction Type badge (`BUY`, `SELL`, `DEPOSIT`, `DIVIDEND`), Units, and Net Amount.

### 4.3. Upcoming Financial Events & Maturities
- Monitors scheduled cash flow milestones within the next 30-90 days.
- Sourced from `financial_events` table (populated on Fixed Deposit creation and bond issuance).
- Columns: Date, Event Type (`FD Maturity`, `Bond Coupon`), Description/Details, and Estimated Cash Flow.

---

## 5. Quick Actions Bar & Cross-Page Routing

Located at the top of the dashboard, the Quick Actions bar enables one-click workflows:
- **`+ Add Investment`**: Launches `InvestmentDialog` modal with dynamic category switching.
- **`Record Transaction`**: Navigates to Transactions Ledger page.
- **`SIP Calculator`**: Navigates directly to the Systematic Investment Plan calculator.
- **`SWP Calculator`**: Navigates directly to the Systematic Withdrawal Plan calculator.
- **`Refresh`**: Instantly triggers `DashboardService::instance().getDashboardData(userId)` to re-aggregate live SQLite metrics.

# InvestSphere — Investment Management Architecture & Implementation

## 1. Overview
The **Investment Management** module in InvestSphere provides end-to-end portfolio management capabilities across five distinct financial asset classes:
1. **Equity Stocks** (NSE / BSE listed equities, exchanges, sectors)
2. **Mutual Funds** (AMC schemes, fund categories, expense ratios, NAV)
3. **Physical / Digital Gold** (24K/22K purity, karatage, form factor, grams)
4. **Fixed Deposits** (Banking institutions, compounding frequencies, maturity dates)
5. **Government & Corporate Bonds** (Coupon yields, payment frequencies, credit ratings)

All holdings, transactions, and valuations are strictly backed by persistent SQLite storage with foreign key constraints, atomic `TransactionScope` boundaries, and strict user ownership isolation.

---

## 2. Relational Database Schema & Specialization Mapping

The data model follows Third Normal Form (3NF) relational design:
```
+-------------------------------------------------------------------------+
|                              instruments                                |
| (id, symbol, name, investment_type_id, isin, currency, is_active, ...)  |
+-------------------------------------------------------------------------+
                                    | 1
                                    |
                                    | N
+-------------------------------------------------------------------------+
|                              investments                                |
| (id, user_id, instrument_id, portfolio_tag, status, notes, created_at)  |
+-------------------------------------------------------------------------+
        |                  |                 |              |          |
        | 1:1              | 1:1             | 1:1          | 1:1      | 1:1
        v                  v                 v              v          v
   +----------+    +--------------+    +------------+   +--------+  +-------+
   |  stocks  |    | mutual_funds |    | gold_hold. |   |   fd   |  | bonds |
   +----------+    +--------------+    +------------+   +--------+  +-------+
```

### Table Mappings:
1. **`instruments`**: Master instrument definitions and exchange symbols (e.g. `TCS.NS`, `HDFCBANK.BO`, `GOLD24K`).
2. **`investments`**: User portfolio ownership linking a `user_id` to an `instrument_id`, tracking holding status (`ACTIVE`, `SOLD`, `MATURED`).
3. **Specialization Tables**:
   - `stocks`: `exchange`, `sector`, `market_cap_category`
   - `mutual_funds`: `fund_house`, `category`, `plan_type`, `expense_ratio`, `scheme_code`
   - `gold_holdings`: `form_factor`, `purity_karat`, `storage_location`
   - `fixed_deposits`: `institution_name`, `account_number`, `principal_amount_cents`, `annual_interest_rate`, `compounding_frequency`, `payout_type`, `start_date`, `maturity_date`, `duration_months`, `estimated_maturity_amount_cents`
   - `bonds`: `issuer_name`, `bond_type`, `face_value_cents`, `coupon_rate`, `coupon_frequency`, `issue_date`, `maturity_date`, `credit_rating`
4. **`transactions`**: Double-entry ledger tracking `BUY`, `SELL`, `DEPOSIT`, `WITHDRAWAL`, `DIVIDEND`, and `INTEREST`.
5. **`financial_events`**: Projected cash flow schedule for FD maturities and bond coupon payments.

---

## 3. Financial Calculation Engines

### 3.1. Cost Basis Engine (`CostBasisEngine`)
- **Weighted Average Cost (WAC)**:
  $$\text{New WAC} = \frac{(\text{Current Qty} \times \text{Old WAC}) + (\text{Buy Qty} \times \text{Buy Price})}{\text{Current Qty} + \text{Buy Qty}}$$
- **Realized P&L on Partial/Full Sales**:
  $$\text{Realized P\&L} = \text{Sell Qty} \times (\text{Sell Price} - \text{WAC})$$
- **Unrealized P&L on Active Holdings**:
  $$\text{Unrealized P\&L} = \text{Holding Qty} \times (\text{CMP} - \text{WAC})$$
  $$\text{Return \%} = \frac{\text{CMP} - \text{WAC}}{\text{WAC}} \times 100$$

### 3.2. Compounding Engine (`CompoundingEngine`)
Computes periodic interest and maturity amounts for Fixed Deposits:
- **Compound Interest**:
  $$A = P \left(1 + \frac{r}{n}\right)^{n \cdot t}$$
  Where:
  - $P$ = Principal amount
  - $r$ = Annual interest rate
  - $n$ = Compounding frequency per year ($12$ Monthly, $4$ Quarterly, $2$ Half-Yearly, $1$ Annually)
  - $t$ = Duration in years ($\text{durationMonths} / 12.0$)
- **Simple Interest**:
  $$A = P \cdot (1 + r \cdot t)$$

---

## 4. Business Rules & Strict Non-Fabrication Policy

### 4.1. Valuation Integrity
- **Strict Prohibition Against Fabricating CMP**:
  If no market quote exists in `market_prices` or `mutual_fund_nav`, the current market price and market value must be explicitly returned as `Unavailable` / `N/A`.
  **Under no circumstances is CMP fabricated as the original purchase price.**
- **Quoted Valuation**:
  When a quote exists, `Market Value = Remaining Quantity * Latest Market Price`.

### 4.2. Referential Integrity & Deletion Policies
- **`ON DELETE RESTRICT` Enforcement**:
  Investments linked to existing transaction records in `transactions` cannot be silently or orphaned-deleted.
- **Cascading Confirmation**:
  If a user confirms deletion of a holding that has linked ledger records, all associated records (`transactions`, `financial_events`, specialization rows) are cleanly deleted within an atomic SQLite transaction boundary.

### 4.3. Multi-Tenant User Isolation
- Every query in `InvestmentRepository` and `InvestmentService` enforces `WHERE user_id = :userId`.
- User A cannot view, edit, search, or delete User B's investment records or transactions.

---

## 5. UI Architecture & Dialogs

### 5.1. `InvestmentDialog`
- Dynamic category switching (Stocks, Mutual Funds, Gold, Fixed Deposits, Bonds).
- Context-sensitive form inputs:
  - Fixed Deposits: Live auto-calculation of projected maturity amount as principal, interest rate, duration, and compounding frequency are changed.
  - Mutual Funds: Scheme code, AMC, expense ratio, and plan type.
  - Stocks: Exchange, sector, and market cap classification.
- Real-time client-side and service-layer validation with error banner display.

### 5.2. `InvestmentDetailsDialog`
- Full-screen modal inspector displaying holding statistics, avg cost, current price, unrealized return, and status.
- Category specialization details layout.
- Embedded audit ledger (`ModernTableWidget`) showing historical `BUY`/`SELL`/`DEPOSIT` events.
- Direct "Edit" and safe "Delete" actions.

### 5.3. `PortfolioPage`
- Master portfolio overview with consolidated cost and asset class split summary cards.
- Search filter field (keyword search across symbols and names).
- Category filter dropdown and status filter dropdown.
- Holdings table with Indian currency formatting (`₹1,25,000.00`), status badges, and action buttons ("View", "Edit", "Delete").

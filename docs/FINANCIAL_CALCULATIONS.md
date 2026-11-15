# Financial Calculations — Developer Reference

## Overview

InvestSphere uses **exact integer arithmetic** (minor units / paise) throughout to avoid floating-point accumulation errors. All monetary values are stored and computed as `int64_t` cents. Floating-point (`double`) is used only for unit quantities and percentage display.

---

## CostBasisEngine (WAC Algorithm)

**File:** [`src/finance/CostBasisEngine.h`](file:///c:/InvestSphere/src/finance/CostBasisEngine.h) / `.cpp`

### Weighted Average Cost

For each investment, all transactions are sorted chronologically. The engine maintains a running:

- `runningQty` — total units currently held
- `runningCostCents` — total cost basis of current holdings

**On BUY / DEPOSIT:**
```
runningQty   += buyQty
runningCostCents += netAmountCents        // net = gross + fees + taxes
```

**On SELL / REDEMPTION:**
```
avgCost          = runningCostCents / runningQty
costOfUnitsSold  = round(sellQty * avgCost)
realizedPnl     += netProceeds - costOfUnitsSold
runningQty      -= sellQty
runningCostCents -= costOfUnitsSold
```

**WAC per unit:**
```
WAC = runningCostCents / runningQty      (integer division with rounding)
```

### Income Buckets (Separated from Capital Gains)

| Source      | Accumulates in         |
|-------------|------------------------|
| DIVIDEND    | `totalDividendsCents`  |
| INTEREST    | `totalInterestCents`   |
| SELL/REDEEM | `totalRealizedPnlCents`|
| FEE         | `totalFeesPaidCents`   |

Income (dividends, interest) does **not** affect the cost basis of remaining holdings.

### CostBasisResult Fields

| Field | Description |
|-------|-------------|
| `remainingQuantity` | Units still held |
| `totalInvestedCostCents` | Cost basis of remaining units (Qty × WAC) |
| `averageCostPerUnitCents` | WAC in minor units |
| `totalRealizedPnlCents` | Capital gains from completed sales |
| `totalRealizedProceedsCents` | Net sale proceeds |
| `totalRealizedCostCents` | Cost basis attributed to sold units |
| `totalDividendsCents` | Net dividend income |
| `totalInterestCents` | Net interest income |
| `totalIncomeCents` | Dividends + Interest |
| `totalFeesPaidCents` | All brokerage/transaction fees |
| `totalTaxesPaidCents` | All taxes paid |
| `cumulativePurchaseTurnoverCents` | All-time gross purchase capital deployed |
| `netExternalCashFlowCents` | Net capital invested (purchases − proceeds − income) |
| `transactionCount` | Total number of transactions |

---

## Unrealized P&L

```cpp
static bool CostBasisEngine::calculateUnrealizedPnl(
    double remainingQuantity,
    int64_t totalCostCents,
    int64_t currentUnitPriceCents,
    int64_t &outCurrentValueCents,
    int64_t &outUnrealizedPnlCents,
    double  &outPnlPercent);
```

**Formulae:**
```
currentValue    = round(remainingQty * currentUnitPrice)
unrealizedPnl   = currentValue - costBasis
pnlPercent      = (unrealizedPnl / costBasis) × 100
```

Returns `false` (and zeroes all outputs) if `remainingQty ≤ 0` or `currentPrice < 0`.

> [!IMPORTANT]
> InvestSphere strictly refuses to fabricate market prices. `hasMarketQuote = false` when no real market data has been provided. The dashboard and portfolio views clearly distinguish between "known market value" and "cost basis only" positions.

---

## PortfolioCalculationService

**File:** [`src/services/PortfolioCalculationService.h`](file:///c:/InvestSphere/src/services/PortfolioCalculationService.h) / `.cpp`

Central aggregator providing a single source of truth for all holdings math.

### PortfolioSummaryDTO

| Field | Description |
|-------|-------------|
| `activeHoldingsCount` | Investments with remaining qty > 0 |
| `quotedHoldingsCount` | Subset with live market quotes |
| `unquotedHoldingsCount` | Subset without market quotes |
| `valuationCoveragePercent` | `quoted / active × 100` |
| `totalInvestedCostBasisCents` | Sum of cost bases for all active holdings |
| `totalCurrentEstimatedValueCents` | Sum of market values for quoted holdings only |
| `totalUnrealizedPnlCents` | Current value of quoted holdings − their cost basis |
| `unrealizedPnlPercent` | Unrealized P&L / cost basis of quoted holdings |
| `totalRealizedPnlCents` | All-time realized capital gains (all transactions) |
| `totalDividendsCents` | All-time dividend income |
| `totalInterestCents` | All-time interest income |
| `totalIncomeCents` | Dividends + Interest |
| `totalFeesPaidCents` | All-time brokerage fees |
| `totalTaxesPaidCents` | All-time taxes paid |
| `cumulativePurchaseTurnoverCents` | All-time gross purchase outlay |
| `netExternalCashFlowCents` | Net capital deployed externally |

### Holding Sufficiency Validation

Before accepting SELL / REDEMPTION / WITHDRAWAL, the service simulates the chronological transaction stream (excluding any transaction being updated) and verifies the running balance never goes negative:

```cpp
std::pair<bool,string> validateHoldingQuantityFeasibility(
    int userId, int investmentId,
    const string &proposedType, double proposedQty,
    const string &proposedDate,
    int ignoreTxId = 0);
```

---

## CompoundingEngine (Fixed Deposit Math)

**File:** [`src/finance/CompoundingEngine.h`](file:///c:/InvestSphere/src/finance/CompoundingEngine.h) / `.cpp`

Compound interest formula:
```
MaturityAmount = Principal × (1 + r/n)^(n×t)
```
where:
- `r` = annual interest rate as decimal
- `n` = compounding periods per year (MONTHLY=12, QUARTERLY=4, SEMI_ANNUALLY=2, ANNUALLY=1)
- `t` = tenure in years

All results are returned as `int64_t` minor units.

---

## Key Numerical Invariants

| Invariant | Description |
|-----------|-------------|
| Non-negative WAC | WAC per unit is always ≥ 0 |
| Non-negative cost basis | After sell-out, cost basis resets to exactly 0 |
| Quantity conservation | Sum(BUY) − Sum(SELL) = remainingQuantity at all times |
| Integer rounding | All cents use `std::round()` — never truncated |
| Tolerance | Net amount consistency allows ±5 minor units for UI rounding |

---

## Testing

Phase 7 tests validate all calculations: see [`tests/test_phase7_transactions.cpp`](file:///c:/InvestSphere/tests/test_phase7_transactions.cpp)

```
Test  1: WAC multi-tranche buy (40 @ $100 + 60 @ $120 = WAC $112)
Test  2: Partial sell realized P&L (WAC method)
Test  3: Full sell-out clears cost basis to zero
Test  4: Dividend income isolated from capital gains
Test  5: Interest income isolated from capital gains
Test  6: Fee accumulation in separate bucket
Test  7: Unrealized P&L — profit scenario (+20%)
Test  8: Unrealized P&L — loss scenario (−16.67%)
```

---

## 7. Systematic Investment Plan (SIP) Engine

**File:** [`src/finance/AnnuityEngine.h`](file:///c:/InvestSphere/src/finance/AnnuityEngine.h) / [`.cpp`](file:///c:/InvestSphere/src/finance/AnnuityEngine.cpp)

### Standard Ordinary Annuity Model
End-of-month contribution assumption:
```
FV = P × [((1 + r)^n − 1) / r]
```
where:
- `P` = monthly contribution amount
- `r` = annual expected return divided by 1200 (`annualRate / 12 / 100`)
- `n` = total number of monthly contributions (`years * 12`)

Zero-rate fallback:
```
FV = P × n
```

### Annual Step-Up Model
For an annual step-up rate `s = stepUp% / 100`:
- Monthly contribution in year `y`: `P_y = P * (1 + s)^(y-1)`
- Running corpus compounds forward year-by-year:
  ```
  corpus_y = corpus_(y-1) * (1 + r)^12 + P_y * [((1 + r)^12 - 1) / r]
  ```

---

## 8. Systematic Withdrawal Plan (SWP) Engine

**File:** [`src/finance/AnnuityEngine.h`](file:///c:/InvestSphere/src/finance/AnnuityEngine.h) / [`.cpp`](file:///c:/InvestSphere/src/finance/AnnuityEngine.cpp)

### Monthly Sequence of Operations
Monthly rate: `r = annualRate / 12 / 100`.
For each month:
1. **Growth Phase**: `balance = balance * (1 + r)`
2. **Withdrawal Phase**:
   - If `balance >= monthlyWithdrawal`: `balance -= monthlyWithdrawal`
   - If `balance < monthlyWithdrawal` and `balance > 0`: `totalWithdrawn += balance`, `balance = 0`, corpus depleted.
   - If `balance == 0`: withdrawals cease, depletion month recorded.
3. Balance is strictly non-negative (`balance >= 0`).

---

## 9. Phase 8 Automated Test Results

Automated unit tests validate all calculations: see [`tests/test_phase8_analytics.cpp`](file:///c:/InvestSphere/tests/test_phase8_analytics.cpp)

```
Test  1: SIP — Zero return rate (FV = P * n)                         : PASS
Test  2: SIP — Positive return rate (12% 10yr compounding)           : PASS (FV=2300386)
Test  3: SIP — 1-year investment horizon                             : PASS
Test  4: SIP — 20-year long duration compounding                     : PASS
Test  5: SIP — Invalid monthly contribution rejected                 : PASS
Test  6: SIP — Invalid duration and rate ranges rejected             : PASS
Test  7: SIP — Step-up SIP annual accumulation schedule              : PASS
Test  8: SIP — Benchmark verification against textbook value         : PASS
Test  9: SWP — Zero return rate linear depletion                     : PASS
Test 10: SWP — Positive return with growth before withdrawal         : PASS
Test 11: SWP — Sustainable withdrawal preserves corpus               : PASS
Test 12: SWP — Depletion tracking with partial final withdrawal      : PASS
Test 13: SWP — Zero withdrawal pure compounding                      : PASS
Test 14: SWP — Invalid corpus and negative withdrawal rejected       : PASS
Test 15: SWP — 5-year timeline breakdown structure                   : PASS
Test 16: Analytics — Empty portfolio graceful response               : PASS
Test 17: Snapshots — Record and retrieve portfolio snapshot          : PASS
Test 18: Snapshots — Same-day deduplication and update               : PASS
Test 19: Analytics — Multi-category aggregation                      : PASS
Test 20: Analytics — Valuation coverage distinguishes unquoted assets: PASS
Test 21: Analytics — Top and worst performer ranking                 : PASS
Test 22: Analytics — Category filtering                              : PASS
Test 23: Analytics — Strict multi-user portfolio data isolation      : PASS
```

Run with:
```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

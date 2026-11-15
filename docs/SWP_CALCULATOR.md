# Systematic Withdrawal Plan (SWP) Calculator Documentation

InvestSphere provides a pure-math, month-by-month Systematic Withdrawal Plan (SWP) calculation engine and interactive user interface for retirement and cash flow planning.

---

## 1. Mathematical Model and Order of Operations

### Monthly Compounding Convention
The monthly return rate is computed using the **nominal annual rate divided by 12**:
$$r = \frac{R}{100 \times 12}$$
where $R$ is the expected annual return rate in percent.

### Monthly Order of Calculation
For each month $m \in [1, \text{totalMonths}]$:
1. **Growth Phase**: Investment return is applied to the opening balance:
   $$\text{interest} = \text{balance} \times r$$
   $$\text{balance} \leftarrow \text{balance} + \text{interest}$$
2. **Withdrawal Phase**:
   - If $\text{balance} \ge \text{withdrawal}$:
     $$\text{balance} \leftarrow \text{balance} - \text{withdrawal}$$
     $$\text{totalWithdrawn} \leftarrow \text{totalWithdrawn} + \text{withdrawal}$$
     $$\text{withdrawalsCompleted} \leftarrow \text{withdrawalsCompleted} + 1$$
   - If $\text{balance} < \text{withdrawal}$ and $\text{balance} > 0$:
     $$\text{partialWithdrawn} = \text{balance}$$
     $$\text{totalWithdrawn} \leftarrow \text{totalWithdrawn} + \text{partialWithdrawn}$$
     $$\text{balance} \leftarrow 0$$
     $$\text{depleted} \leftarrow \text{true}, \quad \text{depletedMonth} \leftarrow m$$
     Withdrawals cease immediately.
   - If $\text{balance} = 0$:
     Corpus was exhausted at or before this month; remaining scheduled withdrawals cease.

### Protection Against Negative Balances
The balance is strictly clamped at zero ($\text{balance} \ge 0$). At no point can a negative balance occur.

---

## 2. Architecture & Components

- **Finance Layer (`InvestSphere::Finance::AnnuityEngine::calculateSwp`)**:
  - Stateless C++17 implementation (`src/finance/AnnuityEngine.h`, `.cpp`).
  - Tracks monthly interest accrual, withdrawal amounts, depletion month, and year-by-year projections (`SwpYearlyProjection`).
- **UI Layer (`InvestSphere::Ui::SwpCalculatorPage`)**:
  - Dual-column responsive layout (`src/ui/calculators/SwpCalculatorPage.h`, `.cpp`).
  - Interactive QDoubleSpinBox and QSlider controls for Initial Corpus, Monthly Withdrawal, Expected Return Rate, and Duration.
  - Summary Cards: Initial Investment, Total Amount Withdrawn, Estimated Remaining Balance, Investment Earnings, and Withdrawals Completed.
  - **Dynamic Depletion Warning Banner**: Displays prominent warning banner in red with exact Year and Month of corpus exhaustion when depletion is projected.
  - Year-wise timeline table (`QTableWidget`) showing annual cumulative withdrawals and remaining corpus.

---

## 3. Input Parameters & Constraints

| Parameter | Range | Default | Behavior on Violation |
| :--- | :--- | :--- | :--- |
| **Initial Corpus** | ₹1,000 to ₹10,00,00,000 | ₹25,00,000 | Rejects values < ₹1,000 |
| **Monthly Withdrawal** | ₹0 to ₹50,00,000 | ₹20,000 | Rejects negative numbers |
| **Expected Return Rate** | 0.0% to 30.0% | 8.0% | Rejects negative or > 100% |
| **Withdrawal Duration** | 1 to 40 Years | 15 Years | Rejects < 1 or > 50 Years |

---

## 4. Verification & Automated Test Cases

The test suite in `tests/test_phase8_analytics.cpp` verifies:
- **Zero return rate**: Initial corpus ₹1,00,000 with ₹10,000/month withdrawal depletes at Month 10 with final balance ₹0 and total withdrawn ₹1,00,000.
- **Growth outstripping withdrawals**: ₹10,00,000 corpus with ₹5,000/month withdrawal at 12% p.a. results in ending balance strictly higher than initial principal.
- **Depletion tracking**: Detects exact depletion month when withdrawals exhaust capital before the selected duration, ensuring partial last withdrawal is properly accounted for.
- **Zero withdrawal scenario**: Functions identically to compound interest accumulation over time.
- **Invalid input detection**: Rejects sub-minimum corpus and negative cash flow parameters.

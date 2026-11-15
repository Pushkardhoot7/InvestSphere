# Systematic Investment Plan (SIP) Calculator Documentation

InvestSphere provides a pure-math, high-precision Systematic Investment Plan (SIP) calculator engine and interactive user interface.

---

## 1. Mathematical Model and Formulas

### Contribution Timing Assumption
The SIP calculator assumes **end-of-month (ordinary annuity)** cash flows, which corresponds to the standard convention used by mutual fund asset management companies (AMCs) and Indian financial institutions.

### Standard SIP Formula
Let:
- $P$ = Monthly installment amount (in ₹)
- $R$ = Nominal annual expected return rate (in %)
- $r = \frac{R}{100 \times 12}$ = Effective monthly decimal return rate
- $n = \text{duration in years} \times 12$ = Total monthly installments

The future maturity value $FV$ is given by:

$$FV = P \times \left[ \frac{(1 + r)^n - 1}{r} \right]$$

### Zero-Rate Fallback
When expected annual return rate $R = 0$ ($r = 0$), division by zero is avoided, and future value equals the total invested principal:

$$FV = P \times n$$

### Step-Up SIP Formula
For an optional annual step-up rate $S\%$ ($s = S / 100$):
- In year $y \in [1, Y]$, the monthly contribution is:
  $$P_y = P \times (1 + s)^{y-1}$$
- Each year $y$, the existing corpus from year $y-1$ compounds for 12 months at rate $r$:
  $$\text{grownCorpus}_y = \text{corpus}_{y-1} \times (1 + r)^{12}$$
- The 12 monthly contributions during year $y$ form an ordinary annuity with future value:
  $$FV_{\text{year}, y} = P_y \times \left[ \frac{(1 + r)^{12} - 1}{r} \right]$$
- Running corpus at the end of year $y$:
  $$\text{corpus}_y = \text{grownCorpus}_y + FV_{\text{year}, y}$$

Total principal invested:
$$\text{Total Invested} = \sum_{y=1}^{Y} 12 \times P_y$$

Wealth Gain and Return Percentage:
$$\text{Estimated Gain} = FV - \text{Total Invested}$$
$$\text{Return } \% = \frac{\text{Estimated Gain}}{\text{Total Invested}} \times 100$$

---

## 2. Architecture & Components

- **Finance Layer (`InvestSphere::Finance::AnnuityEngine`)**:
  - Stateless, pure C++17 mathematical service (`src/finance/AnnuityEngine.h`, `.cpp`).
  - No database or network dependencies.
  - Generates year-by-year projections (`SipYearlyProjection`) with exact integer cents tracking and floating-point return rates.
- **UI Layer (`InvestSphere::Ui::SipCalculatorPage`)**:
  - Dual-column responsive layout built with Qt 6 Widgets (`src/ui/calculators/SipCalculatorPage.h`, `.cpp`).
  - Two-way synchronized QDoubleSpinBox and QSlider controls for monthly amount, expected return rate, duration, and step-up percentage.
  - Interactive summary cards: Total Invested, Estimated Gain, Projected Maturity Corpus, and Total Installments.
  - Capital allocation gradient bar showing visual proportion of Principal vs Gain.
  - Year-by-year breakdown table (`QTableWidget`) showing annual cumulative investment, market value, and wealth gain.
  - Input reset button and inline validation warnings.

---

## 3. Input Constraints & Edge Cases

| Input Field | Minimum | Maximum | Default | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **Monthly Contribution** | ₹100 | ₹1,00,00,000 | ₹10,000 | Rejects values < ₹1 |
| **Return Rate (% p.a.)** | 0.0% | 50.0% | 12.0% | 0.0% safely handled |
| **Investment Duration** | 1 Year | 40 Years | 10 Years | In whole years |
| **Annual Step-Up Rate** | 0.0% | 50.0% | 0.0% | 0% = standard fixed SIP |

---

## 4. Verification and Benchmark Accuracy

Automated unit tests in `tests/test_phase8_analytics.cpp`:
- Zero-rate test: P=₹10,000, 5 years $\implies$ Total Invested = ₹6,00,000, Gain = ₹0.
- Positive compounding test: P=₹10,000, 12% p.a., 10 years $\implies$ $FV = ₹23,00,387$ (matches standard AMC SIP benchmarks within ₹1).
- Step-up compounding: 10% annual increase validated to produce strictly higher corpus than standard SIP.
- Invalid input rejection: Negative values and out-of-range tenures return `res.valid = false` with explanatory error strings.

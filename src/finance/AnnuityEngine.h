#ifndef INVESTSPHERE_ANNUITYENGINE_H
#define INVESTSPHERE_ANNUITYENGINE_H

#include <cstdint>
#include <vector>
#include <string>

namespace InvestSphere {
namespace Finance {

/**
 * @brief Year-wise SIP projection data point.
 */
struct SipYearlyProjection {
    int year{0};
    int64_t investedAtYearEndCents{0};      ///< Cumulative principal invested by end of this year
    int64_t portfolioValueAtYearEndCents{0}; ///< Projected corpus value at end of this year
    int64_t wealthGainAtYearEndCents{0};     ///< Gross gain = portfolioValue - invested
    double  returnPercent{0.0};              ///< Return % at end of this year
};

/**
 * @brief SIP calculation result.
 *
 * Contribution timing assumption: END-OF-MONTH (ordinary annuity).
 *   FV = P × [((1 + r)^n − 1) / r]
 * where r = annualRate / 12 / 100 and n = months.
 *
 * For zero rate:
 *   FV = P × n
 *
 * For step-up SIP: the monthly amount increases by stepUpPercent each year.
 * Calculated by iterating 12-month periods, applying the new monthly amount
 * for each period and compounding the prior year's corpus forward.
 */
struct SipResult {
    bool valid{false};
    std::string errorMessage;

    double  monthlyAmountInitial{0.0};  ///< Starting monthly contribution
    double  annualStepUpPercent{0.0};   ///< Step-up rate per year (0 = standard SIP)
    double  annualReturnPercent{0.0};   ///< Expected annual return rate
    int     durationYears{0};
    int     totalInstallments{0};

    int64_t totalInvestedCents{0};          ///< Sum of all installments in minor units
    int64_t maturityValueCents{0};          ///< Projected corpus at end of tenure
    int64_t estimatedGainCents{0};          ///< maturityValue − totalInvested
    double  returnPercent{0.0};             ///< Gain / totalInvested × 100
    double  xirr{0.0};                     ///< Approximate annualized rate (same as input for standard SIP)

    std::vector<SipYearlyProjection> yearlyBreakdown;
};

/**
 * @brief Year-wise SWP data point.
 */
struct SwpYearlyProjection {
    int     year{0};
    double  withdrawalMonthly{0.0};         ///< Monthly withdrawal amount this year
    int64_t totalWithdrawnThisYearCents{0}; ///< Total withdrawn across 12 months this year
    int64_t balanceAtYearEndCents{0};       ///< Corpus remaining at end of this year
    bool    depletedThisYear{false};        ///< Whether corpus ran out during this year
    int     depletedAtMonth{0};             ///< 1-based month of depletion (0 = not depleted)
};

/**
 * @brief SWP calculation result.
 *
 * Calculation order (per month): Growth first, then withdrawal.
 *   balance = balance × (1 + monthlyRate)
 *   balance -= withdrawal   (if balance >= withdrawal, else extract remainder)
 *
 * Monthly rate convention: nominal annual rate / 12
 *   i.e. monthlyRate = annualRate / 100 / 12
 */
struct SwpResult {
    bool valid{false};
    std::string errorMessage;

    double  initialCorpus{0.0};
    double  monthlyWithdrawal{0.0};
    double  annualReturnPercent{0.0};
    int     durationYears{0};

    int64_t initialCorpusCents{0};
    int64_t totalWithdrawnCents{0};         ///< Sum of all successful withdrawal amounts
    int64_t finalBalanceCents{0};           ///< Remaining corpus at end (0 if depleted)
    int64_t totalEarningsCents{0};          ///< Gross interest credited over tenure
    int     withdrawalsCompleted{0};        ///< Number of months where withdrawal occurred
    bool    depleted{false};                ///< True if corpus ran out before tenure end
    int     depletedAtMonth{0};             ///< 1-based month of depletion (0 = not depleted)
    int     depletedAtYear{0};              ///< Approximate year of depletion (0 = not depleted)

    std::vector<SwpYearlyProjection> yearlyBreakdown;
};

/**
 * @brief Pure-math financial calculation engine for SIP and SWP projections.
 *
 * All monetary inputs are in plain double (no minor-unit conversion needed for
 * a hypothetical calculator).  Outputs are in both double and int64_t cents (×100).
 *
 * This engine is stateless and requires no database connection.
 */
class AnnuityEngine {
public:
    /**
     * @brief Calculate standard or step-up SIP future value.
     *
     * @param monthlyAmountStart  Starting monthly contribution (≥ 1)
     * @param annualReturnPercent Expected annualised return rate (≥ 0, ≤ 100)
     * @param durationYears       Investment horizon in whole years (≥ 1, ≤ 50)
     * @param annualStepUpPercent Annual SIP step-up (0 = standard SIP, ≥ 0, ≤ 100)
     * @return SipResult with detailed breakdown
     */
    static SipResult calculateSip(double monthlyAmountStart,
                                  double annualReturnPercent,
                                  int    durationYears,
                                  double annualStepUpPercent = 0.0);

    /**
     * @brief Calculate SWP depletion and balance timeline.
     *
     * @param initialCorpus       Starting portfolio corpus (≥ 1000)
     * @param monthlyWithdrawal   Fixed monthly withdrawal (≥ 0)
     * @param annualReturnPercent Expected annualised return rate (≥ 0, ≤ 100)
     * @param durationYears       Withdrawal horizon in whole years (≥ 1, ≤ 50)
     * @return SwpResult with detailed breakdown and depletion info
     */
    static SwpResult calculateSwp(double initialCorpus,
                                  double monthlyWithdrawal,
                                  double annualReturnPercent,
                                  int    durationYears);

private:
    static bool validateSipInputs(double monthly, double rate, int years, double stepUp,
                                  std::string &err);
    static bool validateSwpInputs(double corpus, double withdrawal, double rate, int years,
                                  std::string &err);
};

} // namespace Finance
} // namespace InvestSphere

#endif // INVESTSPHERE_ANNUITYENGINE_H

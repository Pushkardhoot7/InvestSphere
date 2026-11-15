#ifndef INVESTSPHERE_COMPOUNDINGENGINE_H
#define INVESTSPHERE_COMPOUNDINGENGINE_H

#include <cstdint>
#include <string>

namespace InvestSphere {
namespace Finance {

/**
 * @brief Financial calculation engine for term deposits (Fixed Deposits).
 * Supports monthly, quarterly, semi-annual, annual, and simple interest compounding.
 */
class CompoundingEngine {
public:
    /**
     * @brief Computes projected maturity value for a Fixed Deposit.
     * @param principalCents Principal deposit in minor units (cents)
     * @param annualRatePercent Annual interest rate (e.g. 7.10)
     * @param durationMonths Tenure in calendar months
     * @param frequency Compounding frequency: 'MONTHLY', 'QUARTERLY', 'HALF_YEARLY', 'ANNUALLY', 'SIMPLE'
     * @param payoutType Payout mode: 'CUMULATIVE' (compounding until maturity) or periodic payout
     * @return Total maturity payout in cents
     */
    static int64_t calculateMaturityAmount(int64_t principalCents,
                                          double annualRatePercent,
                                          int durationMonths,
                                          const std::string &frequency = "QUARTERLY",
                                          const std::string &payoutType = "CUMULATIVE");

    /**
     * @brief Computes total interest earned over the deposit tenure in cents.
     */
    static int64_t calculateTotalInterest(int64_t principalCents,
                                         double annualRatePercent,
                                         int durationMonths,
                                         const std::string &frequency = "QUARTERLY",
                                         const std::string &payoutType = "CUMULATIVE");
};

} // namespace Finance
} // namespace InvestSphere

#endif // INVESTSPHERE_COMPOUNDINGENGINE_H

#include "CompoundingEngine.h"
#include <cmath>
#include <algorithm>

namespace InvestSphere {
namespace Finance {

int64_t CompoundingEngine::calculateMaturityAmount(int64_t principalCents,
                                                  double annualRatePercent,
                                                  int durationMonths,
                                                  const std::string &frequency,
                                                  const std::string &payoutType)
{
    if (principalCents <= 0 || annualRatePercent <= 0.0 || durationMonths <= 0) {
        return principalCents;
    }

    double P = static_cast<double>(principalCents);
    double r = annualRatePercent / 100.0;
    double t = static_cast<double>(durationMonths) / 12.0;

    // If periodic payout (non-cumulative), principal remains constant and interest is paid out periodically:
    // Total gross payout returned at maturity is the principal.
    if (payoutType != "CUMULATIVE") {
        return principalCents;
    }

    if (frequency == "SIMPLE") {
        // A = P * (1 + r * t)
        double A = P * (1.0 + r * t);
        return static_cast<int64_t>(std::round(A));
    }

    double n = 4.0; // Default quarterly
    if (frequency == "MONTHLY") {
        n = 12.0;
    } else if (frequency == "QUARTERLY") {
        n = 4.0;
    } else if (frequency == "HALF_YEARLY") {
        n = 2.0;
    } else if (frequency == "ANNUALLY") {
        n = 1.0;
    }

    // Standard Compound Interest formula: A = P * (1 + r / n)^(n * t)
    double A = P * std::pow(1.0 + (r / n), n * t);
    return static_cast<int64_t>(std::round(A));
}

int64_t CompoundingEngine::calculateTotalInterest(int64_t principalCents,
                                                 double annualRatePercent,
                                                 int durationMonths,
                                                 const std::string &frequency,
                                                 const std::string &payoutType)
{
    if (principalCents <= 0 || annualRatePercent <= 0.0 || durationMonths <= 0) {
        return 0;
    }

    double P = static_cast<double>(principalCents);
    double r = annualRatePercent / 100.0;
    double t = static_cast<double>(durationMonths) / 12.0;

    if (payoutType != "CUMULATIVE" || frequency == "SIMPLE") {
        double interest = P * r * t;
        return static_cast<int64_t>(std::round(interest));
    }

    int64_t maturity = calculateMaturityAmount(principalCents, annualRatePercent, durationMonths, frequency, payoutType);
    return std::max<int64_t>(0, maturity - principalCents);
}

} // namespace Finance
} // namespace InvestSphere

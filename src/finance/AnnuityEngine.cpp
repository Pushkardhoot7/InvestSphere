#include "AnnuityEngine.h"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace InvestSphere {
namespace Finance {

// ─────────────────────────────────────────────────────────────────────────────
// Input validation helpers
// ─────────────────────────────────────────────────────────────────────────────

bool AnnuityEngine::validateSipInputs(double monthly, double rate, int years, double stepUp,
                                      std::string &err)
{
    if (monthly < 1.0) {
        err = "Monthly investment amount must be at least ₹1.";
        return false;
    }
    if (rate < 0.0 || rate > 100.0) {
        err = "Expected return rate must be between 0% and 100%.";
        return false;
    }
    if (years < 1 || years > 50) {
        err = "Investment duration must be between 1 and 50 years.";
        return false;
    }
    if (stepUp < 0.0 || stepUp > 100.0) {
        err = "Annual step-up percentage must be between 0% and 100%.";
        return false;
    }
    return true;
}

bool AnnuityEngine::validateSwpInputs(double corpus, double withdrawal, double rate, int years,
                                      std::string &err)
{
    if (corpus < 1000.0) {
        err = "Initial corpus must be at least ₹1,000.";
        return false;
    }
    if (withdrawal < 0.0) {
        err = "Monthly withdrawal cannot be negative.";
        return false;
    }
    if (rate < 0.0 || rate > 100.0) {
        err = "Expected return rate must be between 0% and 100%.";
        return false;
    }
    if (years < 1 || years > 50) {
        err = "Withdrawal duration must be between 1 and 50 years.";
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// SIP Calculation
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Standard SIP (end-of-month ordinary annuity):
 *   FV = P × [((1 + r)^n − 1) / r]
 *   where r = annualRate / 1200,  n = totalMonths
 *
 * Step-up SIP:
 *   For each year y (0-indexed), the monthly contribution is:
 *     P_y = P_start × (1 + stepUp/100)^y
 *   The future value accumulates each year's 12-month ordinary annuity
 *   and compounds the running corpus forward at the same monthly rate.
 *
 * Zero-rate fallback:
 *   FV = sum of all monthly contributions (no compounding).
 */
SipResult AnnuityEngine::calculateSip(double monthlyAmountStart,
                                      double annualReturnPercent,
                                      int    durationYears,
                                      double annualStepUpPercent)
{
    SipResult result;
    if (!validateSipInputs(monthlyAmountStart, annualReturnPercent, durationYears,
                           annualStepUpPercent, result.errorMessage)) {
        return result;
    }

    result.valid               = true;
    result.monthlyAmountInitial = monthlyAmountStart;
    result.annualStepUpPercent  = annualStepUpPercent;
    result.annualReturnPercent  = annualReturnPercent;
    result.durationYears        = durationYears;
    result.totalInstallments    = durationYears * 12;
    result.xirr                 = annualReturnPercent; // for standard SIP

    const double r = annualReturnPercent / 1200.0; // monthly rate as decimal
    const bool   zeroRate = (r < 1e-9);

    double runningCorpus    = 0.0;   // corpus value being built up
    double totalInvested    = 0.0;   // cumulative principal paid
    double currentMonthly   = monthlyAmountStart;

    for (int y = 1; y <= durationYears; ++y) {
        // Compound the existing corpus for 12 more months (no new contributions yet)
        double corpusStartOfYear = runningCorpus;
        double corpusGrowthFactor = zeroRate ? 1.0 : std::pow(1.0 + r, 12);
        double grownCorpus = corpusStartOfYear * corpusGrowthFactor;

        // Future value of THIS year's 12 monthly contributions (ordinary annuity)
        double thiYearFV = 0.0;
        if (zeroRate) {
            thiYearFV = currentMonthly * 12.0;
        } else {
            // Ordinary annuity: contributions at END of each month
            thiYearFV = currentMonthly * ((std::pow(1.0 + r, 12) - 1.0) / r);
        }

        runningCorpus  = grownCorpus + thiYearFV;
        totalInvested += currentMonthly * 12.0;

        SipYearlyProjection pt;
        pt.year                     = y;
        pt.investedAtYearEndCents   = static_cast<int64_t>(std::round(totalInvested * 100.0));
        pt.portfolioValueAtYearEndCents = static_cast<int64_t>(std::round(runningCorpus * 100.0));
        pt.wealthGainAtYearEndCents = pt.portfolioValueAtYearEndCents - pt.investedAtYearEndCents;
        pt.returnPercent = (totalInvested > 0.0)
            ? ((runningCorpus - totalInvested) / totalInvested * 100.0)
            : 0.0;
        result.yearlyBreakdown.push_back(pt);

        // Step-up: next year's monthly contribution
        if (annualStepUpPercent > 0.0) {
            currentMonthly *= (1.0 + annualStepUpPercent / 100.0);
        }
    }

    result.totalInvestedCents   = static_cast<int64_t>(std::round(totalInvested * 100.0));
    result.maturityValueCents   = static_cast<int64_t>(std::round(runningCorpus * 100.0));
    result.estimatedGainCents   = result.maturityValueCents - result.totalInvestedCents;
    result.returnPercent = (totalInvested > 0.0)
        ? ((runningCorpus - totalInvested) / totalInvested * 100.0)
        : 0.0;

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// SWP Calculation
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Calculation order per month: GROWTH FIRST, then WITHDRAWAL.
 *   balance = balance × (1 + monthlyRate)
 *   if balance >= withdrawal:
 *       balance -= withdrawal
 *       totalWithdrawn += withdrawal
 *   else:
 *       totalWithdrawn += balance   (partial last withdrawal)
 *       balance = 0                 (depleted)
 *
 * Monthly rate convention: nominal annual rate / 12
 *   monthlyRate = annualRate / 100 / 12
 *
 * Zero-rate: no growth applied before withdrawal.
 */
SwpResult AnnuityEngine::calculateSwp(double initialCorpus,
                                      double monthlyWithdrawal,
                                      double annualReturnPercent,
                                      int    durationYears)
{
    SwpResult result;
    if (!validateSwpInputs(initialCorpus, monthlyWithdrawal, annualReturnPercent, durationYears,
                           result.errorMessage)) {
        return result;
    }

    result.valid               = true;
    result.initialCorpus       = initialCorpus;
    result.monthlyWithdrawal   = monthlyWithdrawal;
    result.annualReturnPercent = annualReturnPercent;
    result.durationYears       = durationYears;
    result.initialCorpusCents  = static_cast<int64_t>(std::round(initialCorpus * 100.0));

    const double r         = annualReturnPercent / 1200.0;
    const bool   zeroRate  = (r < 1e-9);
    const int    totalMths = durationYears * 12;

    double balance       = initialCorpus;
    double totalWithdrawn = 0.0;
    double totalInterest  = 0.0;
    bool   depleted      = false;
    int    depletedMonth = 0;

    for (int y = 1; y <= durationYears; ++y) {
        SwpYearlyProjection yr;
        yr.year = y;
        yr.withdrawalMonthly = monthlyWithdrawal;

        int64_t yearWithdrawn = 0;

        for (int m = 1; m <= 12; ++m) {
            if (depleted) break;

            int globalMonth = (y - 1) * 12 + m;
            if (globalMonth > totalMths) break;

            // 1. Growth phase
            double interest = zeroRate ? 0.0 : balance * r;
            totalInterest += interest;
            balance += interest;

            // 2. Withdrawal phase
            if (monthlyWithdrawal <= 0.0) {
                // Zero withdrawal — just grow
            } else if (balance > monthlyWithdrawal + 1e-9) {
                balance -= monthlyWithdrawal;
                totalWithdrawn += monthlyWithdrawal;
                yearWithdrawn  += static_cast<int64_t>(std::round(monthlyWithdrawal * 100.0));
                result.withdrawalsCompleted++;
            } else if (std::abs(balance - monthlyWithdrawal) <= 1e-9) {
                // Exact withdrawal exhausts remaining corpus
                balance = 0.0;
                totalWithdrawn += monthlyWithdrawal;
                yearWithdrawn  += static_cast<int64_t>(std::round(monthlyWithdrawal * 100.0));
                result.withdrawalsCompleted++;
                if (globalMonth < totalMths) {
                    depleted      = true;
                    depletedMonth = globalMonth;
                    break;
                }
            } else if (balance > 1e-9) {
                // Partial last withdrawal — corpus runs out
                totalWithdrawn += balance;
                yearWithdrawn  += static_cast<int64_t>(std::round(balance * 100.0));
                result.withdrawalsCompleted++;
                depleted      = true;
                depletedMonth = globalMonth;
                balance       = 0.0;
                break;
            } else {
                // Balance was already 0
                depleted      = true;
                depletedMonth = globalMonth;
                break;
            }
        }

        yr.totalWithdrawnThisYearCents = yearWithdrawn;
        yr.balanceAtYearEndCents       = static_cast<int64_t>(std::round(balance * 100.0));
        yr.depletedThisYear            = depleted && (depletedMonth >= (y-1)*12+1) && (depletedMonth <= y*12);
        yr.depletedAtMonth             = yr.depletedThisYear ? depletedMonth : 0;
        result.yearlyBreakdown.push_back(yr);

        if (depleted) break;
    }

    result.totalWithdrawnCents  = static_cast<int64_t>(std::round(totalWithdrawn * 100.0));
    result.finalBalanceCents    = static_cast<int64_t>(std::round(balance * 100.0));
    result.totalEarningsCents   = static_cast<int64_t>(std::round(totalInterest * 100.0));
    result.depleted             = depleted;
    result.depletedAtMonth      = depletedMonth;
    result.depletedAtYear       = depleted ? ((depletedMonth + 11) / 12) : 0;

    return result;
}

} // namespace Finance
} // namespace InvestSphere

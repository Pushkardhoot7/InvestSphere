#ifndef INVESTSPHERE_COSTBASISENGINE_H
#define INVESTSPHERE_COSTBASISENGINE_H

#include "TransactionModel.h"
#include <vector>
#include <cstdint>
#include <string>

namespace InvestSphere {
namespace Finance {

/**
 * @brief Comprehensive results of Weighted Average Cost (WAC) calculations on an investment position.
 */
struct CostBasisResult {
    double remainingQuantity{0.0};
    int64_t totalInvestedCostCents{0};         // Remaining holdings' cost basis (Remaining Qty * WAC)
    int64_t averageCostPerUnitCents{0};        // Weighted average cost per unit in minor units
    int64_t totalRealizedPnlCents{0};          // Capital gains strictly from SELL/REDEMPTION (Proceeds - Cost Basis)
    int64_t totalRealizedProceedsCents{0};     // Net sale proceeds
    int64_t totalRealizedCostCents{0};         // Cost basis of units sold
    int64_t totalDividendsCents{0};            // Dividend income (kept separate from capital gains)
    int64_t totalInterestCents{0};             // Interest income (kept separate from capital gains)
    int64_t totalIncomeCents{0};               // Total non-capital income (Dividends + Interest)
    int64_t totalFeesPaidCents{0};             // Brokerage / transaction fees
    int64_t totalTaxesPaidCents{0};            // STT / transaction taxes
    int64_t cumulativePurchaseTurnoverCents{0}; // Cumulative historical gross purchase outlay
    int64_t netExternalCashFlowCents{0};       // Net capital deployed (Purchases - Sales - Income)
    int transactionCount{0};
};

/**
 * @brief Engine implementing the Weighted Average Cost (WAC) algorithm for multi-tranche
 * buy/sell transaction streams with exact minor-unit (cents) integer arithmetic.
 */
class CostBasisEngine {
public:
    /**
     * @brief Computes running cost basis, remaining units, and realized P&L across a series
     * of chronological transaction records for an investment.
     */
    static CostBasisResult calculateCostBasis(const std::vector<Models::Transaction> &transactions);

    /**
     * @brief Computes unrealized profit/loss given remaining quantity, cost basis, and current unit market price.
     * @param remainingQuantity Remaining units held
     * @param totalCostCents Total cost basis of remaining units (cents)
     * @param currentUnitPriceCents Latest observed price per unit (cents)
     * @param[out] outCurrentValueCents Estimated current market value (cents)
     * @param[out] outUnrealizedPnlCents Unrealized profit/loss (cents)
     * @param[out] outPnlPercent Return on investment percentage (e.g., +15.5%)
     * @return true if calculation was possible; false if inputs invalid (e.g. zero units or negative price)
     */
    static bool calculateUnrealizedPnl(double remainingQuantity,
                                      int64_t totalCostCents,
                                      int64_t currentUnitPriceCents,
                                      int64_t &outCurrentValueCents,
                                      int64_t &outUnrealizedPnlCents,
                                      double &outPnlPercent);
};

} // namespace Finance
} // namespace InvestSphere

#endif // INVESTSPHERE_COSTBASISENGINE_H

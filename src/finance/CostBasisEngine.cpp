#include "CostBasisEngine.h"
#include <algorithm>
#include <cmath>

namespace InvestSphere {
namespace Finance {

CostBasisResult CostBasisEngine::calculateCostBasis(const std::vector<Models::Transaction> &transactions)
{
    CostBasisResult result;
    if (transactions.empty()) {
        return result;
    }

    // Copy and sort chronologically by date and transaction ID
    auto sortedTx = transactions;
    std::sort(sortedTx.begin(), sortedTx.end(), [](const Models::Transaction &a, const Models::Transaction &b) {
        if (a.transactionDate != b.transactionDate) {
            return a.transactionDate < b.transactionDate;
        }
        return a.id < b.id;
    });

    double runningQty = 0.0;
    int64_t runningCostCents = 0;
    int64_t realizedPnlCents = 0;
    int64_t realizedProceedsCents = 0;
    int64_t realizedCostCents = 0;
    int64_t dividendsCents = 0;
    int64_t interestCents = 0;
    int64_t feesCents = 0;
    int64_t taxesCents = 0;
    int64_t cumulativeTurnoverCents = 0;
    int64_t netCashFlowCents = 0;

    for (const auto &tx : sortedTx) {
        feesCents += tx.feesCents;
        taxesCents += tx.taxesCents;

        if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
            double buyQty = tx.quantity;
            int64_t buyAmount = tx.netAmountCents;

            runningQty += buyQty;
            runningCostCents += buyAmount;
            cumulativeTurnoverCents += buyAmount;
            netCashFlowCents += buyAmount;
        } else if (tx.transactionType == "SELL" || tx.transactionType == "REDEMPTION") {
            double sellQty = tx.quantity;
            int64_t sellProceeds = tx.netAmountCents;

            if (runningQty > 0.0 && sellQty > 0.0) {
                // Calculate average cost per unit prior to sale
                double avgCost = static_cast<double>(runningCostCents) / runningQty;
                double portionSold = std::min(sellQty, runningQty);
                int64_t costOfUnitsSold = static_cast<int64_t>(std::round(portionSold * avgCost));

                // Realized P/L = Net sale proceeds - Cost basis of units sold
                int64_t gain = sellProceeds - costOfUnitsSold;
                realizedPnlCents += gain;
                realizedProceedsCents += sellProceeds;
                realizedCostCents += costOfUnitsSold;

                runningQty -= portionSold;
                runningCostCents -= costOfUnitsSold;
                if (runningQty <= 0.00001) {
                    runningQty = 0.0;
                    runningCostCents = 0;
                }
            } else {
                realizedProceedsCents += sellProceeds;
            }
            netCashFlowCents -= sellProceeds;
        } else if (tx.transactionType == "DIVIDEND") {
            dividendsCents += tx.netAmountCents;
            netCashFlowCents -= tx.netAmountCents;
        } else if (tx.transactionType == "INTEREST") {
            interestCents += tx.netAmountCents;
            netCashFlowCents -= tx.netAmountCents;
        } else if (tx.transactionType == "FEE") {
            feesCents += tx.netAmountCents;
            netCashFlowCents += tx.netAmountCents;
        }
    }

    result.remainingQuantity = runningQty;
    result.totalInvestedCostCents = runningCostCents;
    result.averageCostPerUnitCents = (runningQty > 0.00001)
        ? static_cast<int64_t>(std::round(static_cast<double>(runningCostCents) / runningQty))
        : 0;
    result.totalRealizedPnlCents = realizedPnlCents;
    result.totalRealizedProceedsCents = realizedProceedsCents;
    result.totalRealizedCostCents = realizedCostCents;
    result.totalDividendsCents = dividendsCents;
    result.totalInterestCents = interestCents;
    result.totalIncomeCents = dividendsCents + interestCents;
    result.totalFeesPaidCents = feesCents;
    result.totalTaxesPaidCents = taxesCents;
    result.cumulativePurchaseTurnoverCents = cumulativeTurnoverCents;
    result.netExternalCashFlowCents = netCashFlowCents;
    result.transactionCount = static_cast<int>(transactions.size());

    return result;
}

bool CostBasisEngine::calculateUnrealizedPnl(double remainingQuantity,
                                            int64_t totalCostCents,
                                            int64_t currentUnitPriceCents,
                                            int64_t &outCurrentValueCents,
                                            int64_t &outUnrealizedPnlCents,
                                            double &outPnlPercent)
{
    if (remainingQuantity <= 0.00001 || currentUnitPriceCents < 0) {
        outCurrentValueCents = 0;
        outUnrealizedPnlCents = 0;
        outPnlPercent = 0.0;
        return false;
    }

    // Market Value = Remaining Quantity * Current Price
    outCurrentValueCents = static_cast<int64_t>(std::round(remainingQuantity * currentUnitPriceCents));
    
    // Unrealized P&L = Current Value - Cost Basis
    outUnrealizedPnlCents = outCurrentValueCents - totalCostCents;
    
    // Return % = (Unrealized P&L / Cost Basis) * 100
    if (totalCostCents > 0) {
        outPnlPercent = (static_cast<double>(outUnrealizedPnlCents) / totalCostCents) * 100.0;
    } else {
        outPnlPercent = 0.0;
    }

    return true;
}

} // namespace Finance
} // namespace InvestSphere

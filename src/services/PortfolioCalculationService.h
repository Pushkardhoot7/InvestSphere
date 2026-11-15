#ifndef INVESTSPHERE_PORTFOLIOCALCULATIONSERVICE_H
#define INVESTSPHERE_PORTFOLIOCALCULATIONSERVICE_H

#include "TransactionModel.h"
#include "InvestmentModel.h"
#include "CostBasisEngine.h"
#include "Compat.h"

#include <vector>
#include <string>
#include <cstdint>

namespace InvestSphere {
namespace Services {

struct HoldingPositionDTO {
    int investmentId{0};
    int instrumentId{0};
    std::string symbol;
    std::string name;
    std::string categoryCode;
    std::string categoryName;
    std::string status{"ACTIVE"};
    std::string portfolioTag{"DEFAULT"};

    double remainingQuantity{0.0};
    int64_t costBasisCents{0};            // Remaining Qty * WAC
    int64_t averageCostPerUnitCents{0};   // WAC
    
    // Valuation fields (strict non-fabrication)
    bool hasMarketQuote{false};
    int64_t currentUnitPriceCents{0};
    bool hasMarketValue{false};
    int64_t marketValueCents{0};
    bool hasPnl{false};
    int64_t unrealizedPnlCents{0};
    double unrealizedPnlPercent{0.0};

    // Performance & income
    int64_t realizedPnlCents{0};         // Realized capital gains from sales
    int64_t dividendsCents{0};           // Dividend income
    int64_t interestCents{0};            // Interest income
    int64_t totalIncomeCents{0};         // Dividends + Interest
    int64_t feesCents{0};
    int64_t taxesCents{0};
    int transactionCount{0};
};

struct PortfolioSummaryDTO {
    int activeHoldingsCount{0};
    int quotedHoldingsCount{0};
    int unquotedHoldingsCount{0};
    double valuationCoveragePercent{0.0};
    bool hasValuationData{false};
    bool isValuationComplete{false};

    int64_t totalInvestedCostBasisCents{0};     // Cost basis of remaining holdings
    int64_t totalCurrentEstimatedValueCents{0}; // Current value of quoted holdings
    int64_t totalUnrealizedPnlCents{0};         // Estimated unrealized P&L
    double unrealizedPnlPercent{0.0};           // Simple return %
    
    int64_t totalRealizedPnlCents{0};           // Capital gains from completed sales
    int64_t totalDividendsCents{0};             // Dividend income
    int64_t totalInterestCents{0};              // Interest income
    int64_t totalIncomeCents{0};                // Total dividend + interest income
    int64_t totalFeesPaidCents{0};              // Total brokerage/fees
    int64_t totalTaxesPaidCents{0};             // Total taxes paid
    int64_t cumulativePurchaseTurnoverCents{0};  // Total purchase capital deployed all-time
    int64_t netExternalCashFlowCents{0};        // Net external capital outlay
};

/**
 * @brief Centralized financial calculation service providing single authoritative
 * source of truth for holdings, cost basis (WAC), P&L, and portfolio summaries.
 */
class PortfolioCalculationService {
public:
    static PortfolioCalculationService& instance();

    /**
     * @brief Consolidated portfolio summary metrics for the user.
     */
    PortfolioSummaryDTO calculatePortfolioSummary(int userId);

    /**
     * @brief Detailed position calculation for all holdings of the user.
     */
    std::vector<HoldingPositionDTO> calculateHoldingsPositions(int userId);

    /**
     * @brief Calculates a single position's cost basis and valuation.
     */
    InvestSphere::Optional<HoldingPositionDTO> calculateHoldingPosition(int userId, int investmentId);

    /**
     * @brief Validates if a proposed transaction (or edit) maintains non-negative quantities
     * across historical and current timeline.
     * @return pair of (isFeasible, errorMessageIfInfeasible)
     */
    std::pair<bool, std::string> validateHoldingQuantityFeasibility(int userId,
                                                                  int investmentId,
                                                                  const std::string &proposedType,
                                                                  double proposedQty,
                                                                  const std::string &proposedDate,
                                                                  int ignoreTxId = 0);
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_PORTFOLIOCALCULATIONSERVICE_H

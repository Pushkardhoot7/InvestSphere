#ifndef INVESTSPHERE_ANALYTICSSERVICE_H
#define INVESTSPHERE_ANALYTICSSERVICE_H

#include "PortfolioCalculationService.h"
#include "PortfolioSnapshotRepository.h"
#include "Compat.h"

#include <vector>
#include <string>
#include <cstdint>

namespace InvestSphere {
namespace Services {

/**
 * @brief Category-level analytics aggregation.
 */
struct CategoryAnalyticsDTO {
    std::string categoryCode;
    std::string categoryName;
    int         holdingsCount{0};
    int64_t     costBasisCents{0};
    bool        hasValuation{false};
    int64_t     estimatedValueCents{0};
    int64_t     unrealizedPnlCents{0};
    double      unrealizedPnlPercent{0.0};
    int64_t     realizedPnlCents{0};
    int64_t     incomeCents{0};
    double      allocationPercent{0.0}; ///< % of total valued portfolio
};

/**
 * @brief A single position entry for best/worst performer ranking.
 * Only populated when both cost basis and valuation are available.
 */
struct PerformerDTO {
    int         investmentId{0};
    std::string symbol;
    std::string name;
    std::string categoryCode;
    int64_t     costBasisCents{0};
    int64_t     marketValueCents{0};
    int64_t     unrealizedPnlCents{0};
    double      unrealizedPnlPercent{0.0};
};

/**
 * @brief Full analytics result for a user's portfolio.
 */
struct PortfolioAnalyticsDTO {
    // ── Portfolio Overview ──────────────────────────────────────────────────
    PortfolioSummaryDTO summary;

    // ── Category Breakdown ─────────────────────────────────────────────────
    std::vector<CategoryAnalyticsDTO> categories;

    // ── Performance History (from snapshots) ───────────────────────────────
    std::vector<Database::PortfolioSnapshot> performanceHistory;
    bool hasPerformanceHistory{false};
    std::string performanceHistoryNote;

    // ── Top/Bottom Performers (quoted holdings only) ────────────────────────
    std::vector<PerformerDTO> topPerformers;    ///< Best unrealized return %
    std::vector<PerformerDTO> worstPerformers;  ///< Worst unrealized return %

    // ── Metadata ────────────────────────────────────────────────────────────
    std::string generatedAt;
    std::string filterCategoryCode;  ///< "" = all categories
    std::string filterStartDate;
    std::string filterEndDate;
    bool isEmptyPortfolio{false};
};

/**
 * @brief Centralized analytics aggregation service.
 * Delegates all financial calculations to PortfolioCalculationService and
 * PortfolioSnapshotRepository — does not duplicate math formulas.
 */
class AnalyticsService {
public:
    static AnalyticsService &instance();

    /**
     * @brief Compute full analytics for a user.
     * @param userId         Authenticated user.
     * @param filterCategory Optional asset class filter ("" = all).
     * @param startDate      Optional date range lower bound "YYYY-MM-DD".
     * @param endDate        Optional date range upper bound "YYYY-MM-DD".
     * @param maxPerformers  Maximum entries in top/worst performer lists.
     */
    PortfolioAnalyticsDTO computeAnalytics(int userId,
                                           const std::string &filterCategory = "",
                                           const std::string &startDate = "",
                                           const std::string &endDate   = "",
                                           int maxPerformers = 5);

    /**
     * @brief Record a portfolio snapshot for the authenticated user.
     * Should be called after a transaction or investment is saved, if the
     * portfolio has at least one quoted holding.
     * @return true if snapshot was recorded or updated.
     */
    bool recordSnapshotIfValuationAvailable(int userId);

    /**
     * @brief Retrieve snapshot history for a user within an optional date range.
     */
    std::vector<Database::PortfolioSnapshot> getSnapshotHistory(int userId,
                                                                const std::string &startDate = "",
                                                                const std::string &endDate   = "");

private:
    AnalyticsService() = default;
    AnalyticsService(const AnalyticsService &) = delete;
    AnalyticsService &operator=(const AnalyticsService &) = delete;

    std::vector<CategoryAnalyticsDTO> buildCategoryBreakdown(
        const std::vector<HoldingPositionDTO> &positions,
        const std::string &filterCategory);

    std::pair<std::vector<PerformerDTO>, std::vector<PerformerDTO>>
    buildPerformerLists(const std::vector<HoldingPositionDTO> &positions, int maxN);
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_ANALYTICSSERVICE_H

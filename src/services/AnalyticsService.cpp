#include "AnalyticsService.h"
#include "PortfolioCalculationService.h"
#include "PortfolioSnapshotRepository.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <ctime>
#include <cmath>

namespace InvestSphere {
namespace Services {

// ─────────────────────────────────────────────────────────────────────────────
// Singleton
// ─────────────────────────────────────────────────────────────────────────────

AnalyticsService &AnalyticsService::instance()
{
    static AnalyticsService sInstance;
    return sInstance;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static std::string currentTimestamp()
{
    std::time_t t = std::time(nullptr);
    char buf[24] = {};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}


// ─────────────────────────────────────────────────────────────────────────────
// computeAnalytics
// ─────────────────────────────────────────────────────────────────────────────

PortfolioAnalyticsDTO AnalyticsService::computeAnalytics(int userId,
                                                          const std::string &filterCategory,
                                                          const std::string &startDate,
                                                          const std::string &endDate,
                                                          int maxPerformers)
{
    PortfolioAnalyticsDTO result;
    result.generatedAt        = currentTimestamp();
    result.filterCategoryCode = filterCategory;
    result.filterStartDate    = startDate;
    result.filterEndDate      = endDate;

    // ── 1. Portfolio summary (delegates to PortfolioCalculationService) ───────
    auto &calcSvc = PortfolioCalculationService::instance();
    result.summary = calcSvc.calculatePortfolioSummary(userId);

    if (result.summary.activeHoldingsCount == 0) {
        result.isEmptyPortfolio = true;
        result.performanceHistoryNote = "No active holdings. Add investments to see analytics.";
        return result;
    }

    // ── 2. Per-position breakdown ─────────────────────────────────────────────
    auto positions = calcSvc.calculateHoldingsPositions(userId);

    // ── 3. Category breakdown ─────────────────────────────────────────────────
    result.categories = buildCategoryBreakdown(positions, filterCategory);

    // ── 4. Performer lists ────────────────────────────────────────────────────
    auto performers = buildPerformerLists(positions, maxPerformers);
    result.topPerformers   = performers.first;
    result.worstPerformers = performers.second;

    // ── 5. Performance history (snapshot-based) ───────────────────────────────
    Database::PortfolioSnapshotRepository snapRepo;
    result.performanceHistory = snapRepo.getSnapshots(userId, startDate, endDate, 0);
    result.hasPerformanceHistory = !result.performanceHistory.empty();

    if (!result.hasPerformanceHistory) {
        result.performanceHistoryNote =
            "No historical snapshots available for the selected period. "
            "Snapshots are recorded automatically when you save transactions or investments "
            "and your portfolio has at least one quoted holding.";
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildCategoryBreakdown
// ─────────────────────────────────────────────────────────────────────────────

std::vector<CategoryAnalyticsDTO> AnalyticsService::buildCategoryBreakdown(
    const std::vector<HoldingPositionDTO> &positions,
    const std::string &filterCategory)
{
    // Aggregate positions by category
    std::map<std::string, CategoryAnalyticsDTO> cats;

    for (const auto &pos : positions) {
        if (!filterCategory.empty() && pos.categoryCode != filterCategory) continue;

        auto &cat = cats[pos.categoryCode];
        cat.categoryCode = pos.categoryCode;
        cat.categoryName = pos.categoryName;
        cat.holdingsCount++;
        cat.costBasisCents     += pos.costBasisCents;
        cat.realizedPnlCents   += pos.realizedPnlCents;
        cat.incomeCents        += pos.totalIncomeCents;

        if (pos.hasMarketValue) {
            cat.hasValuation = true;
            cat.estimatedValueCents += pos.marketValueCents;
            cat.unrealizedPnlCents  += pos.unrealizedPnlCents;
        }
    }

    // Compute unrealized % and allocation % per category
    // Total valued portfolio is sum of all estimated values (quoted holdings only)
    int64_t totalValuedCents = 0;
    for (const auto &pair : cats) {
        const auto &cat = pair.second;
        if (cat.hasValuation) totalValuedCents += cat.estimatedValueCents;
    }

    std::vector<CategoryAnalyticsDTO> result;
    for (const auto &pair : cats) {
        CategoryAnalyticsDTO cat = pair.second;
        if (cat.costBasisCents > 0) {
            cat.unrealizedPnlPercent = cat.hasValuation
                ? (static_cast<double>(cat.unrealizedPnlCents) /
                   static_cast<double>(cat.costBasisCents) * 100.0)
                : 0.0;
        }
        cat.allocationPercent = (totalValuedCents > 0 && cat.hasValuation)
            ? (static_cast<double>(cat.estimatedValueCents) /
               static_cast<double>(totalValuedCents) * 100.0)
            : 0.0;
        result.push_back(cat);
    }

    // Sort: valued categories first, then by estimated value descending
    std::sort(result.begin(), result.end(),
              [](const CategoryAnalyticsDTO &a, const CategoryAnalyticsDTO &b) {
                  if (a.hasValuation != b.hasValuation) return a.hasValuation > b.hasValuation;
                  return a.estimatedValueCents > b.estimatedValueCents;
              });

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildPerformerLists
// ─────────────────────────────────────────────────────────────────────────────

std::pair<std::vector<PerformerDTO>, std::vector<PerformerDTO>>
AnalyticsService::buildPerformerLists(const std::vector<HoldingPositionDTO> &positions, int maxN)
{
    std::vector<PerformerDTO> quotedPositions;
    for (const auto &pos : positions) {
        if (!pos.hasMarketValue || pos.costBasisCents <= 0) continue;
        PerformerDTO p;
        p.investmentId         = pos.investmentId;
        p.symbol               = pos.symbol;
        p.name                 = pos.name;
        p.categoryCode         = pos.categoryCode;
        p.costBasisCents       = pos.costBasisCents;
        p.marketValueCents     = pos.marketValueCents;
        p.unrealizedPnlCents   = pos.unrealizedPnlCents;
        p.unrealizedPnlPercent = pos.unrealizedPnlPercent;
        quotedPositions.push_back(p);
    }

    if (quotedPositions.empty()) return {{}, {}};

    // Sort by return % descending for top performers
    std::vector<PerformerDTO> sorted = quotedPositions;
    std::sort(sorted.begin(), sorted.end(), [](const PerformerDTO &a, const PerformerDTO &b) {
        return a.unrealizedPnlPercent > b.unrealizedPnlPercent;
    });

    std::vector<PerformerDTO> top, worst;
    for (int i = 0; i < static_cast<int>(sorted.size()) && i < maxN; ++i) {
        top.push_back(sorted[i]);
    }
    for (int i = static_cast<int>(sorted.size()) - 1;
         i >= 0 && static_cast<int>(worst.size()) < maxN; --i) {
        // Avoid including same item in both lists if portfolio is very small
        if (top.size() < sorted.size() || sorted.size() <= static_cast<size_t>(maxN)) {
            worst.push_back(sorted[i]);
        }
    }
    // Worst list is already ordered worst-first (ascending) by walking backwards from sorted
    return {top, worst};
}

// ─────────────────────────────────────────────────────────────────────────────
// recordSnapshotIfValuationAvailable
// ─────────────────────────────────────────────────────────────────────────────

bool AnalyticsService::recordSnapshotIfValuationAvailable(int userId)
{
    auto &calcSvc = PortfolioCalculationService::instance();
    auto summary = calcSvc.calculatePortfolioSummary(userId);

    // Only record if at least one holding has a real market quote
    if (summary.quotedHoldingsCount == 0) return false;

    Database::PortfolioSnapshot snap;
    snap.userId              = userId;
    snap.snapshotTimestamp   = currentTimestamp();
    snap.totalInvestedCents  = summary.totalInvestedCostBasisCents;
    snap.estimatedValueCents = summary.totalCurrentEstimatedValueCents;
    snap.realizedPnlCents    = summary.totalRealizedPnlCents;
    snap.unrealizedPnlCents  = summary.totalUnrealizedPnlCents;

    Database::PortfolioSnapshotRepository repo;
    auto idOpt = repo.recordSnapshot(snap);
    return static_cast<bool>(idOpt);
}

// ─────────────────────────────────────────────────────────────────────────────
// getSnapshotHistory
// ─────────────────────────────────────────────────────────────────────────────

std::vector<Database::PortfolioSnapshot> AnalyticsService::getSnapshotHistory(
    int userId, const std::string &startDate, const std::string &endDate)
{
    Database::PortfolioSnapshotRepository repo;
    return repo.getSnapshots(userId, startDate, endDate, 0);
}

} // namespace Services
} // namespace InvestSphere

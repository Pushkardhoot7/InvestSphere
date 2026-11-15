#ifndef INVESTSPHERE_PORTFOLIOSNAPSHOTREPOSITORY_H
#define INVESTSPHERE_PORTFOLIOSNAPSHOTREPOSITORY_H

#include "Compat.h"
#include <string>
#include <vector>
#include <cstdint>

namespace InvestSphere {
namespace Database {

/**
 * @brief A single portfolio valuation snapshot for a user.
 */
struct PortfolioSnapshot {
    int     snapshotId{0};
    int     userId{0};
    std::string snapshotTimestamp;     ///< ISO-8601 datetime
    int64_t totalInvestedCents{0};     ///< Cost basis of active holdings
    int64_t estimatedValueCents{0};    ///< Current estimated market value
    int64_t realizedPnlCents{0};       ///< Realized P&L at snapshot time
    int64_t unrealizedPnlCents{0};     ///< Unrealized P&L at snapshot time
    std::string notes;
};

/**
 * @brief Data access layer for the portfolio_snapshots table.
 * Snapshots are only written when valuation data is available (hasMarketQuote > 0).
 * One snapshot per user per calendar day is the recommended frequency.
 */
class PortfolioSnapshotRepository {
public:
    PortfolioSnapshotRepository() = default;

    /**
     * @brief Record a portfolio snapshot for the user.
     * Silently de-duplicates: if a snapshot already exists for the same user
     * and same calendar day, the existing row is updated (not inserted again).
     * @return snapshot_id on success, empty on failure.
     */
    InvestSphere::Optional<int> recordSnapshot(const PortfolioSnapshot &snapshot);

    /**
     * @brief Retrieve snapshots for a user, ordered ascending by timestamp.
     * @param userId      Target user.
     * @param startDate   Optional ISO date lower bound "YYYY-MM-DD" (inclusive).
     * @param endDate     Optional ISO date upper bound "YYYY-MM-DD" (inclusive).
     * @param limitRows   Maximum rows to return (0 = unlimited).
     */
    std::vector<PortfolioSnapshot> getSnapshots(int userId,
                                                const std::string &startDate = "",
                                                const std::string &endDate   = "",
                                                int limitRows = 0);

    /**
     * @brief Return the most recent snapshot for the user.
     */
    InvestSphere::Optional<PortfolioSnapshot> getLatestSnapshot(int userId);

    /**
     * @brief Count snapshots available for user within date range.
     */
    int countSnapshots(int userId,
                       const std::string &startDate = "",
                       const std::string &endDate   = "");

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // INVESTSPHERE_PORTFOLIOSNAPSHOTREPOSITORY_H

#ifndef INVESTSPHERE_DASHBOARDSERVICE_H
#define INVESTSPHERE_DASHBOARDSERVICE_H

#include "InvestmentService.h"
#include <string>
#include <vector>
#include <cstdint>

namespace InvestSphere {
namespace Services {

struct DashboardSummaryDTO {
    int64_t totalInvestedCents{0};
    int64_t currentEstimatedValueCents{0};
    bool hasValuationData{false};
    int64_t unrealizedPnlCents{0};
    double unrealizedPnlPercent{0.0};
    int64_t realizedPnlCents{0};
    int activeHoldingsCount{0};
    int unquotedHoldingsCount{0};
};

struct AssetAllocationItemDTO {
    std::string categoryCode;
    std::string categoryName;
    int64_t investedAmountCents{0};
    int64_t currentValueCents{0};
    double percentage{0.0};
    int holdingsCount{0};
    std::string colorHex;
};

struct RecentTransactionDTO {
    int transactionId{0};
    std::string date;
    std::string symbol;
    std::string instrumentName;
    std::string categoryCode;
    std::string transactionType; // 'BUY', 'SELL', 'DEPOSIT', 'DIVIDEND', etc.
    double quantity{0.0};
    int64_t unitPriceCents{0};
    int64_t netAmountCents{0};
    std::string notes;
};

struct UpcomingEventDTO {
    int eventId{0};
    std::string eventDate;
    std::string eventType;      // 'FD_MATURITY', 'BOND_COUPON', 'SIP_INSTALLMENT'
    std::string typeDisplay;
    std::string description;
    int64_t amountCents{0};
    std::string status;
};

struct TopHoldingDTO {
    int investmentId{0};
    std::string symbol;
    std::string name;
    std::string categoryCode;
    std::string categoryName;
    double quantity{0.0};
    int64_t investedCostCents{0};
    bool hasValuation{false};
    int64_t currentPriceCents{0};
    int64_t currentValueCents{0};
    bool hasPnl{false};
    int64_t unrealizedPnlCents{0};
    double unrealizedPnlPercent{0.0};
};

struct DashboardDataDTO {
    DashboardSummaryDTO summary;
    std::vector<AssetAllocationItemDTO> allocation;
    std::vector<RecentTransactionDTO> recentTransactions;
    std::vector<UpcomingEventDTO> upcomingEvents;
    std::vector<TopHoldingDTO> topHoldings;
};

/**
 * @brief Executive financial dashboard service calculating portfolio totals,
 * asset allocation breakdowns, recent ledger events, and top holdings from SQLite.
 */
class DashboardService {
public:
    static DashboardService& instance();

    /**
     * @brief Computes consolidated dashboard data for the authenticated user.
     */
    DashboardDataDTO getDashboardData(int userId);

    /**
     * @brief Calculates category asset allocation items.
     */
    std::vector<AssetAllocationItemDTO> getAssetAllocation(int userId);

    /**
     * @brief Retrieves latest ledger transactions for dashboard activity table.
     */
    std::vector<RecentTransactionDTO> getRecentTransactions(int userId, int limit = 6);

    /**
     * @brief Retrieves upcoming financial events and maturities for dashboard calendar.
     */
    std::vector<UpcomingEventDTO> getUpcomingEvents(int userId, int limit = 6);

    /**
     * @brief Retrieves top holdings ranked by estimated value or invested capital.
     */
    std::vector<TopHoldingDTO> getTopHoldings(int userId, int limit = 5);

private:
    DashboardService() = default;
    ~DashboardService() = default;
    DashboardService(const DashboardService&) = delete;
    DashboardService& operator=(const DashboardService&) = delete;
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_DASHBOARDSERVICE_H

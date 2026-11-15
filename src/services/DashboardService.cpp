#include "DashboardService.h"
#include "InvestmentService.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"
#include "FinancialEventRepository.h"
#include "CostBasisEngine.h"

#include <algorithm>
#include <map>
#include <cmath>

namespace InvestSphere {
namespace Services {

DashboardService& DashboardService::instance()
{
    static DashboardService s_instance;
    return s_instance;
}

DashboardDataDTO DashboardService::getDashboardData(int userId)
{
    DashboardDataDTO data;

    // 1. Get active investments
    auto holdings = InvestmentService::instance().getUserInvestments(userId, "", "ALL", "ACTIVE", "VALUE", false);

    int64_t totalInvested = 0;
    int64_t totalCurrentValue = 0;
    int64_t valuedHoldingsCost = 0;
    int64_t totalUnrealizedPnl = 0;
    int valuedCount = 0;
    int unquotedCount = 0;

    for (const auto &h : holdings) {
        totalInvested += h.investedAmountCents;
        if (h.hasCurrentValue) {
            totalCurrentValue += h.currentValueCents;
            valuedHoldingsCost += h.investedAmountCents;
            totalUnrealizedPnl += h.unrealizedPnlCents;
            valuedCount++;
        } else {
            unquotedCount++;
        }
    }

    // 2. Realized P&L from transactions
    Database::TransactionRepository txnRepo;
    auto userTxs = txnRepo.getTransactionsByUserId(userId);
    auto allTxsCostBasis = Finance::CostBasisEngine::calculateCostBasis(userTxs);
    int64_t realizedPnl = allTxsCostBasis.totalRealizedPnlCents;

    data.summary.totalInvestedCents = totalInvested;
    data.summary.currentEstimatedValueCents = totalCurrentValue;
    data.summary.hasValuationData = (valuedCount > 0);
    data.summary.unrealizedPnlCents = totalUnrealizedPnl;
    if (valuedHoldingsCost > 0) {
        data.summary.unrealizedPnlPercent = (static_cast<double>(totalUnrealizedPnl) / static_cast<double>(valuedHoldingsCost)) * 100.0;
    } else {
        data.summary.unrealizedPnlPercent = 0.0;
    }
    data.summary.realizedPnlCents = realizedPnl;
    data.summary.activeHoldingsCount = static_cast<int>(holdings.size());
    data.summary.unquotedHoldingsCount = unquotedCount;

    // 3. Asset Allocation
    data.allocation = getAssetAllocation(userId);

    // 4. Recent Transactions
    data.recentTransactions = getRecentTransactions(userId, 6);

    // 5. Upcoming Financial Events
    data.upcomingEvents = getUpcomingEvents(userId, 6);

    // 6. Top Holdings
    data.topHoldings = getTopHoldings(userId, 5);

    return data;
}

std::vector<AssetAllocationItemDTO> DashboardService::getAssetAllocation(int userId)
{
    auto holdings = InvestmentService::instance().getUserInvestments(userId, "", "ALL", "ACTIVE");
    std::vector<AssetAllocationItemDTO> allocation;

    struct CategoryBucket {
        std::string code;
        std::string name;
        std::string color;
        int64_t invested{0};
        int64_t value{0};
        int count{0};
    };

    std::map<std::string, CategoryBucket> buckets = {
        {"STOCK", {"STOCK", "Stocks", "#2563EB", 0, 0, 0}},
        {"MUTUAL_FUND", {"MUTUAL_FUND", "Mutual Funds", "#7C3AED", 0, 0, 0}},
        {"GOLD", {"GOLD", "Gold", "#F59E0B", 0, 0, 0}},
        {"FIXED_DEPOSIT", {"FIXED_DEPOSIT", "Fixed Deposits", "#10B981", 0, 0, 0}},
        {"BOND", {"BOND", "Bonds", "#06B6D4", 0, 0, 0}}
    };

    int64_t grandTotalValued = 0;
    int64_t grandTotalInvested = 0;

    for (const auto &h : holdings) {
        auto it = buckets.find(h.categoryCode);
        if (it != buckets.end()) {
            it->second.invested += h.investedAmountCents;
            int64_t val = h.hasCurrentValue ? h.currentValueCents : h.investedAmountCents;
            it->second.value += val;
            it->second.count++;

            grandTotalValued += val;
            grandTotalInvested += h.investedAmountCents;
        }
    }

    int64_t baseDenominator = (grandTotalValued > 0) ? grandTotalValued : grandTotalInvested;

    for (const auto &pair : buckets) {
        if (pair.second.count > 0 || pair.second.invested > 0) {
            AssetAllocationItemDTO item;
            item.categoryCode = pair.second.code;
            item.categoryName = pair.second.name;
            item.colorHex = pair.second.color;
            item.investedAmountCents = pair.second.invested;
            item.currentValueCents = pair.second.value;
            item.holdingsCount = pair.second.count;

            if (baseDenominator > 0) {
                item.percentage = (static_cast<double>(pair.second.value) / static_cast<double>(baseDenominator)) * 100.0;
            } else {
                item.percentage = 0.0;
            }
            allocation.push_back(item);
        }
    }

    // Sort descending by value / percentage
    std::sort(allocation.begin(), allocation.end(), [](const AssetAllocationItemDTO &a, const AssetAllocationItemDTO &b) {
        return a.currentValueCents > b.currentValueCents;
    });

    return allocation;
}

std::vector<RecentTransactionDTO> DashboardService::getRecentTransactions(int userId, int limit)
{
    Database::TransactionRepository txnRepo;
    Database::InvestmentRepository invRepo;

    auto allTxs = txnRepo.getTransactionsByUserId(userId);
    // Sort descending by date
    std::sort(allTxs.begin(), allTxs.end(), [](const Models::Transaction &a, const Models::Transaction &b) {
        if (a.transactionDate != b.transactionDate) {
            return a.transactionDate > b.transactionDate;
        }
        return a.id > b.id;
    });

    std::vector<RecentTransactionDTO> results;
    int count = 0;
    for (const auto &tx : allTxs) {
        if (count >= limit) break;

        RecentTransactionDTO dto;
        dto.transactionId = tx.id;
        dto.date = tx.transactionDate.length() >= 10 ? tx.transactionDate.substr(0, 10) : tx.transactionDate;
        dto.transactionType = tx.transactionType;
        dto.quantity = tx.quantity;
        dto.unitPriceCents = tx.unitPriceCents;
        dto.netAmountCents = tx.netAmountCents;
        dto.notes = tx.notes;

        if (tx.investmentId > 0) {
            auto inv = invRepo.getInvestmentById(tx.investmentId);
            if (inv) {
                dto.symbol = inv->symbol;
                dto.instrumentName = inv->instrumentName;
                dto.categoryCode = inv->assetTypeCode;
            } else {
                dto.symbol = "TX-" + std::to_string(tx.id);
                dto.instrumentName = "General Transaction";
                dto.categoryCode = "CASH";
            }
        } else {
            dto.symbol = "CASH";
            dto.instrumentName = "Direct Cash Flow";
            dto.categoryCode = "CASH";
        }

        results.push_back(dto);
        count++;
    }

    return results;
}

std::vector<UpcomingEventDTO> DashboardService::getUpcomingEvents(int userId, int limit)
{
    Database::FinancialEventRepository eventRepo;
    auto events = eventRepo.getUpcomingEvents(userId, 90);

    std::vector<UpcomingEventDTO> results;
    int count = 0;
    for (const auto &ev : events) {
        if (count >= limit) break;

        UpcomingEventDTO dto;
        dto.eventId = ev.id;
        dto.eventDate = ev.eventDate.length() >= 10 ? ev.eventDate.substr(0, 10) : ev.eventDate;
        dto.eventType = ev.eventType;
        dto.amountCents = ev.amountCents ? *ev.amountCents : 0;
        dto.status = ev.status;
        dto.description = ev.description;

        if (ev.eventType == "FD_MATURITY") {
            dto.typeDisplay = "FD Maturity";
        } else if (ev.eventType == "BOND_MATURITY") {
            dto.typeDisplay = "Bond Maturity";
        } else if (ev.eventType == "BOND_COUPON") {
            dto.typeDisplay = "Bond Coupon";
        } else if (ev.eventType == "SIP_INSTALLMENT") {
            dto.typeDisplay = "SIP Debit";
        } else if (ev.eventType == "SWP_WITHDRAWAL") {
            dto.typeDisplay = "SWP Outflow";
        } else if (ev.eventType == "DIVIDEND_EXPECTED") {
            dto.typeDisplay = "Dividend";
        } else {
            dto.typeDisplay = ev.eventType;
        }

        results.push_back(dto);
        count++;
    }

    return results;
}

std::vector<TopHoldingDTO> DashboardService::getTopHoldings(int userId, int limit)
{
    auto holdings = InvestmentService::instance().getUserInvestments(userId, "", "ALL", "ACTIVE", "VALUE", false);
    std::vector<TopHoldingDTO> results;

    int count = 0;
    for (const auto &h : holdings) {
        if (count >= limit) break;

        TopHoldingDTO dto;
        dto.investmentId = h.investmentId;
        dto.symbol = h.symbol;
        dto.name = h.name;
        dto.categoryCode = h.categoryCode;
        dto.categoryName = h.categoryName;
        dto.quantity = h.quantity;
        dto.investedCostCents = h.investedAmountCents;
        dto.hasValuation = h.hasCurrentValue;
        dto.currentPriceCents = h.latestPriceCents;
        dto.currentValueCents = h.hasCurrentValue ? h.currentValueCents : 0;
        dto.hasPnl = h.hasPnl;
        dto.unrealizedPnlCents = h.unrealizedPnlCents;
        dto.unrealizedPnlPercent = h.unrealizedPnlPercent;

        results.push_back(dto);
        count++;
    }

    return results;
}

} // namespace Services
} // namespace InvestSphere

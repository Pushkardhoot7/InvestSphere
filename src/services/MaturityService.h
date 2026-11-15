#ifndef INVESTSPHERE_MATURITYSERVICE_H
#define INVESTSPHERE_MATURITYSERVICE_H

#include "InvestmentModel.h"
#include "FinancialEventModel.h"
#include "FinancialEventRepository.h"
#include "InvestmentRepository.h"
#include "Compat.h"

#include <string>
#include <vector>
#include <cstdint>

namespace InvestSphere {
namespace Services {

struct MaturityItemDTO {
    int         investmentId{0};
    std::string assetType; // "FIXED_DEPOSIT" or "BOND"
    std::string title;
    std::string institutionOrIssuer;
    std::string accountOrRating;
    int64_t     principalOrFaceValueCents{0};
    double      quantity{1.0};
    int64_t     totalPrincipalCents{0};
    double      ratePercent{0.0};
    std::string frequency;
    std::string payoutType;
    std::string startDate;
    std::string maturityDate;
    int         daysRemaining{0};
    int64_t     estimatedProceedsCents{0};
    bool        hasEstimatedProceeds{true};
    std::string status; // ACTIVE, MATURED, PREMATURE_WITHDRAWN, REDEEMED
    std::string windowBadge; // "7_DAYS", "30_DAYS", "90_DAYS", "FUTURE", "MATURED"
};

struct MaturitySummaryDTO {
    int     in7DaysCount{0};
    int64_t in7DaysAmountCents{0};
    int     in30DaysCount{0};
    int64_t in30DaysAmountCents{0};
    int     in90DaysCount{0};
    int64_t in90DaysAmountCents{0};
    int     totalFutureCount{0};
    int64_t totalFutureAmountCents{0};
    int     maturedCount{0};
    int64_t maturedAmountCents{0};
};

class MaturityService {
public:
    static MaturityService& instance();

    MaturitySummaryDTO getMaturitySummary(int userId, const std::string &currentDate = "");

    std::vector<MaturityItemDTO> getMaturitySchedule(
        int userId,
        const std::string &windowFilter = "ALL_FUTURE", // "7_DAYS", "30_DAYS", "90_DAYS", "ALL_FUTURE", "MATURED", "ALL"
        const std::string &assetClassFilter = "ALL",     // "ALL", "FIXED_DEPOSIT", "BOND"
        const std::string &searchQuery = "",
        const std::string &currentDate = ""
    );

    // Idempotent financial event generation
    int syncFinancialEventsForUser(int userId, const std::string &currentDate = "");

    // Events management
    std::vector<Models::FinancialEvent> getEvents(int userId, const std::string &statusFilter = "");
    bool updateEventStatus(int eventId, const std::string &newStatus);

    // Instrument status updates
    bool markDepositAsMatured(int userId, int investmentId);
    bool markBondAsRedeemed(int userId, int investmentId);

    // Date math utility
    static int calculateDaysRemaining(const std::string &targetDate, const std::string &currentDate = "");

private:
    MaturityService() = default;
    MaturityService(const MaturityService&) = delete;
    MaturityService& operator=(const MaturityService&) = delete;
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_MATURITYSERVICE_H

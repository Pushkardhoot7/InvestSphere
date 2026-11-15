#ifndef FINANCIALEVENTMODEL_H
#define FINANCIALEVENTMODEL_H

#include "Compat.h"
#include <cstdint>

namespace InvestSphere {
namespace Models {

struct FinancialEvent {
    int id{0};
    int userId{0};
    InvestSphere::Optional<int> investmentId;
    std::string eventType; // FD_MATURITY, BOND_MATURITY, BOND_COUPON, SIP_INSTALLMENT, SWP_WITHDRAWAL, DIVIDEND_EXPECTED
    std::string eventDate;
    InvestSphere::Optional<int64_t> amountCents;
    std::string status{"PENDING"}; // PENDING, PROCESSED, CANCELLED, OVERDUE
    std::string description;
    std::string createdAt;

    // Joined view attributes
    std::string symbol;
    std::string instrumentName;
    int daysRemaining{0};
};

} // namespace Models
} // namespace InvestSphere

#endif // FINANCIALEVENTMODEL_H

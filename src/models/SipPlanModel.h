#ifndef SIPPLANMODEL_H
#define SIPPLANMODEL_H

#include "Compat.h"
#include <cstdint>

namespace InvestSphere {
namespace Models {

struct SipPlan {
    int id{0};
    int userId{0};
    InvestSphere::Optional<int> instrumentId;
    std::string planName;
    int64_t monthlyAmountCents{0};
    double expectedAnnualReturn{0.0};
    int durationMonths{12};
    std::string startDate;
    std::string endDate;
    int contributionDayOfMonth{1};
    int64_t projectedInvestedCapitalCents{0};
    int64_t projectedFutureValueCents{0};
    std::string status{"ACTIVE"};
    std::string createdAt;
};

} // namespace Models
} // namespace InvestSphere

#endif // SIPPLANMODEL_H

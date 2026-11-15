#ifndef SWPPLANMODEL_H
#define SWPPLANMODEL_H

#include "Compat.h"
#include <cstdint>

namespace InvestSphere {
namespace Models {

struct SwpPlan {
    int id{0};
    int userId{0};
    InvestSphere::Optional<int> instrumentId;
    std::string planName;
    int64_t initialCorpusCents{0};
    int64_t monthlyWithdrawalCents{0};
    double expectedAnnualReturn{0.0};
    int durationMonths{12};
    std::string startDate;
    int withdrawalDayOfMonth{1};
    int64_t projectedFinalBalanceCents{0};
    std::string status{"ACTIVE"};
    std::string createdAt;
};

} // namespace Models
} // namespace InvestSphere

#endif // SWPPLANMODEL_H

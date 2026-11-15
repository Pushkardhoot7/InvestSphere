#ifndef SWPREPOSITORY_H
#define SWPREPOSITORY_H

#include "SwpPlanModel.h"
#include "Compat.h"
#include <vector>
#include <string>

namespace InvestSphere {
namespace Database {

class SwpRepository {
public:
    SwpRepository() = default;

    InvestSphere::Optional<int> createPlan(const Models::SwpPlan &plan);
    InvestSphere::Optional<Models::SwpPlan> getPlanById(int planId);
    std::vector<Models::SwpPlan> getPlansByUserId(int userId);
    bool updatePlanStatus(int planId, const std::string &status);
    bool deletePlan(int planId);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // SWPREPOSITORY_H

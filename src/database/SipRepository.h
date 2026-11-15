#ifndef SIPREPOSITORY_H
#define SIPREPOSITORY_H

#include "SipPlanModel.h"
#include "Compat.h"
#include <vector>
#include <string>

namespace InvestSphere {
namespace Database {

class SipRepository {
public:
    SipRepository() = default;

    InvestSphere::Optional<int> createPlan(const Models::SipPlan &plan);
    InvestSphere::Optional<Models::SipPlan> getPlanById(int planId);
    std::vector<Models::SipPlan> getPlansByUserId(int userId);
    bool updatePlanStatus(int planId, const std::string &status);
    bool deletePlan(int planId);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // SIPREPOSITORY_H

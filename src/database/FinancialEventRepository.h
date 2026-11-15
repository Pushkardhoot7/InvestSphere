#ifndef FINANCIALEVENTREPOSITORY_H
#define FINANCIALEVENTREPOSITORY_H

#include "FinancialEventModel.h"
#include "Compat.h"
#include <vector>
#include <string>

namespace InvestSphere {
namespace Database {

class FinancialEventRepository {
public:
    FinancialEventRepository() = default;

    InvestSphere::Optional<int> createEvent(const Models::FinancialEvent &event);
    InvestSphere::Optional<Models::FinancialEvent> getEventById(int eventId);
    std::vector<Models::FinancialEvent> getUpcomingEvents(int userId, int daysAhead = 90);
    std::vector<Models::FinancialEvent> getEventsForUser(int userId, const std::string &statusFilter = "");
    bool eventExists(int userId, InvestSphere::Optional<int> investmentId, const std::string &eventType, const std::string &eventDate);
    int markOverdueEvents(int userId, const std::string &currentDate = "");
    bool updateEventStatus(int eventId, const std::string &status);
    bool deleteEvent(int eventId);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // FINANCIALEVENTREPOSITORY_H

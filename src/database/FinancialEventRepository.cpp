#include "FinancialEventRepository.h"
#include "DatabaseManager.h"

#include <map>
#include <algorithm>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

namespace {
#if !defined(INVESTSPHERE_HAS_QT) || !INVESTSPHERE_HAS_QT
InvestSphere::Mutex s_eventMutex;
std::map<int, Models::FinancialEvent> s_inMemoryEvents;
int s_nextEventId = 1;
#endif
} // anonymous namespace

InvestSphere::Optional<int> FinancialEventRepository::createEvent(const Models::FinancialEvent &event)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO financial_events ("
        "   user_id, investment_id, event_type, event_date, amount, status, description"
        ") VALUES ("
        "   :userId, :investmentId, :type, :date, :amount, :status, :desc"
        ");"
    ));
    query.bindValue(QStringLiteral(":userId"), event.userId);
    if (static_cast<bool>(event.investmentId)) {
        query.bindValue(QStringLiteral(":investmentId"), *event.investmentId);
    } else {
        query.bindValue(QStringLiteral(":investmentId"), QVariant(QMetaType(QMetaType::Int)));
    }
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(event.eventType));
    query.bindValue(QStringLiteral(":date"), QString::fromStdString(event.eventDate));
    if (static_cast<bool>(event.amountCents)) {
        query.bindValue(QStringLiteral(":amount"), static_cast<qlonglong>(*event.amountCents));
    } else {
        query.bindValue(QStringLiteral(":amount"), QVariant(QMetaType(QMetaType::LongLong)));
    }
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(event.status.empty() ? "PENDING" : event.status));
    query.bindValue(QStringLiteral(":desc"), QString::fromStdString(event.description));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }
    return query.lastInsertId().toInt();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    int id = s_nextEventId++;
    Models::FinancialEvent stored = event;
    stored.id = id;
    s_inMemoryEvents[id] = stored;
    return id;
#endif
}

InvestSphere::Optional<Models::FinancialEvent> FinancialEventRepository::getEventById(int eventId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT fe.event_id, fe.user_id, fe.investment_id, fe.event_type, "
        "       fe.event_date, fe.amount, fe.status, fe.description, fe.created_at, "
        "       inst.symbol, inst.instrument_name "
        "FROM financial_events fe "
        "LEFT JOIN investments inv ON fe.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE fe.event_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), eventId);

    if (query.exec() && query.next()) {
        Models::FinancialEvent ev;
        ev.id = query.value(0).toInt();
        ev.userId = query.value(1).toInt();
        ev.investmentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
        ev.eventType = query.value(3).toString().toStdString();
        ev.eventDate = query.value(4).toString().toStdString();
        ev.amountCents = query.value(5).isNull() ? InvestSphere::Optional<int64_t>() : InvestSphere::Optional<int64_t>(query.value(5).toLongLong());
        ev.status = query.value(6).toString().toStdString();
        ev.description = query.value(7).toString().toStdString();
        ev.createdAt = query.value(8).toString().toStdString();
        ev.symbol = query.value(9).toString().toStdString();
        ev.instrumentName = query.value(10).toString().toStdString();
        return ev;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    auto it = s_inMemoryEvents.find(eventId);
    if (it != s_inMemoryEvents.end()) {
        return it->second;
    }
    return {};
#endif
}

std::vector<Models::FinancialEvent> FinancialEventRepository::getUpcomingEvents(int userId, int daysAhead)
{
    std::vector<Models::FinancialEvent> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT fe.event_id, fe.user_id, fe.investment_id, fe.event_type, "
        "       fe.event_date, fe.amount, fe.status, fe.description, fe.created_at, "
        "       inst.symbol, inst.instrument_name, "
        "       CAST((julianday(fe.event_date) - julianday(date('now'))) AS INTEGER) AS days_remaining "
        "FROM financial_events fe "
        "LEFT JOIN investments inv ON fe.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE fe.user_id = :userId AND fe.status = 'PENDING' "
        "  AND (julianday(fe.event_date) - julianday(date('now'))) BETWEEN 0 AND :days "
        "ORDER BY fe.event_date ASC;"
    ));
    query.bindValue(QStringLiteral(":userId"), userId);
    query.bindValue(QStringLiteral(":days"), daysAhead);

    if (query.exec()) {
        while (query.next()) {
            Models::FinancialEvent ev;
            ev.id = query.value(0).toInt();
            ev.userId = query.value(1).toInt();
            ev.investmentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
            ev.eventType = query.value(3).toString().toStdString();
            ev.eventDate = query.value(4).toString().toStdString();
            ev.amountCents = query.value(5).isNull() ? InvestSphere::Optional<int64_t>() : InvestSphere::Optional<int64_t>(query.value(5).toLongLong());
            ev.status = query.value(6).toString().toStdString();
            ev.description = query.value(7).toString().toStdString();
            ev.createdAt = query.value(8).toString().toStdString();
            ev.symbol = query.value(9).toString().toStdString();
            ev.instrumentName = query.value(10).toString().toStdString();
            ev.daysRemaining = query.value(11).toInt();
            list.push_back(ev);
        }
    }
#else
    (void)daysAhead;
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    for (const auto &pair : s_inMemoryEvents) {
        if (pair.second.userId == userId && pair.second.status == "PENDING") {
            list.push_back(pair.second);
        }
    }
    std::sort(list.begin(), list.end(), [](const Models::FinancialEvent &a, const Models::FinancialEvent &b) {
        return a.eventDate < b.eventDate;
    });
#endif
    return list;
}

bool FinancialEventRepository::updateEventStatus(int eventId, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE financial_events SET status = :status WHERE event_id = :id;"));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status));
    query.bindValue(QStringLiteral(":id"), eventId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    auto it = s_inMemoryEvents.find(eventId);
    if (it != s_inMemoryEvents.end()) {
        it->second.status = status;
        return true;
    }
    return false;
#endif
}

bool FinancialEventRepository::deleteEvent(int eventId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM financial_events WHERE event_id = :id;"));
    query.bindValue(QStringLiteral(":id"), eventId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    return s_inMemoryEvents.erase(eventId) > 0;
#endif
}

std::vector<Models::FinancialEvent> FinancialEventRepository::getEventsForUser(int userId, const std::string &statusFilter)
{
    std::vector<Models::FinancialEvent> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    QString sql = QStringLiteral(
        "SELECT fe.event_id, fe.user_id, fe.investment_id, fe.event_type, "
        "       fe.event_date, fe.amount, fe.status, fe.description, fe.created_at, "
        "       inst.symbol, inst.instrument_name, "
        "       CAST((julianday(fe.event_date) - julianday(date('now'))) AS INTEGER) AS days_remaining "
        "FROM financial_events fe "
        "LEFT JOIN investments inv ON fe.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE fe.user_id = :userId "
    );
    if (!statusFilter.empty()) {
        sql += QStringLiteral(" AND fe.status = :status ");
    }
    sql += QStringLiteral(" ORDER BY fe.event_date ASC;");

    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), userId);
    if (!statusFilter.empty()) {
        query.bindValue(QStringLiteral(":status"), QString::fromStdString(statusFilter));
    }

    if (query.exec()) {
        while (query.next()) {
            Models::FinancialEvent ev;
            ev.id = query.value(0).toInt();
            ev.userId = query.value(1).toInt();
            ev.investmentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
            ev.eventType = query.value(3).toString().toStdString();
            ev.eventDate = query.value(4).toString().toStdString();
            ev.amountCents = query.value(5).isNull() ? InvestSphere::Optional<int64_t>() : InvestSphere::Optional<int64_t>(query.value(5).toLongLong());
            ev.status = query.value(6).toString().toStdString();
            ev.description = query.value(7).toString().toStdString();
            ev.createdAt = query.value(8).toString().toStdString();
            ev.symbol = query.value(9).toString().toStdString();
            ev.instrumentName = query.value(10).toString().toStdString();
            ev.daysRemaining = query.value(11).toInt();
            list.push_back(ev);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    for (const auto &pair : s_inMemoryEvents) {
        if (pair.second.userId == userId && (statusFilter.empty() || pair.second.status == statusFilter)) {
            list.push_back(pair.second);
        }
    }
    std::sort(list.begin(), list.end(), [](const Models::FinancialEvent &a, const Models::FinancialEvent &b) {
        return a.eventDate < b.eventDate;
    });
#endif
    return list;
}

bool FinancialEventRepository::eventExists(int userId, InvestSphere::Optional<int> investmentId, const std::string &eventType, const std::string &eventDate)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    if (static_cast<bool>(investmentId)) {
        query.prepare(QStringLiteral(
            "SELECT 1 FROM financial_events "
            "WHERE user_id = :userId AND investment_id = :invId AND event_type = :type AND event_date = :date LIMIT 1;"
        ));
        query.bindValue(QStringLiteral(":invId"), *investmentId);
    } else {
        query.prepare(QStringLiteral(
            "SELECT 1 FROM financial_events "
            "WHERE user_id = :userId AND investment_id IS NULL AND event_type = :type AND event_date = :date LIMIT 1;"
        ));
    }
    query.bindValue(QStringLiteral(":userId"), userId);
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(eventType));
    query.bindValue(QStringLiteral(":date"), QString::fromStdString(eventDate));

    if (query.exec() && query.next()) {
        return true;
    }
    return false;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    for (const auto &pair : s_inMemoryEvents) {
        const auto &ev = pair.second;
        if (ev.userId == userId && ev.eventType == eventType && ev.eventDate == eventDate) {
            if ((!static_cast<bool>(investmentId) && !static_cast<bool>(ev.investmentId)) ||
                (static_cast<bool>(investmentId) && static_cast<bool>(ev.investmentId) && *investmentId == *ev.investmentId)) {
                return true;
            }
        }
    }
    return false;
#endif
}

int FinancialEventRepository::markOverdueEvents(int userId, const std::string &currentDate)
{
    std::string dateStr = currentDate.empty() ? "date('now')" : "'" + currentDate + "'";
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    QString sql = QStringLiteral("UPDATE financial_events SET status = 'OVERDUE' WHERE user_id = :userId AND status = 'PENDING' AND event_date < %1;").arg(QString::fromStdString(dateStr));
    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), userId);
    if (query.exec()) {
        return query.numRowsAffected();
    }
    return 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_eventMutex);
    int count = 0;
    std::string today = currentDate.empty() ? "2026-10-27" : currentDate;
    for (auto &pair : s_inMemoryEvents) {
        if (pair.second.userId == userId && pair.second.status == "PENDING" && pair.second.eventDate < today) {
            pair.second.status = "OVERDUE";
            ++count;
        }
    }
    return count;
#endif
}

} // namespace Database
} // namespace InvestSphere

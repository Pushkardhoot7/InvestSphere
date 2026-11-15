#include "SipRepository.h"
#include "DatabaseManager.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

InvestSphere::Optional<int> SipRepository::createPlan(const Models::SipPlan &plan)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO sip_plans ("
        "   user_id, instrument_id, plan_name, monthly_amount, "
        "   expected_annual_return, duration_months, start_date, end_date, "
        "   contribution_day_of_month, projected_invested_capital, projected_future_value, status"
        ") VALUES ("
        "   :userId, :instrumentId, :name, :amount, "
        "   :returnRate, :duration, :start, :end, "
        "   :day, :invested, :futureVal, :status"
        ");"
    ));
    query.bindValue(QStringLiteral(":userId"), plan.userId);
    if (static_cast<bool>(plan.instrumentId)) {
        query.bindValue(QStringLiteral(":instrumentId"), *plan.instrumentId);
    } else {
        query.bindValue(QStringLiteral(":instrumentId"), QVariant(QMetaType(QMetaType::Int)));
    }
    query.bindValue(QStringLiteral(":name"), QString::fromStdString(plan.planName));
    query.bindValue(QStringLiteral(":amount"), static_cast<qlonglong>(plan.monthlyAmountCents));
    query.bindValue(QStringLiteral(":returnRate"), plan.expectedAnnualReturn);
    query.bindValue(QStringLiteral(":duration"), plan.durationMonths);
    query.bindValue(QStringLiteral(":start"), QString::fromStdString(plan.startDate));
    query.bindValue(QStringLiteral(":end"), QString::fromStdString(plan.endDate));
    query.bindValue(QStringLiteral(":day"), plan.contributionDayOfMonth);
    query.bindValue(QStringLiteral(":invested"), static_cast<qlonglong>(plan.projectedInvestedCapitalCents));
    query.bindValue(QStringLiteral(":futureVal"), static_cast<qlonglong>(plan.projectedFutureValueCents));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(plan.status.empty() ? "ACTIVE" : plan.status));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }
    return query.lastInsertId().toInt();
#else
    (void)plan;
    return 1;
#endif
}

InvestSphere::Optional<Models::SipPlan> SipRepository::getPlanById(int planId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT sip_plan_id, user_id, instrument_id, plan_name, monthly_amount, "
        "       expected_annual_return, duration_months, start_date, end_date, "
        "       contribution_day_of_month, projected_invested_capital, projected_future_value, status, created_at "
        "FROM sip_plans WHERE sip_plan_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), planId);

    if (query.exec() && query.next()) {
        Models::SipPlan p;
        p.id = query.value(0).toInt();
        p.userId = query.value(1).toInt();
        p.instrumentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
        p.planName = query.value(3).toString().toStdString();
        p.monthlyAmountCents = query.value(4).toLongLong();
        p.expectedAnnualReturn = query.value(5).toDouble();
        p.durationMonths = query.value(6).toInt();
        p.startDate = query.value(7).toString().toStdString();
        p.endDate = query.value(8).toString().toStdString();
        p.contributionDayOfMonth = query.value(9).toInt();
        p.projectedInvestedCapitalCents = query.value(10).toLongLong();
        p.projectedFutureValueCents = query.value(11).toLongLong();
        p.status = query.value(12).toString().toStdString();
        p.createdAt = query.value(13).toString().toStdString();
        return p;
    }
    return {};
#else
    (void)planId;
    return {};
#endif
}

std::vector<Models::SipPlan> SipRepository::getPlansByUserId(int userId)
{
    std::vector<Models::SipPlan> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT sip_plan_id, user_id, instrument_id, plan_name, monthly_amount, "
        "       expected_annual_return, duration_months, start_date, end_date, "
        "       contribution_day_of_month, projected_invested_capital, projected_future_value, status, created_at "
        "FROM sip_plans WHERE user_id = :userId ORDER BY created_at DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), userId);

    if (query.exec()) {
        while (query.next()) {
            Models::SipPlan p;
            p.id = query.value(0).toInt();
            p.userId = query.value(1).toInt();
            p.instrumentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
            p.planName = query.value(3).toString().toStdString();
            p.monthlyAmountCents = query.value(4).toLongLong();
            p.expectedAnnualReturn = query.value(5).toDouble();
            p.durationMonths = query.value(6).toInt();
            p.startDate = query.value(7).toString().toStdString();
            p.endDate = query.value(8).toString().toStdString();
            p.contributionDayOfMonth = query.value(9).toInt();
            p.projectedInvestedCapitalCents = query.value(10).toLongLong();
            p.projectedFutureValueCents = query.value(11).toLongLong();
            p.status = query.value(12).toString().toStdString();
            p.createdAt = query.value(13).toString().toStdString();
            list.push_back(p);
        }
    }
#else
    (void)userId;
#endif
    return list;
}

bool SipRepository::updatePlanStatus(int planId, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE sip_plans SET status = :status WHERE sip_plan_id = :id;"));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status));
    query.bindValue(QStringLiteral(":id"), planId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    (void)planId;
    (void)status;
    return true;
#endif
}

bool SipRepository::deletePlan(int planId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM sip_plans WHERE sip_plan_id = :id;"));
    query.bindValue(QStringLiteral(":id"), planId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    (void)planId;
    return true;
#endif
}

} // namespace Database
} // namespace InvestSphere

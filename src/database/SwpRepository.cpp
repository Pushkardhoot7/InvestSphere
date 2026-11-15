#include "SwpRepository.h"
#include "DatabaseManager.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

InvestSphere::Optional<int> SwpRepository::createPlan(const Models::SwpPlan &plan)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO swp_plans ("
        "   user_id, instrument_id, plan_name, initial_corpus, "
        "   monthly_withdrawal, expected_annual_return, duration_months, start_date, "
        "   withdrawal_day_of_month, projected_final_balance, status"
        ") VALUES ("
        "   :userId, :instrumentId, :name, :corpus, "
        "   :withdrawal, :returnRate, :duration, :start, "
        "   :day, :finalBal, :status"
        ");"
    ));
    query.bindValue(QStringLiteral(":userId"), plan.userId);
    if (static_cast<bool>(plan.instrumentId)) {
        query.bindValue(QStringLiteral(":instrumentId"), *plan.instrumentId);
    } else {
        query.bindValue(QStringLiteral(":instrumentId"), QVariant(QMetaType(QMetaType::Int)));
    }
    query.bindValue(QStringLiteral(":name"), QString::fromStdString(plan.planName));
    query.bindValue(QStringLiteral(":corpus"), static_cast<qlonglong>(plan.initialCorpusCents));
    query.bindValue(QStringLiteral(":withdrawal"), static_cast<qlonglong>(plan.monthlyWithdrawalCents));
    query.bindValue(QStringLiteral(":returnRate"), plan.expectedAnnualReturn);
    query.bindValue(QStringLiteral(":duration"), plan.durationMonths);
    query.bindValue(QStringLiteral(":start"), QString::fromStdString(plan.startDate));
    query.bindValue(QStringLiteral(":day"), plan.withdrawalDayOfMonth);
    query.bindValue(QStringLiteral(":finalBal"), static_cast<qlonglong>(plan.projectedFinalBalanceCents));
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

InvestSphere::Optional<Models::SwpPlan> SwpRepository::getPlanById(int planId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT swp_plan_id, user_id, instrument_id, plan_name, initial_corpus, "
        "       monthly_withdrawal, expected_annual_return, duration_months, start_date, "
        "       withdrawal_day_of_month, projected_final_balance, status, created_at "
        "FROM swp_plans WHERE swp_plan_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), planId);

    if (query.exec() && query.next()) {
        Models::SwpPlan p;
        p.id = query.value(0).toInt();
        p.userId = query.value(1).toInt();
        p.instrumentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
        p.planName = query.value(3).toString().toStdString();
        p.initialCorpusCents = query.value(4).toLongLong();
        p.monthlyWithdrawalCents = query.value(5).toLongLong();
        p.expectedAnnualReturn = query.value(6).toDouble();
        p.durationMonths = query.value(7).toInt();
        p.startDate = query.value(8).toString().toStdString();
        p.withdrawalDayOfMonth = query.value(9).toInt();
        p.projectedFinalBalanceCents = query.value(10).toLongLong();
        p.status = query.value(11).toString().toStdString();
        p.createdAt = query.value(12).toString().toStdString();
        return p;
    }
    return {};
#else
    (void)planId;
    return {};
#endif
}

std::vector<Models::SwpPlan> SwpRepository::getPlansByUserId(int userId)
{
    std::vector<Models::SwpPlan> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT swp_plan_id, user_id, instrument_id, plan_name, initial_corpus, "
        "       monthly_withdrawal, expected_annual_return, duration_months, start_date, "
        "       withdrawal_day_of_month, projected_final_balance, status, created_at "
        "FROM swp_plans WHERE user_id = :userId ORDER BY created_at DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), userId);

    if (query.exec()) {
        while (query.next()) {
            Models::SwpPlan p;
            p.id = query.value(0).toInt();
            p.userId = query.value(1).toInt();
            p.instrumentId = query.value(2).isNull() ? InvestSphere::Optional<int>() : InvestSphere::Optional<int>(query.value(2).toInt());
            p.planName = query.value(3).toString().toStdString();
            p.initialCorpusCents = query.value(4).toLongLong();
            p.monthlyWithdrawalCents = query.value(5).toLongLong();
            p.expectedAnnualReturn = query.value(6).toDouble();
            p.durationMonths = query.value(7).toInt();
            p.startDate = query.value(8).toString().toStdString();
            p.withdrawalDayOfMonth = query.value(9).toInt();
            p.projectedFinalBalanceCents = query.value(10).toLongLong();
            p.status = query.value(11).toString().toStdString();
            p.createdAt = query.value(12).toString().toStdString();
            list.push_back(p);
        }
    }
#else
    (void)userId;
#endif
    return list;
}

bool SwpRepository::updatePlanStatus(int planId, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE swp_plans SET status = :status WHERE swp_plan_id = :id;"));
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

bool SwpRepository::deletePlan(int planId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM swp_plans WHERE swp_plan_id = :id;"));
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

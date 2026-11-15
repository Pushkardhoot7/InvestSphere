#include "PortfolioSnapshotRepository.h"
#include "DatabaseManager.h"

#include <map>
#include <algorithm>
#include <iostream>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QString>
#endif

namespace InvestSphere {
namespace Database {

namespace {
#if !defined(INVESTSPHERE_HAS_QT) || !INVESTSPHERE_HAS_QT
// In-memory fallback for non-Qt builds (unit tests)
InvestSphere::Mutex s_snapMutex;
std::map<int, PortfolioSnapshot> s_inMemorySnapshots; // snapshotId -> snapshot
int s_nextSnapshotId = 1;
#endif
} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// recordSnapshot
// ─────────────────────────────────────────────────────────────────────────────

InvestSphere::Optional<int> PortfolioSnapshotRepository::recordSnapshot(const PortfolioSnapshot &snapshot)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);

    // Extract date portion of timestamp for deduplication (one snapshot per user per day)
    QString dateOnly = QString::fromStdString(snapshot.snapshotTimestamp).left(10);

    // Check for existing snapshot on same calendar day for this user
    query.prepare(QStringLiteral(
        "SELECT snapshot_id FROM portfolio_snapshots "
        "WHERE user_id = :uid AND date(snapshot_timestamp) = date(:day) "
        "ORDER BY snapshot_timestamp DESC LIMIT 1;"
    ));
    query.bindValue(QStringLiteral(":uid"), snapshot.userId);
    query.bindValue(QStringLiteral(":day"), dateOnly);

    if (query.exec() && query.next()) {
        int existingId = query.value(0).toInt();
        // Update existing record
        QSqlQuery upd(db);
        upd.prepare(QStringLiteral(
            "UPDATE portfolio_snapshots SET "
            "  total_invested = :invested, "
            "  estimated_portfolio_value = :value, "
            "  realized_profit_loss = :realized, "
            "  unrealized_profit_loss = :unrealized, "
            "  notes = :notes "
            "WHERE snapshot_id = :id;"
        ));
        upd.bindValue(QStringLiteral(":invested"),   static_cast<qlonglong>(snapshot.totalInvestedCents));
        upd.bindValue(QStringLiteral(":value"),      static_cast<qlonglong>(snapshot.estimatedValueCents));
        upd.bindValue(QStringLiteral(":realized"),   static_cast<qlonglong>(snapshot.realizedPnlCents));
        upd.bindValue(QStringLiteral(":unrealized"), static_cast<qlonglong>(snapshot.unrealizedPnlCents));
        upd.bindValue(QStringLiteral(":notes"),      QString::fromStdString(snapshot.notes));
        upd.bindValue(QStringLiteral(":id"),         existingId);
        if (!upd.exec()) {
            m_lastError = upd.lastError().text().toStdString();
            return {};
        }
        return existingId;
    }

    // Insert new snapshot
    QSqlQuery ins(db);
    ins.prepare(QStringLiteral(
        "INSERT INTO portfolio_snapshots "
        "  (user_id, snapshot_timestamp, total_invested, estimated_portfolio_value, "
        "   realized_profit_loss, unrealized_profit_loss, notes) "
        "VALUES (:uid, :ts, :invested, :value, :realized, :unrealized, :notes);"
    ));
    ins.bindValue(QStringLiteral(":uid"),        snapshot.userId);
    ins.bindValue(QStringLiteral(":ts"),         QString::fromStdString(snapshot.snapshotTimestamp));
    ins.bindValue(QStringLiteral(":invested"),   static_cast<qlonglong>(snapshot.totalInvestedCents));
    ins.bindValue(QStringLiteral(":value"),      static_cast<qlonglong>(snapshot.estimatedValueCents));
    ins.bindValue(QStringLiteral(":realized"),   static_cast<qlonglong>(snapshot.realizedPnlCents));
    ins.bindValue(QStringLiteral(":unrealized"), static_cast<qlonglong>(snapshot.unrealizedPnlCents));
    ins.bindValue(QStringLiteral(":notes"),      QString::fromStdString(snapshot.notes));

    if (!ins.exec()) {
        m_lastError = ins.lastError().text().toStdString();
        return {};
    }
    return ins.lastInsertId().toInt();

#else
    // Non-Qt in-memory fallback
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_snapMutex);

    // Simple date dedup: compare first 10 chars of timestamp
    std::string day = snapshot.snapshotTimestamp.size() >= 10
        ? snapshot.snapshotTimestamp.substr(0, 10)
        : snapshot.snapshotTimestamp;

    for (auto &kv : s_inMemorySnapshots) {
        auto &s = kv.second;
        if (s.userId == snapshot.userId &&
            s.snapshotTimestamp.substr(0, 10) == day) {
            s.totalInvestedCents  = snapshot.totalInvestedCents;
            s.estimatedValueCents = snapshot.estimatedValueCents;
            s.realizedPnlCents    = snapshot.realizedPnlCents;
            s.unrealizedPnlCents  = snapshot.unrealizedPnlCents;
            s.notes               = snapshot.notes;
            return kv.first;
        }
    }

    int id = s_nextSnapshotId++;
    PortfolioSnapshot stored = snapshot;
    stored.snapshotId = id;
    s_inMemorySnapshots[id] = stored;
    return id;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// getSnapshots
// ─────────────────────────────────────────────────────────────────────────────

std::vector<PortfolioSnapshot> PortfolioSnapshotRepository::getSnapshots(
    int userId, const std::string &startDate, const std::string &endDate, int limitRows)
{
    std::vector<PortfolioSnapshot> results;

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery q(db);

    QString sql = QStringLiteral(
        "SELECT snapshot_id, user_id, snapshot_timestamp, "
        "  total_invested, estimated_portfolio_value, "
        "  realized_profit_loss, unrealized_profit_loss, notes "
        "FROM portfolio_snapshots WHERE user_id = :uid "
    );
    if (!startDate.empty()) sql += QStringLiteral(" AND date(snapshot_timestamp) >= :start");
    if (!endDate.empty())   sql += QStringLiteral(" AND date(snapshot_timestamp) <= :end");
    sql += QStringLiteral(" ORDER BY snapshot_timestamp ASC");
    if (limitRows > 0) sql += QStringLiteral(" LIMIT :lim");
    sql += QStringLiteral(";");

    q.prepare(sql);
    q.bindValue(QStringLiteral(":uid"), userId);
    if (!startDate.empty()) q.bindValue(QStringLiteral(":start"), QString::fromStdString(startDate));
    if (!endDate.empty())   q.bindValue(QStringLiteral(":end"),   QString::fromStdString(endDate));
    if (limitRows > 0)      q.bindValue(QStringLiteral(":lim"),   limitRows);

    if (!q.exec()) {
        m_lastError = q.lastError().text().toStdString();
        return results;
    }
    while (q.next()) {
        PortfolioSnapshot s;
        s.snapshotId          = q.value(0).toInt();
        s.userId              = q.value(1).toInt();
        s.snapshotTimestamp   = q.value(2).toString().toStdString();
        s.totalInvestedCents  = q.value(3).toLongLong();
        s.estimatedValueCents = q.value(4).toLongLong();
        s.realizedPnlCents    = q.value(5).toLongLong();
        s.unrealizedPnlCents  = q.value(6).toLongLong();
        s.notes               = q.value(7).toString().toStdString();
        results.push_back(s);
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_snapMutex);
    for (const auto &kv : s_inMemorySnapshots) {
        const auto &s = kv.second;
        if (s.userId != userId) continue;
        std::string day = s.snapshotTimestamp.size() >= 10 ? s.snapshotTimestamp.substr(0, 10) : "";
        if (!startDate.empty() && day < startDate) continue;
        if (!endDate.empty()   && day > endDate)   continue;
        results.push_back(s);
    }
    std::sort(results.begin(), results.end(), [](const PortfolioSnapshot &a, const PortfolioSnapshot &b) {
        return a.snapshotTimestamp < b.snapshotTimestamp;
    });
    if (limitRows > 0 && static_cast<int>(results.size()) > limitRows) {
        results.resize(static_cast<size_t>(limitRows));
    }
#endif
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// getLatestSnapshot
// ─────────────────────────────────────────────────────────────────────────────

InvestSphere::Optional<PortfolioSnapshot> PortfolioSnapshotRepository::getLatestSnapshot(int userId)
{
    auto results = getSnapshots(userId, "", "", 0);
    if (results.empty()) return {};
    return results.back();
}

// ─────────────────────────────────────────────────────────────────────────────
// countSnapshots
// ─────────────────────────────────────────────────────────────────────────────

int PortfolioSnapshotRepository::countSnapshots(int userId,
                                                const std::string &startDate,
                                                const std::string &endDate)
{
    return static_cast<int>(getSnapshots(userId, startDate, endDate, 0).size());
}

} // namespace Database
} // namespace InvestSphere

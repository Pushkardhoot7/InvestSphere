#include "TransactionRepository.h"
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
InvestSphere::Mutex s_txnMutex;
std::map<int, Models::Transaction> s_inMemoryTransactions;
int s_nextTransactionId = 1;
#endif
} // anonymous namespace

InvestSphere::Optional<int> TransactionRepository::recordTransaction(const Models::Transaction &tx)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO transactions ("
        "   user_id, investment_id, transaction_type, transaction_date, "
        "   quantity, unit_price, gross_amount, fees, taxes, net_amount, notes"
        ") VALUES ("
        "   :userId, :investmentId, :type, :date, "
        "   :qty, :price, :gross, :fees, :taxes, :net, :notes"
        ");"
    ));
    query.bindValue(QStringLiteral(":userId"), tx.userId);
    if (tx.investmentId > 0) {
        query.bindValue(QStringLiteral(":investmentId"), tx.investmentId);
    } else {
        query.bindValue(QStringLiteral(":investmentId"), QVariant(QMetaType(QMetaType::Int)));
    }
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(tx.transactionType));
    query.bindValue(QStringLiteral(":date"), QString::fromStdString(tx.transactionDate.empty() ? "2026-10-26 12:00:00" : tx.transactionDate));
    if (tx.quantity > 0.0) {
        query.bindValue(QStringLiteral(":qty"), tx.quantity);
    } else {
        query.bindValue(QStringLiteral(":qty"), QVariant(QMetaType(QMetaType::Double)));
    }
    if (tx.unitPriceCents > 0) {
        query.bindValue(QStringLiteral(":price"), static_cast<qlonglong>(tx.unitPriceCents));
    } else {
        query.bindValue(QStringLiteral(":price"), QVariant(QMetaType(QMetaType::LongLong)));
    }
    query.bindValue(QStringLiteral(":gross"), static_cast<qlonglong>(tx.grossAmountCents));
    query.bindValue(QStringLiteral(":fees"), static_cast<qlonglong>(tx.feesCents));
    query.bindValue(QStringLiteral(":taxes"), static_cast<qlonglong>(tx.taxesCents));
    query.bindValue(QStringLiteral(":net"), static_cast<qlonglong>(tx.netAmountCents));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(tx.notes));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }
    return query.lastInsertId().toInt();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    int id = s_nextTransactionId++;
    Models::Transaction stored = tx;
    stored.id = id;
    s_inMemoryTransactions[id] = stored;
    return id;
#endif
}

bool TransactionRepository::updateTransaction(const Models::Transaction &tx)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE transactions SET "
        "   transaction_type = :type, "
        "   transaction_date = :date, "
        "   quantity = :qty, "
        "   unit_price = :price, "
        "   gross_amount = :gross, "
        "   fees = :fees, "
        "   taxes = :taxes, "
        "   net_amount = :net, "
        "   notes = :notes "
        "WHERE transaction_id = :id AND user_id = :userId;"
    ));
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(tx.transactionType));
    query.bindValue(QStringLiteral(":date"), QString::fromStdString(tx.transactionDate));
    if (tx.quantity > 0.0) {
        query.bindValue(QStringLiteral(":qty"), tx.quantity);
    } else {
        query.bindValue(QStringLiteral(":qty"), QVariant(QMetaType(QMetaType::Double)));
    }
    if (tx.unitPriceCents > 0) {
        query.bindValue(QStringLiteral(":price"), static_cast<qlonglong>(tx.unitPriceCents));
    } else {
        query.bindValue(QStringLiteral(":price"), QVariant(QMetaType(QMetaType::LongLong)));
    }
    query.bindValue(QStringLiteral(":gross"), static_cast<qlonglong>(tx.grossAmountCents));
    query.bindValue(QStringLiteral(":fees"), static_cast<qlonglong>(tx.feesCents));
    query.bindValue(QStringLiteral(":taxes"), static_cast<qlonglong>(tx.taxesCents));
    query.bindValue(QStringLiteral(":net"), static_cast<qlonglong>(tx.netAmountCents));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(tx.notes));
    query.bindValue(QStringLiteral(":id"), tx.id);
    query.bindValue(QStringLiteral(":userId"), tx.userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    auto it = s_inMemoryTransactions.find(tx.id);
    if (it != s_inMemoryTransactions.end() && it->second.userId == tx.userId) {
        Models::Transaction updated = tx;
        if (updated.symbol.empty()) updated.symbol = it->second.symbol;
        if (updated.instrumentName.empty()) updated.instrumentName = it->second.instrumentName;
        if (updated.categoryCode.empty()) updated.categoryCode = it->second.categoryCode;
        if (updated.categoryName.empty()) updated.categoryName = it->second.categoryName;
        it->second = updated;
        return true;
    }
    return false;
#endif
}

bool TransactionRepository::deleteTransaction(int transactionId, int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM transactions WHERE transaction_id = :id AND user_id = :userId;"));
    query.bindValue(QStringLiteral(":id"), transactionId);
    query.bindValue(QStringLiteral(":userId"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    auto it = s_inMemoryTransactions.find(transactionId);
    if (it != s_inMemoryTransactions.end() && it->second.userId == userId) {
        s_inMemoryTransactions.erase(it);
        return true;
    }
    return false;
#endif
}

InvestSphere::Optional<Models::Transaction> TransactionRepository::getTransactionById(int transactionId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT t.transaction_id, t.user_id, t.investment_id, t.transaction_type, "
        "       t.transaction_date, t.quantity, t.unit_price, t.gross_amount, "
        "       t.fees, t.taxes, t.net_amount, t.notes, t.created_at, "
        "       inst.symbol, inst.instrument_name, "
        "       it.type_code, it.type_name "
        "FROM transactions t "
        "LEFT JOIN investments inv ON t.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE t.transaction_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), transactionId);

    if (query.exec() && query.next()) {
        Models::Transaction tx;
        tx.id = query.value(0).toInt();
        tx.userId = query.value(1).toInt();
        tx.investmentId = query.value(2).isNull() ? 0 : query.value(2).toInt();
        tx.transactionType = query.value(3).toString().toStdString();
        tx.transactionDate = query.value(4).toString().toStdString();
        tx.quantity = query.value(5).isNull() ? 0.0 : query.value(5).toDouble();
        tx.unitPriceCents = query.value(6).isNull() ? 0 : query.value(6).toLongLong();
        tx.grossAmountCents = query.value(7).toLongLong();
        tx.feesCents = query.value(8).toLongLong();
        tx.taxesCents = query.value(9).toLongLong();
        tx.netAmountCents = query.value(10).toLongLong();
        tx.notes = query.value(11).toString().toStdString();
        tx.createdAt = query.value(12).toString().toStdString();
        tx.symbol = query.value(13).toString().toStdString();
        tx.instrumentName = query.value(14).toString().toStdString();
        tx.categoryCode = query.value(15).toString().toStdString();
        tx.categoryName = query.value(16).toString().toStdString();
        return tx;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    auto it = s_inMemoryTransactions.find(transactionId);
    if (it != s_inMemoryTransactions.end()) {
        return it->second;
    }
    return {};
#endif
}

std::vector<Models::Transaction> TransactionRepository::getTransactionsByUserId(int userId)
{
    return getFilteredTransactions(userId, "", "ALL", "", "", "DATE", false);
}

std::vector<Models::Transaction> TransactionRepository::getTransactionsByInvestmentId(int investmentId)
{
    std::vector<Models::Transaction> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT t.transaction_id, t.user_id, t.investment_id, t.transaction_type, "
        "       t.transaction_date, t.quantity, t.unit_price, t.gross_amount, "
        "       t.fees, t.taxes, t.net_amount, t.notes, t.created_at, "
        "       inst.symbol, inst.instrument_name, "
        "       it.type_code, it.type_name "
        "FROM transactions t "
        "LEFT JOIN investments inv ON t.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE t.investment_id = :id "
        "ORDER BY t.transaction_date ASC, t.transaction_id ASC;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);

    if (query.exec()) {
        while (query.next()) {
            Models::Transaction tx;
            tx.id = query.value(0).toInt();
            tx.userId = query.value(1).toInt();
            tx.investmentId = query.value(2).isNull() ? 0 : query.value(2).toInt();
            tx.transactionType = query.value(3).toString().toStdString();
            tx.transactionDate = query.value(4).toString().toStdString();
            tx.quantity = query.value(5).isNull() ? 0.0 : query.value(5).toDouble();
            tx.unitPriceCents = query.value(6).isNull() ? 0 : query.value(6).toLongLong();
            tx.grossAmountCents = query.value(7).toLongLong();
            tx.feesCents = query.value(8).toLongLong();
            tx.taxesCents = query.value(9).toLongLong();
            tx.netAmountCents = query.value(10).toLongLong();
            tx.notes = query.value(11).toString().toStdString();
            tx.createdAt = query.value(12).toString().toStdString();
            tx.symbol = query.value(13).toString().toStdString();
            tx.instrumentName = query.value(14).toString().toStdString();
            tx.categoryCode = query.value(15).toString().toStdString();
            tx.categoryName = query.value(16).toString().toStdString();
            list.push_back(tx);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    for (const auto &pair : s_inMemoryTransactions) {
        if (pair.second.investmentId == investmentId) {
            list.push_back(pair.second);
        }
    }
    std::sort(list.begin(), list.end(), [](const Models::Transaction &a, const Models::Transaction &b) {
        if (a.transactionDate != b.transactionDate) return a.transactionDate < b.transactionDate;
        return a.id < b.id;
    });
#endif
    return list;
}

std::vector<Models::Transaction> TransactionRepository::getTransactionsByDateRange(int userId, const std::string &startDate, const std::string &endDate)
{
    return getFilteredTransactions(userId, "", "ALL", startDate, endDate, "DATE", false);
}

std::vector<Models::Transaction> TransactionRepository::getFilteredTransactions(int userId,
                                                                               const std::string &searchQuery,
                                                                               const std::string &typeFilter,
                                                                               const std::string &startDate,
                                                                               const std::string &endDate,
                                                                               const std::string &sortBy,
                                                                               bool sortAscending)
{
    std::vector<Models::Transaction> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    
    QString sql = QStringLiteral(
        "SELECT t.transaction_id, t.user_id, t.investment_id, t.transaction_type, "
        "       t.transaction_date, t.quantity, t.unit_price, t.gross_amount, "
        "       t.fees, t.taxes, t.net_amount, t.notes, t.created_at, "
        "       inst.symbol, inst.instrument_name, "
        "       it.type_code, it.type_name "
        "FROM transactions t "
        "LEFT JOIN investments inv ON t.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "LEFT JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE t.user_id = :userId "
    );

    if (!typeFilter.empty() && typeFilter != "ALL") {
        sql += QStringLiteral(" AND t.transaction_type = :typeFilter ");
    }
    if (!startDate.empty()) {
        sql += QStringLiteral(" AND t.transaction_date >= :startDate ");
    }
    if (!endDate.empty()) {
        sql += QStringLiteral(" AND t.transaction_date <= :endDate ");
    }
    if (!searchQuery.empty()) {
        sql += QStringLiteral(" AND (inst.symbol LIKE :query OR inst.instrument_name LIKE :query OR t.notes LIKE :query) ");
    }

    if (sortBy == "AMOUNT") {
        sql += QStringLiteral(" ORDER BY t.net_amount ") + (sortAscending ? QStringLiteral("ASC") : QStringLiteral("DESC"));
    } else {
        sql += QStringLiteral(" ORDER BY t.transaction_date ") + (sortAscending ? QStringLiteral("ASC") : QStringLiteral("DESC"))
            + QStringLiteral(", t.transaction_id ") + (sortAscending ? QStringLiteral("ASC") : QStringLiteral("DESC"));
    }

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), userId);
    if (!typeFilter.empty() && typeFilter != "ALL") {
        query.bindValue(QStringLiteral(":typeFilter"), QString::fromStdString(typeFilter));
    }
    if (!startDate.empty()) {
        query.bindValue(QStringLiteral(":startDate"), QString::fromStdString(startDate));
    }
    if (!endDate.empty()) {
        query.bindValue(QStringLiteral(":endDate"), QString::fromStdString(endDate));
    }
    if (!searchQuery.empty()) {
        QString qLike = QStringLiteral("%") + QString::fromStdString(searchQuery) + QStringLiteral("%");
        query.bindValue(QStringLiteral(":query"), qLike);
    }

    if (query.exec()) {
        while (query.next()) {
            Models::Transaction tx;
            tx.id = query.value(0).toInt();
            tx.userId = query.value(1).toInt();
            tx.investmentId = query.value(2).isNull() ? 0 : query.value(2).toInt();
            tx.transactionType = query.value(3).toString().toStdString();
            tx.transactionDate = query.value(4).toString().toStdString();
            tx.quantity = query.value(5).isNull() ? 0.0 : query.value(5).toDouble();
            tx.unitPriceCents = query.value(6).isNull() ? 0 : query.value(6).toLongLong();
            tx.grossAmountCents = query.value(7).toLongLong();
            tx.feesCents = query.value(8).toLongLong();
            tx.taxesCents = query.value(9).toLongLong();
            tx.netAmountCents = query.value(10).toLongLong();
            tx.notes = query.value(11).toString().toStdString();
            tx.createdAt = query.value(12).toString().toStdString();
            tx.symbol = query.value(13).toString().toStdString();
            tx.instrumentName = query.value(14).toString().toStdString();
            tx.categoryCode = query.value(15).toString().toStdString();
            tx.categoryName = query.value(16).toString().toStdString();
            list.push_back(tx);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    for (const auto &pair : s_inMemoryTransactions) {
        const auto &tx = pair.second;
        if (tx.userId != userId) continue;

        if (!typeFilter.empty() && typeFilter != "ALL" && tx.transactionType != typeFilter) {
            continue;
        }
        if (!startDate.empty() && tx.transactionDate < startDate) {
            continue;
        }
        if (!endDate.empty() && tx.transactionDate > endDate) {
            continue;
        }
        if (!searchQuery.empty()) {
            std::string q = searchQuery;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            std::string sym = tx.symbol;
            std::transform(sym.begin(), sym.end(), sym.begin(), ::tolower);
            std::string name = tx.instrumentName;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::string notes = tx.notes;
            std::transform(notes.begin(), notes.end(), notes.begin(), ::tolower);

            if (sym.find(q) == std::string::npos &&
                name.find(q) == std::string::npos &&
                notes.find(q) == std::string::npos) {
                continue;
            }
        }
        list.push_back(tx);
    }

    std::sort(list.begin(), list.end(), [sortBy, sortAscending](const Models::Transaction &a, const Models::Transaction &b) {
        if (sortBy == "AMOUNT") {
            if (a.netAmountCents != b.netAmountCents) {
                return sortAscending ? a.netAmountCents < b.netAmountCents : a.netAmountCents > b.netAmountCents;
            }
        }
        if (a.transactionDate != b.transactionDate) {
            return sortAscending ? a.transactionDate < b.transactionDate : a.transactionDate > b.transactionDate;
        }
        return sortAscending ? a.id < b.id : a.id > b.id;
    });
#endif
    return list;
}

int TransactionRepository::countTransactionsByUserId(int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT COUNT(*) FROM transactions WHERE user_id = :userId;"));
    query.bindValue(QStringLiteral(":userId"), userId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_txnMutex);
    int count = 0;
    for (const auto &pair : s_inMemoryTransactions) {
        if (pair.second.userId == userId) count++;
    }
    return count;
#endif
}

} // namespace Database
} // namespace InvestSphere

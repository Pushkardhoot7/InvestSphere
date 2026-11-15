#include "UserRepository.h"
#include "DatabaseManager.h"

#include <algorithm>
#include <cctype>
#include <map>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

namespace {
#if !defined(INVESTSPHERE_HAS_QT) || !INVESTSPHERE_HAS_QT
// Thread-safe in-memory store for pure C++ testing/foundation environments
InvestSphere::Mutex s_userStoreMutex;
std::map<int, Models::User> s_inMemoryUsers;
int s_nextUserId = 1;

std::string toLower(const std::string &str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return lower;
}
#endif
} // anonymous namespace

InvestSphere::Optional<int> UserRepository::createUser(const Models::User &user)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO users (full_name, email, password_hash, account_status) "
        "VALUES (:name, :email, :hash, :status);"
    ));
    query.bindValue(QStringLiteral(":name"), QString::fromStdString(user.fullName));
    query.bindValue(QStringLiteral(":email"), QString::fromStdString(user.email));
    query.bindValue(QStringLiteral(":hash"), QString::fromStdString(user.passwordHash));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(user.accountStatus.empty() ? "ACTIVE" : user.accountStatus));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }

    return query.lastInsertId().toInt();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    std::string lowerEmail = toLower(user.email);
    for (const auto &pair : s_inMemoryUsers) {
        if (toLower(pair.second.email) == lowerEmail) {
            m_lastError = "UNIQUE constraint failed: users.email";
            return {};
        }
    }

    int id = s_nextUserId++;
    Models::User stored = user;
    stored.id = id;
    stored.createdAt = "2026-10-27 10:00:00";
    stored.updatedAt = "2026-10-27 10:00:00";
    if (stored.accountStatus.empty()) stored.accountStatus = "ACTIVE";

    s_inMemoryUsers[id] = stored;
    return id;
#endif
}

InvestSphere::Optional<Models::User> UserRepository::getUserById(int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT user_id, full_name, email, password_hash, account_status, created_at, updated_at "
        "FROM users WHERE user_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), userId);

    if (query.exec() && query.next()) {
        Models::User u;
        u.id = query.value(0).toInt();
        u.fullName = query.value(1).toString().toStdString();
        u.email = query.value(2).toString().toStdString();
        u.passwordHash = query.value(3).toString().toStdString();
        u.accountStatus = query.value(4).toString().toStdString();
        u.createdAt = query.value(5).toString().toStdString();
        u.updatedAt = query.value(6).toString().toStdString();
        return u;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    auto it = s_inMemoryUsers.find(userId);
    if (it != s_inMemoryUsers.end()) {
        return it->second;
    }
    return {};
#endif
}

InvestSphere::Optional<Models::User> UserRepository::getUserByEmail(const std::string &email)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT user_id, full_name, email, password_hash, account_status, created_at, updated_at "
        "FROM users WHERE email = :email COLLATE NOCASE;"
    ));
    query.bindValue(QStringLiteral(":email"), QString::fromStdString(email));

    if (query.exec() && query.next()) {
        Models::User u;
        u.id = query.value(0).toInt();
        u.fullName = query.value(1).toString().toStdString();
        u.email = query.value(2).toString().toStdString();
        u.passwordHash = query.value(3).toString().toStdString();
        u.accountStatus = query.value(4).toString().toStdString();
        u.createdAt = query.value(5).toString().toStdString();
        u.updatedAt = query.value(6).toString().toStdString();
        return u;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    std::string lowerEmail = toLower(email);
    for (const auto &pair : s_inMemoryUsers) {
        if (toLower(pair.second.email) == lowerEmail) {
            return pair.second;
        }
    }
    return {};
#endif
}

bool UserRepository::emailExists(const std::string &email)
{
    return bool(getUserByEmail(email));
}

bool UserRepository::updateProfile(int userId, const std::string &fullName, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE users SET full_name = :name, account_status = :status, updated_at = datetime('now') "
        "WHERE user_id = :id;"
    ));
    query.bindValue(QStringLiteral(":name"), QString::fromStdString(fullName));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status));
    query.bindValue(QStringLiteral(":id"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    auto it = s_inMemoryUsers.find(userId);
    if (it != s_inMemoryUsers.end()) {
        it->second.fullName = fullName;
        if (!status.empty()) it->second.accountStatus = status;
        it->second.updatedAt = "2026-10-27 10:05:00";
        return true;
    }
    return false;
#endif
}

bool UserRepository::updatePassword(int userId, const std::string &newPasswordHash)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE users SET password_hash = :hash, updated_at = datetime('now') "
        "WHERE user_id = :id;"
    ));
    query.bindValue(QStringLiteral(":hash"), QString::fromStdString(newPasswordHash));
    query.bindValue(QStringLiteral(":id"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    auto it = s_inMemoryUsers.find(userId);
    if (it != s_inMemoryUsers.end()) {
        it->second.passwordHash = newPasswordHash;
        it->second.updatedAt = "2026-10-27 10:05:00";
        return true;
    }
    return false;
#endif
}

std::vector<Models::User> UserRepository::getAllUsers()
{
    std::vector<Models::User> results;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(QStringLiteral(
        "SELECT user_id, full_name, email, password_hash, account_status, created_at, updated_at FROM users;"
    ), db);

    while (query.next()) {
        Models::User u;
        u.id = query.value(0).toInt();
        u.fullName = query.value(1).toString().toStdString();
        u.email = query.value(2).toString().toStdString();
        u.passwordHash = query.value(3).toString().toStdString();
        u.accountStatus = query.value(4).toString().toStdString();
        u.createdAt = query.value(5).toString().toStdString();
        u.updatedAt = query.value(6).toString().toStdString();
        results.push_back(u);
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_userStoreMutex);
    for (const auto &pair : s_inMemoryUsers) {
        results.push_back(pair.second);
    }
#endif
    return results;
}

} // namespace Database
} // namespace InvestSphere

#ifndef INVESTSPHERE_SESSIONMANAGER_H
#define INVESTSPHERE_SESSIONMANAGER_H

#include "UserModel.h"
#include "Compat.h"
#include <string>
#include <memory>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QObject>
#endif

namespace InvestSphere {
namespace App {

/**
 * @brief Thread-safe in-memory session manager for authenticated users.
 * Enforces multi-user isolation by maintaining current active identity
 * and clearing user-specific state upon logout.
 */
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
class SessionManager : public QObject {
    Q_OBJECT
#else
class SessionManager {
#endif

public:
    static SessionManager& instance();

    bool isAuthenticated() const;
    int currentUserId() const;
    std::string currentUserName() const;
    std::string currentUserEmail() const;
    std::string currentAccountStatus() const;
    std::string currentCreatedAt() const;

    void startSession(const Models::User &user);
    void updateProfile(const std::string &fullName);
    void logout();

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
signals:
    void sessionStarted(int userId);
    void sessionEnded();
    void profileUpdated(const QString &fullName);
#endif

private:
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    explicit SessionManager(QObject *parent = nullptr);
#else
    SessionManager();
#endif
    ~SessionManager() = default;
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    mutable InvestSphere::Mutex m_mutex;
    bool m_authenticated{false};
    int m_userId{0};
    std::string m_fullName;
    std::string m_email;
    std::string m_accountStatus;
    std::string m_createdAt;
};

} // namespace App
} // namespace InvestSphere

#endif // INVESTSPHERE_SESSIONMANAGER_H

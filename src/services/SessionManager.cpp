#include "SessionManager.h"

namespace InvestSphere {
namespace App {

SessionManager& SessionManager::instance()
{
    static SessionManager s_instance;
    return s_instance;
}

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
{
}
#else
SessionManager::SessionManager()
{
}
#endif

bool SessionManager::isAuthenticated() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_authenticated;
}

int SessionManager::currentUserId() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_userId;
}

std::string SessionManager::currentUserName() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_fullName;
}

std::string SessionManager::currentUserEmail() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_email;
}

std::string SessionManager::currentAccountStatus() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_accountStatus;
}

std::string SessionManager::currentCreatedAt() const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
    return m_createdAt;
}

void SessionManager::startSession(const Models::User &user)
{
    {
        InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
        m_authenticated = true;
        m_userId = user.id;
        m_fullName = user.fullName;
        m_email = user.email;
        m_accountStatus = user.accountStatus;
        m_createdAt = user.createdAt;
    }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    emit sessionStarted(user.id);
#endif
}

void SessionManager::updateProfile(const std::string &fullName)
{
    {
        InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
        m_fullName = fullName;
    }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    emit profileUpdated(QString::fromStdString(fullName));
#endif
}

void SessionManager::logout()
{
    {
        InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_mutex);
        m_authenticated = false;
        m_userId = 0;
        m_fullName.clear();
        m_email.clear();
        m_accountStatus.clear();
        m_createdAt.clear();
    }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    emit sessionEnded();
#endif
}

} // namespace App
} // namespace InvestSphere

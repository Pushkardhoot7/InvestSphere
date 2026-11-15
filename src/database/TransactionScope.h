#ifndef TRANSACTIONSCOPE_H
#define TRANSACTIONSCOPE_H

#include "DatabaseManager.h"

namespace InvestSphere {
namespace Database {

/**
 * @brief RAII Transaction Scope guard ensuring atomic execution and automatic rollback on failure.
 */
class TransactionScope {
public:
    explicit TransactionScope(DatabaseManager &db = DatabaseManager::instance())
        : m_db(db), m_isActive(false), m_isCommitted(false)
    {
        m_isActive = m_db.beginTransaction();
    }

    ~TransactionScope()
    {
        if (m_isActive && !m_isCommitted) {
            m_db.rollbackTransaction();
        }
    }

    // Disable copy
    TransactionScope(const TransactionScope&) = delete;
    TransactionScope& operator=(const TransactionScope&) = delete;

    // Move semantics
    TransactionScope(TransactionScope &&other) noexcept
        : m_db(other.m_db), m_isActive(other.m_isActive), m_isCommitted(other.m_isCommitted)
    {
        other.m_isActive = false;
        other.m_isCommitted = true;
    }

    bool commit()
    {
        if (m_isActive && !m_isCommitted) {
            m_isCommitted = m_db.commitTransaction();
            m_isActive = false;
            return m_isCommitted;
        }
        return false;
    }

    void rollback()
    {
        if (m_isActive && !m_isCommitted) {
            m_db.rollbackTransaction();
            m_isActive = false;
            m_isCommitted = false;
        }
    }

    bool isActive() const { return m_isActive; }
    bool isCommitted() const { return m_isCommitted; }

private:
    DatabaseManager &m_db;
    bool m_isActive{false};
    bool m_isCommitted{false};
};

} // namespace Database
} // namespace InvestSphere

#endif // TRANSACTIONSCOPE_H

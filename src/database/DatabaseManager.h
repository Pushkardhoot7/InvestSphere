#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QString>
#endif

namespace InvestSphere {
namespace Database {

struct MigrationInfo {
    int version{0};
    std::string description;
    std::string appliedAt;
};

class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Lifecycle
    bool initialize(const std::string &databasePath = "");
    void close();
    bool isOpen() const;
    bool isNewDatabase() const { return m_isNewDatabase; }

    // Schema & Migrations
    bool initializeSchema(const std::string &schemaSqlPath = "sql/schema.sql");
    bool initializeViews(const std::string &viewsSqlPath = "sql/views.sql");
    bool initializeTriggers(const std::string &triggersSqlPath = "sql/triggers.sql");
    bool loadSeedData(const std::string &seedSqlPath = "sql/seed_data.sql");

    int getCurrentSchemaVersion() const;
    bool applyMigration(int targetVersion, const std::string &description, const std::string &sqlScript);
    std::vector<MigrationInfo> getAppliedMigrations() const;

    // Table verification
    bool verifyCoreTablesExist() const;

    // Transaction Management (ACID)
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Query Execution
    bool executeBatchScript(const std::string &sqlContent);
    bool executeFile(const std::string &filePath);

    // Database metadata
    std::string getDatabasePath() const { return m_databasePath; }
    std::string getLastError() const { return m_lastError; }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlDatabase getQtDatabase() const;
    QSqlQuery prepareQuery(const QString &queryStr) const;
#endif

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    std::string resolveDefaultDatabasePath() const;
    bool configurePragmas();

    std::string m_databasePath;
    std::string m_lastError;
    bool m_isOpen{false};
    bool m_isNewDatabase{false};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QString m_connectionName;
#endif
};

} // namespace Database
} // namespace InvestSphere

#endif // DATABASEMANAGER_H

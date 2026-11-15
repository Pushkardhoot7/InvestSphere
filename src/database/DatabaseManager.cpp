#include "DatabaseManager.h"
#include "Compat.h"

#include <fstream>
#include <sstream>
#include <iostream>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#endif

namespace InvestSphere {
namespace Database {

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager s_instance;
    return s_instance;
}

DatabaseManager::DatabaseManager()
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    : m_connectionName(QStringLiteral("InvestSphereConnection"))
#endif
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

std::string DatabaseManager::resolveDefaultDatabasePath() const
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    return dir.filePath(QStringLiteral("investsphere.db")).toStdString();
#else
    return "investsphere.db";
#endif
}

bool DatabaseManager::initialize(const std::string &databasePath)
{
    m_databasePath = databasePath.empty() ? resolveDefaultDatabasePath() : databasePath;

    // Ensure parent directory exists
    InvestSphere::Utils::ensureDirectoryExists(m_databasePath);
    {
        std::ifstream checkFile(m_databasePath, std::ios::binary | std::ios::ate);
        m_isNewDatabase = !checkFile.is_open() || (checkFile.tellg() == 0);
    }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            QSqlDatabase existingDb = QSqlDatabase::database(m_connectionName);
            if (existingDb.isOpen()) {
                existingDb.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(QString::fromStdString(m_databasePath));

    if (!db.open()) {
        m_lastError = db.lastError().text().toStdString();
        m_isOpen = false;
        std::cerr << "[DatabaseManager] Failed to open SQLite database at: " 
                  << m_databasePath << " | Error: " << m_lastError << std::endl;
        return false;
    }

    m_isOpen = true;
    if (!configurePragmas()) {
        return false;
    }

    // If new database, initialize schema
    if (m_isNewDatabase) {
        std::cout << "[DatabaseManager] Brand new database detected. Initializing schema..." << std::endl;
        if (!initializeSchema() || !initializeViews() || !initializeTriggers()) {
            std::cerr << "[DatabaseManager] Schema initialization failed: " << m_lastError << std::endl;
            return false;
        }
    } else {
        std::cout << "[DatabaseManager] Existing database detected. Preserving data (Current Version: "
                  << getCurrentSchemaVersion() << ")..." << std::endl;
    }

    if (!verifyCoreTablesExist()) {
        std::cerr << "[DatabaseManager] Core table verification failed!" << std::endl;
        return false;
    }

    std::cout << "[DatabaseManager] Connected and verified SQLite database at: " 
              << m_databasePath << std::endl;
    return true;
#else
    m_isOpen = true;
    std::cout << "[DatabaseManager] Initialized database configuration for: " 
              << m_databasePath << " (Foundation C++ mode | IsNew: " 
              << (m_isNewDatabase ? "YES" : "NO") << ")" << std::endl;
    return true;
#endif
}

bool DatabaseManager::configurePragmas()
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());

    // 1. Mandatory Foreign Key Enforcement
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys = ON;"))) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }

    // 2. Write-Ahead Logging (WAL) Mode
    if (!query.exec(QStringLiteral("PRAGMA journal_mode = WAL;"))) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }

    // 3. Normal Synchronous Mode
    if (!query.exec(QStringLiteral("PRAGMA synchronous = NORMAL;"))) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }

    // 4. Busy Timeout 5000ms
    if (!query.exec(QStringLiteral("PRAGMA busy_timeout = 5000;"))) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }

    return true;
#else
    return true;
#endif
}

void DatabaseManager::close()
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    if (m_isOpen) {
        {
            QSqlDatabase db = getQtDatabase();
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
#endif
    m_isOpen = false;
}

bool DatabaseManager::isOpen() const
{
    return m_isOpen;
}

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
QSqlDatabase DatabaseManager::getQtDatabase() const
{
    return QSqlDatabase::database(m_connectionName);
}

QSqlQuery DatabaseManager::prepareQuery(const QString &queryStr) const
{
    QSqlQuery query(getQtDatabase());
    query.prepare(queryStr);
    return query;
}
#endif

bool DatabaseManager::beginTransaction()
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    return getQtDatabase().transaction();
#else
    return true;
#endif
}

bool DatabaseManager::commitTransaction()
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    return getQtDatabase().commit();
#else
    return true;
#endif
}

bool DatabaseManager::rollbackTransaction()
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    return getQtDatabase().rollback();
#else
    return true;
#endif
}

bool DatabaseManager::executeFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        m_lastError = "Could not open SQL file: " + filePath;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return executeBatchScript(buffer.str());
}

bool DatabaseManager::executeBatchScript(const std::string &sqlContent)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());
    QString content = QString::fromStdString(sqlContent);

    // Split on semicolons
    QStringList statements = content.split(QStringLiteral(";"), Qt::SkipEmptyParts);

    for (QString &stmt : statements) {
        QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        if (!query.exec(trimmed)) {
            m_lastError = query.lastError().text().toStdString();
            std::cerr << "[DatabaseManager] SQL execution failure:\n"
                      << trimmed.toStdString() << "\nError: " << m_lastError << std::endl;
            return false;
        }
    }
    return true;
#else
    (void)sqlContent;
    return true;
#endif
}

bool DatabaseManager::initializeSchema(const std::string &schemaSqlPath)
{
    return executeFile(schemaSqlPath);
}

bool DatabaseManager::initializeViews(const std::string &viewsSqlPath)
{
    return executeFile(viewsSqlPath);
}

bool DatabaseManager::initializeTriggers(const std::string &triggersSqlPath)
{
    return executeFile(triggersSqlPath);
}

bool DatabaseManager::loadSeedData(const std::string &seedSqlPath)
{
    return executeFile(seedSqlPath);
}

bool DatabaseManager::verifyCoreTablesExist() const
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());
    const char *requiredTables[] = {
        "users", "investment_types", "instruments", "investments", 
        "transactions", "schema_migrations", "audit_logs"
    };

    for (const char *tbl : requiredTables) {
        query.prepare(QStringLiteral("SELECT count(*) FROM sqlite_master WHERE type='table' AND name=:name;"));
        query.bindValue(QStringLiteral(":name"), QString::fromUtf8(tbl));
        if (!query.exec() || !query.next() || query.value(0).toInt() == 0) {
            std::cerr << "[DatabaseManager] Required table missing: " << tbl << std::endl;
            return false;
        }
    }
    return true;
#else
    return true;
#endif
}

int DatabaseManager::getCurrentSchemaVersion() const
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());
    if (query.exec(QStringLiteral("SELECT MAX(version) FROM schema_migrations;"))) {
        if (query.next() && !query.value(0).isNull()) {
            return query.value(0).toInt();
        }
    }
    return 0;
#else
    return 1;
#endif
}

bool DatabaseManager::applyMigration(int targetVersion, const std::string &description, const std::string &sqlScript)
{
    beginTransaction();
    if (!executeBatchScript(sqlScript)) {
        rollbackTransaction();
        return false;
    }

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());
    query.prepare(QStringLiteral("INSERT INTO schema_migrations (version, description) VALUES (:v, :desc);"));
    query.bindValue(QStringLiteral(":v"), targetVersion);
    query.bindValue(QStringLiteral(":desc"), QString::fromStdString(description));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        rollbackTransaction();
        return false;
    }
#else
    (void)targetVersion;
    (void)description;
#endif

    return commitTransaction();
}

std::vector<MigrationInfo> DatabaseManager::getAppliedMigrations() const
{
    std::vector<MigrationInfo> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QSqlQuery query(getQtDatabase());
    if (query.exec(QStringLiteral("SELECT version, description, applied_at FROM schema_migrations ORDER BY version ASC;"))) {
        while (query.next()) {
            MigrationInfo info;
            info.version = query.value(0).toInt();
            info.description = query.value(1).toString().toStdString();
            info.appliedAt = query.value(2).toString().toStdString();
            list.push_back(info);
        }
    }
#endif
    return list;
}

} // namespace Database
} // namespace InvestSphere

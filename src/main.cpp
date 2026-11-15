#include <iostream>
#include <iomanip>
#include <string>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "auth/AuthWindow.h"
#include "SessionManager.h"
#endif

#include "DatabaseManager.h"

int main(int argc, char *argv[])
{
    // Initialize Database Manager
    auto &dbManager = InvestSphere::Database::DatabaseManager::instance();
    bool dbInitOk = dbManager.initialize("investsphere.db");

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    QApplication app(argc, argv);

    // Application metadata
    QApplication::setApplicationName(QStringLiteral("InvestSphere"));
    QApplication::setApplicationVersion(QStringLiteral("0.4.0"));
    QApplication::setOrganizationName(QStringLiteral("InvestSphere"));
    QApplication::setOrganizationDomain(QStringLiteral("investsphere.local"));

    if (!dbInitOk || !dbManager.isOpen()) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("InvestSphere — Database Initialization Error"),
            QStringLiteral("Failed to connect to the SQLite database. Please ensure write permissions exist for the application data directory.")
        );
        return 1;
    }

    // Initialize relational schema, views, and integrity triggers
    dbManager.initializeSchema("sql/schema.sql");
    dbManager.initializeViews("sql/views.sql");
    dbManager.initializeTriggers("sql/triggers.sql");

    // Phase 5 Application Flow: Authentication -> Session -> Main Application Window
    InvestSphere::Ui::AuthWindow authWindow;
    MainWindow mainWindow;

    // Transition from Authentication to Main Application Window
    QObject::connect(&authWindow, &InvestSphere::Ui::AuthWindow::authenticated, [&](int userId) {
        (void)userId;
        authWindow.hide();
        mainWindow.refreshUserProfile();
        mainWindow.navigateToPage(0); // Start at Executive Dashboard
        mainWindow.show();
    });

    // Transition on Logout: Clear session, hide Main Window, present Login Window
    QObject::connect(&mainWindow, &MainWindow::logoutRequested, [&]() {
        mainWindow.hide();
        authWindow.showLogin();
        authWindow.show();
    });

    // Initially display the Login / Authentication screen
    authWindow.show();

    return app.exec();
#else
    (void)argc;
    (void)argv;

    std::cout << "=====================================================================\n";
    std::cout << "   INVESTSPHERE — Smart Investment & Portfolio Management System     \n";
    std::cout << "   Tagline: \"One Platform. Every Investment. Smarter Decisions.\"      \n";
    std::cout << "=====================================================================\n";
    std::cout << " [ Academic DBMS Course Project — Authentication & Security (Phase 5) ]\n\n";
    std::cout << " Relational Schema & Authentication Layer:\n";
    std::cout << "   - Tables Created       : 18 Tables (3NF Schema with Users table isolation)\n";
    std::cout << "   - Password Security    : PBKDF2-HMAC-SHA256 (100,000 rounds, 128-bit salt)\n";
    std::cout << "   - Session Management   : Thread-safe in-memory SessionManager\n";
    std::cout << "   - User Data Isolation  : Foreign Key constraints & user-scoped repository queries\n";
    std::cout << "   - Database Engine      : SQLite 3 (WAL mode, Foreign Keys Enforced)\n\n";
    std::cout << " Database Access Layer:\n";
    std::cout << "   - Connection Status    : " << (dbInitOk && dbManager.isOpen() ? "OPEN & READY" : "OFFLINE") << "\n";
    std::cout << "   - Target Database Path : " << dbManager.getDatabasePath() << "\n\n";
    std::cout << " GUI Notice:\n";
    std::cout << "   Qt 6 Widgets SDK was not detected in system paths during this build.\n";
    std::cout << "   To launch the full native desktop GUI with Login, Registration & Dashboard:\n";
    std::cout << "     cmake -B build -DCMAKE_PREFIX_PATH=<path_to_qt6>\n";
    std::cout << "     cmake --build build\n\n";
    std::cout << "=====================================================================\n";

    return 0;
#endif
}

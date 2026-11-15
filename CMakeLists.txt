cmake_minimum_required(VERSION 3.16)

project(InvestSphere
    VERSION 0.1.0
    DESCRIPTION "InvestSphere — Smart Investment & Portfolio Management System"
    LANGUAGES CXX
)

# C++ Standard Configuration (C++17 or newer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE indexing / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directory structure
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/mainwindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/dashboard
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/portfolio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/stocks
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/mutualfunds
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/gold
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/fixeddeposits
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/bonds
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/transactions
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/market
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/calculators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/maturity
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/analytics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/reports
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/settings
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/auth
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
    ${CMAKE_CURRENT_SOURCE_DIR}/src/finance
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Search for Qt 6 components
find_package(Qt6 COMPONENTS Widgets Sql QUIET)
find_package(Qt6 COMPONENTS Charts QUIET)

set(INVESTSPHERE_DB_SOURCES
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/database/TransactionScope.h
    src/database/UserRepository.cpp
    src/database/UserRepository.h
    src/database/InvestmentRepository.cpp
    src/database/InvestmentRepository.h
    src/database/TransactionRepository.cpp
    src/database/TransactionRepository.h
    src/database/SipRepository.cpp
    src/database/SipRepository.h
    src/database/SwpRepository.cpp
    src/database/SwpRepository.h
    src/database/FinancialEventRepository.cpp
    src/database/FinancialEventRepository.h
    src/database/PortfolioSnapshotRepository.cpp
    src/database/PortfolioSnapshotRepository.h
    src/database/ReportRepository.cpp
    src/database/ReportRepository.h
    src/models/UserModel.h
    src/models/InstrumentModel.h
    src/models/InvestmentModel.h
    src/models/TransactionModel.h
    src/models/SipPlanModel.h
    src/models/SwpPlanModel.h
    src/models/FinancialEventModel.h
)

set(INVESTSPHERE_AUTH_SOURCES
    src/services/PasswordHasher.cpp
    src/services/PasswordHasher.h
    src/services/SessionManager.cpp
    src/services/SessionManager.h
    src/services/AuthenticationService.cpp
    src/services/AuthenticationService.h
)

set(INVESTSPHERE_FINANCE_SOURCES
    src/finance/CostBasisEngine.cpp
    src/finance/CostBasisEngine.h
    src/finance/CompoundingEngine.cpp
    src/finance/CompoundingEngine.h
    src/finance/AnnuityEngine.cpp
    src/finance/AnnuityEngine.h
)

set(INVESTSPHERE_SERVICES_SOURCES
    src/services/InvestmentService.cpp
    src/services/InvestmentService.h
    src/services/DashboardService.cpp
    src/services/DashboardService.h
    src/services/PortfolioCalculationService.cpp
    src/services/PortfolioCalculationService.h
    src/services/TransactionService.cpp
    src/services/TransactionService.h
    src/services/AnalyticsService.cpp
    src/services/AnalyticsService.h
    src/services/MarketDataProvider.cpp
    src/services/MarketDataProvider.h
    src/services/MarketDataService.cpp
    src/services/MarketDataService.h
    src/services/MaturityService.cpp
    src/services/MaturityService.h
    src/services/CsvExportService.cpp
    src/services/CsvExportService.h
    src/services/ReportService.cpp
    src/services/ReportService.h
)

set(INVESTSPHERE_UI_SOURCES
    src/ui/common/Theme.cpp
    src/ui/common/Theme.h
    src/ui/common/IconHelper.cpp
    src/ui/common/IconHelper.h
    src/ui/common/SummaryCard.cpp
    src/ui/common/SummaryCard.h
    src/ui/common/SectionHeader.cpp
    src/ui/common/SectionHeader.h
    src/ui/common/SearchField.cpp
    src/ui/common/SearchField.h
    src/ui/common/StatusBadge.cpp
    src/ui/common/StatusBadge.h
    src/ui/common/EmptyStateWidget.cpp
    src/ui/common/EmptyStateWidget.h
    src/ui/common/FilterBar.cpp
    src/ui/common/FilterBar.h
    src/ui/common/ConfirmDialog.cpp
    src/ui/common/ConfirmDialog.h
    src/ui/common/NotificationBanner.cpp
    src/ui/common/NotificationBanner.h
    src/ui/common/ChartPlaceholder.cpp
    src/ui/common/ChartPlaceholder.h
    src/ui/common/ModernTableWidget.cpp
    src/ui/common/ModernTableWidget.h
    src/ui/mainwindow/MainWindow.cpp
    src/ui/mainwindow/MainWindow.h
    src/ui/auth/LoginWidget.cpp
    src/ui/auth/LoginWidget.h
    src/ui/auth/RegisterWidget.cpp
    src/ui/auth/RegisterWidget.h
    src/ui/auth/AuthWindow.cpp
    src/ui/auth/AuthWindow.h
    src/ui/dashboard/DashboardPage.cpp
    src/ui/dashboard/DashboardPage.h
    src/ui/portfolio/PortfolioPage.cpp
    src/ui/portfolio/PortfolioPage.h
    src/ui/portfolio/InvestmentDialog.cpp
    src/ui/portfolio/InvestmentDialog.h
    src/ui/portfolio/InvestmentDetailsDialog.cpp
    src/ui/portfolio/InvestmentDetailsDialog.h
    src/ui/stocks/StocksPage.cpp
    src/ui/stocks/StocksPage.h
    src/ui/mutualfunds/MutualFundsPage.cpp
    src/ui/mutualfunds/MutualFundsPage.h
    src/ui/gold/GoldPage.cpp
    src/ui/gold/GoldPage.h
    src/ui/fixeddeposits/FixedDepositsPage.cpp
    src/ui/fixeddeposits/FixedDepositsPage.h
    src/ui/bonds/BondsPage.cpp
    src/ui/bonds/BondsPage.h
    src/ui/transactions/TransactionsPage.cpp
    src/ui/transactions/TransactionsPage.h
    src/ui/market/MarketWatchPage.cpp
    src/ui/market/MarketWatchPage.h
    src/ui/calculators/SipCalculatorPage.cpp
    src/ui/calculators/SipCalculatorPage.h
    src/ui/calculators/SwpCalculatorPage.cpp
    src/ui/calculators/SwpCalculatorPage.h
    src/ui/maturity/MaturityTrackerPage.cpp
    src/ui/maturity/MaturityTrackerPage.h
    src/ui/analytics/AnalyticsPage.cpp
    src/ui/analytics/AnalyticsPage.h
    src/ui/reports/ReportsPage.cpp
    src/ui/reports/ReportsPage.h
    src/ui/settings/SettingsPage.cpp
    src/ui/settings/SettingsPage.h
)

if(Qt6_FOUND)
    # Enable Qt standard automatic code generation tools
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    message(STATUS "--------------------------------------------------------")
    message(STATUS "InvestSphere: Qt 6 FOUND (Version: ${Qt6_VERSION})")
    message(STATUS "  - Qt6Widgets: FOUND")
    message(STATUS "  - Qt6Sql:     FOUND")
    if(Qt6Charts_FOUND)
        message(STATUS "  - Qt6Charts:  FOUND")
    else()
        message(STATUS "  - Qt6Charts:  NOT FOUND (Optional charts fallback active)")
    endif()
    message(STATUS "--------------------------------------------------------")

    set(INVESTSPHERE_SOURCES
        src/main.cpp
        ${INVESTSPHERE_AUTH_SOURCES}
        ${INVESTSPHERE_FINANCE_SOURCES}
        ${INVESTSPHERE_SERVICES_SOURCES}
        ${INVESTSPHERE_UI_SOURCES}
        ${INVESTSPHERE_DB_SOURCES}
    )

    add_executable(InvestSphere ${INVESTSPHERE_SOURCES})

    set_target_properties(InvestSphere PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    target_compile_definitions(InvestSphere PRIVATE
        INVESTSPHERE_HAS_QT=1
        INVESTSPHERE_APP_NAME="InvestSphere"
        INVESTSPHERE_APP_VERSION="0.6.0"
        INVESTSPHERE_TAGLINE="One Platform. Every Investment. Smarter Decisions."
    )

    target_link_libraries(InvestSphere PRIVATE
        Qt6::Widgets
        Qt6::Sql
    )

    if(Qt6Charts_FOUND)
        target_compile_definitions(InvestSphere PRIVATE INVESTSPHERE_HAS_CHARTS=1)
        target_link_libraries(InvestSphere PRIVATE Qt6::Charts)
    endif()

else()
    message(STATUS "--------------------------------------------------------")
    message(STATUS "InvestSphere: Qt 6 NOT DETECTED in current system paths.")
    message(STATUS "To build the full native desktop GUI:")
    message(STATUS "  1. Install Qt 6.x (Widgets, SQL) via Qt Online Installer or vcpkg.")
    message(STATUS "  2. Configure CMake with -DCMAKE_PREFIX_PATH=<Qt6_Installation_Path>")
    message(STATUS "Building the core InvestSphere architecture & foundation module.")
    message(STATUS "--------------------------------------------------------")

    set(INVESTSPHERE_SOURCES
        src/main.cpp
        ${INVESTSPHERE_AUTH_SOURCES}
        ${INVESTSPHERE_FINANCE_SOURCES}
        ${INVESTSPHERE_SERVICES_SOURCES}
        ${INVESTSPHERE_DB_SOURCES}
    )

    add_executable(InvestSphere ${INVESTSPHERE_SOURCES})

    target_compile_definitions(InvestSphere PRIVATE
        INVESTSPHERE_HAS_QT=0
        INVESTSPHERE_APP_NAME="InvestSphere"
        INVESTSPHERE_APP_VERSION="0.6.0"
        INVESTSPHERE_TAGLINE="One Platform. Every Investment. Smarter Decisions."
    )
endif()

# Compiler-specific warnings
if(MSVC)
    target_compile_options(InvestSphere PRIVATE /W4 /permissive-)
else()
    target_compile_options(InvestSphere PRIVATE -Wall -Wextra -pedantic)
endif()

# Enable Testing and Database Test Executable
enable_testing()

add_executable(test_database
    tests/test_database.cpp
    ${INVESTSPHERE_DB_SOURCES}
)

if(Qt6_FOUND)
    target_compile_definitions(test_database PRIVATE INVESTSPHERE_HAS_QT=1)
    target_link_libraries(test_database PRIVATE Qt6::Sql)
else()
    target_compile_definitions(test_database PRIVATE INVESTSPHERE_HAS_QT=0)
endif()

if(MSVC)
    target_compile_options(test_database PRIVATE /W4)
else()
    target_compile_options(test_database PRIVATE -Wall -Wextra)
endif()

add_test(NAME DatabaseManagerTest COMMAND test_database WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Authentication & Security Test Executable
add_executable(test_authentication
    tests/test_authentication.cpp
    ${INVESTSPHERE_AUTH_SOURCES}
    ${INVESTSPHERE_DB_SOURCES}
)

if(Qt6_FOUND)
    target_compile_definitions(test_authentication PRIVATE INVESTSPHERE_HAS_QT=1)
    target_link_libraries(test_authentication PRIVATE Qt6::Sql)
else()
    target_compile_definitions(test_authentication PRIVATE INVESTSPHERE_HAS_QT=0)
endif()

if(MSVC)
    target_compile_options(test_authentication PRIVATE /W4)
else()
    target_compile_options(test_authentication PRIVATE -Wall -Wextra)
endif()

add_test(NAME AuthenticationTest COMMAND test_authentication WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Phase 6 Investment Management & Dashboard Test Executable
add_executable(test_phase6_investments
    tests/test_phase6_investments.cpp
    ${INVESTSPHERE_FINANCE_SOURCES}
    ${INVESTSPHERE_SERVICES_SOURCES}
    ${INVESTSPHERE_AUTH_SOURCES}
    ${INVESTSPHERE_DB_SOURCES}
)

if(Qt6_FOUND)
    target_compile_definitions(test_phase6_investments PRIVATE INVESTSPHERE_HAS_QT=1)
    target_link_libraries(test_phase6_investments PRIVATE Qt6::Sql)
else()
    target_compile_definitions(test_phase6_investments PRIVATE INVESTSPHERE_HAS_QT=0)
endif()

if(MSVC)
    target_compile_options(test_phase6_investments PRIVATE /W4)
else()
    target_compile_options(test_phase6_investments PRIVATE -Wall -Wextra)
endif()

add_test(NAME Phase6InvestmentsTest COMMAND test_phase6_investments WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Phase 7 Transaction Management & Portfolio Calculations Test Executable
add_executable(test_phase7_transactions
    tests/test_phase7_transactions.cpp
    ${INVESTSPHERE_FINANCE_SOURCES}
    ${INVESTSPHERE_SERVICES_SOURCES}
    ${INVESTSPHERE_AUTH_SOURCES}
    ${INVESTSPHERE_DB_SOURCES}
)

if(Qt6_FOUND)
    target_compile_definitions(test_phase7_transactions PRIVATE INVESTSPHERE_HAS_QT=1)
    target_link_libraries(test_phase7_transactions PRIVATE Qt6::Sql)
else()
    target_compile_definitions(test_phase7_transactions PRIVATE INVESTSPHERE_HAS_QT=0)
endif()

if(MSVC)
    target_compile_options(test_phase7_transactions PRIVATE /W4)
else()
    target_compile_options(test_phase7_transactions PRIVATE -Wall -Wextra)
endif()

add_test(NAME Phase7TransactionsTest COMMAND test_phase7_transactions WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Phase 8 SIP/SWP Calculators & Portfolio Analytics Test Executable
add_executable(test_phase8_analytics
    tests/test_phase8_analytics.cpp
    ${INVESTSPHERE_FINANCE_SOURCES}
    ${INVESTSPHERE_SERVICES_SOURCES}
    ${INVESTSPHERE_AUTH_SOURCES}
    ${INVESTSPHERE_DB_SOURCES}
)

if(Qt6_FOUND)
    target_compile_definitions(test_phase8_analytics PRIVATE INVESTSPHERE_HAS_QT=1)
    target_link_libraries(test_phase8_analytics PRIVATE Qt6::Sql)
else()
    target_compile_definitions(test_phase8_analytics PRIVATE INVESTSPHERE_HAS_QT=0)
endif()

if(MSVC)
    target_compile_options(test_phase8_analytics PRIVATE /W4)
else()
    target_compile_options(test_phase8_analytics PRIVATE -Wall -Wextra)
endif()

add_test(NAME Phase8AnalyticsTest COMMAND test_phase8_analytics WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

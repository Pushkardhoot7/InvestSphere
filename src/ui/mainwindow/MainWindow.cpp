#include "MainWindow.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QApplication>
#include <QScreen>
#include <QStatusBar>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>

#include "Theme.h"
#include "IconHelper.h"
#include "SearchField.h"
#include "ConfirmDialog.h"
#include "NotificationBanner.h"
#include "SessionManager.h"

#include "dashboard/DashboardPage.h"
#include "portfolio/PortfolioPage.h"
#include "portfolio/InvestmentDialog.h"
#include "stocks/StocksPage.h"
#include "mutualfunds/MutualFundsPage.h"
#include "gold/GoldPage.h"
#include "fixeddeposits/FixedDepositsPage.h"
#include "bonds/BondsPage.h"
#include "transactions/TransactionsPage.h"
#include "transactions/TransactionDialog.h"
#include "market/MarketWatchPage.h"
#include "calculators/SipCalculatorPage.h"
#include "calculators/SwpCalculatorPage.h"
#include "maturity/MaturityTrackerPage.h"
#include "analytics/AnalyticsPage.h"
#include "reports/ReportsPage.h"
#include "settings/SettingsPage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Apply global centralized design system stylesheet
    qApp->setStyleSheet(InvestSphere::Ui::Theme::getApplicationStyleSheet());
    setupUi();

    // Listen to session changes
    connect(&InvestSphere::App::SessionManager::instance(), &InvestSphere::App::SessionManager::profileUpdated,
            this, &MainWindow::refreshUserProfile);
    connect(&InvestSphere::App::SessionManager::instance(), &InvestSphere::App::SessionManager::sessionStarted,
            this, &MainWindow::refreshUserProfile);

    refreshUserProfile();
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("InvestSphere — Smart Investment & Portfolio Management System"));
    setMinimumSize(1080, 700);
    resize(1280, 820);

    // Metadata for the 15 managed pages
    m_pageMetadata = {
        {QStringLiteral("Executive Dashboard"), QStringLiteral("Overview / Real-time portfolio valuation and allocation summary")},
        {QStringLiteral("My Portfolio"), QStringLiteral("Overview / Multi-asset holding positions and aggregate asset mix")},
        {QStringLiteral("Stock Equities"), QStringLiteral("Investments / Direct equity holdings, day changes, and capital gains")},
        {QStringLiteral("Mutual Funds"), QStringLiteral("Investments / Fund schemes, active SIP plans, and NAV valuation")},
        {QStringLiteral("Gold Holdings"), QStringLiteral("Investments / Physical bullion, Sovereign Gold Bonds (SGB), and digital gold")},
        {QStringLiteral("Fixed Deposits"), QStringLiteral("Investments / Bank & corporate term deposits, compounding, and maturities")},
        {QStringLiteral("Bonds & Debt"), QStringLiteral("Investments / Government securities, corporate bonds, and coupon schedules")},
        {QStringLiteral("Transactions Ledger"), QStringLiteral("Activity / Comprehensive double-entry financial audit trail")},
        {QStringLiteral("Market Watch"), QStringLiteral("Activity / Real-time market indices, watchlist quotes, and price movements")},
        {QStringLiteral("SIP Calculator"), QStringLiteral("Financial Tools / Systematic investment wealth compounding simulator")},
        {QStringLiteral("SWP Calculator"), QStringLiteral("Financial Tools / Systematic retirement cash outflow & corpus sustainability")},
        {QStringLiteral("Maturity Tracker"), QStringLiteral("Financial Tools / Upcoming redemption dates, bond coupons, and liquidity inflows")},
        {QStringLiteral("Portfolio Analytics"), QStringLiteral("Insights / Risk-adjusted performance metrics, Sharpe ratio, and sector exposure")},
        {QStringLiteral("Financial Reports"), QStringLiteral("Insights / Capital gains tax statements (STCG/LTCG) and holding exports")},
        {QStringLiteral("System Settings"), QStringLiteral("Account / User profile, database diagnostics, and system preferences")}
    };

    // Central layout
    auto *centralWidget = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Left Navigation Sidebar
    QWidget *sidebar = createSidebar();
    rootLayout->addWidget(sidebar);

    // Right-side dynamic container (Header + Notification Banner + Stacked Content)
    auto *rightContainer = new QWidget(centralWidget);
    rightContainer->setStyleSheet(QStringLiteral("background-color: %1;").arg(InvestSphere::Ui::Theme::ColorMainBackground));
    auto *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *header = createHeader();
    rightLayout->addWidget(header);

    // Inline notification banner for global system notices
    m_topNotificationBanner = new InvestSphere::Ui::NotificationBanner(QString(), InvestSphere::Ui::NotificationBanner::BannerType::Info, true, rightContainer);
    rightLayout->addWidget(m_topNotificationBanner);

    QWidget *content = createContentArea();
    rightLayout->addWidget(content, 1);

    rootLayout->addWidget(rightContainer, 1);
    setCentralWidget(centralWidget);

    // Status bar setup
    auto *status = statusBar();
    status->setStyleSheet(QStringLiteral(
        "QStatusBar { background-color: #FFFFFF; color: #64748B; border-top: 1px solid #E5E7EB; font-size: 11px; padding: 4px 12px; }"
    ));
    status->showMessage(QStringLiteral("InvestSphere v0.4.0 | SQLite 3 Relational Foundation (WAL Mode) | Authenticated Session"));

    // Default to Dashboard (index 0)
    navigateToPage(0);
}

void MainWindow::refreshUserProfile()
{
    auto &session = InvestSphere::App::SessionManager::instance();
    std::string name = session.currentUserName();
    if (m_userNameLabel) {
        m_userNameLabel->setText(QString::fromStdString(name.empty() ? "Investor User" : name));
    }
    if (m_settingsPage) {
        m_settingsPage->loadCurrentUserData();
    }
    if (m_dashboardPage) {
        m_dashboardPage->loadDashboardData();
    }
    if (m_portfolioPage) {
        m_portfolioPage->loadInvestments();
    }
    if (m_transactionsPage) {
        m_transactionsPage->loadTransactions();
    }
}

QWidget* MainWindow::createSidebar()
{
    auto *sidebar = new QWidget(this);
    sidebar->setFixedWidth(260);
    sidebar->setStyleSheet(QStringLiteral(
        "QWidget {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-family: %3;"
        "}"
        "QLabel#brandTitle { color: #FFFFFF; font-size: 18px; font-weight: 700; letter-spacing: 0.5px; }"
        "QLabel#brandTagline { color: #60A5FA; font-size: 11px; font-weight: 500; letter-spacing: 0.3px; }"
        "QLabel#sectionHeader { color: #64748B; font-size: 10px; font-weight: 700; padding: 14px 16px 4px 16px; letter-spacing: 1px; }"
    ).arg(InvestSphere::Ui::Theme::ColorPrimarySidebar)
     .arg(InvestSphere::Ui::Theme::ColorSidebarText)
     .arg(InvestSphere::Ui::Theme::fontFamily()));

    auto *sidebarMainLayout = new QVBoxLayout(sidebar);
    sidebarMainLayout->setContentsMargins(0, 0, 0, 16);
    sidebarMainLayout->setSpacing(0);

    // 1. Branding Header Container
    auto *brandContainer = new QWidget(sidebar);
    auto *brandLayout = new QHBoxLayout(brandContainer);
    brandLayout->setContentsMargins(20, 20, 20, 16);
    brandLayout->setSpacing(12);

    auto *brandLogo = new QLabel(brandContainer);
    brandLogo->setFixedSize(36, 36);
    brandLogo->setPixmap(InvestSphere::Ui::IconHelper::getPixmap(
        InvestSphere::Ui::IconHelper::IconType::BrandLogo, QColor(37, 99, 235), 32));

    auto *brandTextLayout = new QVBoxLayout();
    brandTextLayout->setSpacing(2);

    auto *brandTitle = new QLabel(QStringLiteral("InvestSphere"), brandContainer);
    brandTitle->setObjectName(QStringLiteral("brandTitle"));

    auto *brandTagline = new QLabel(QStringLiteral("Smart Investment Management"), brandContainer);
    brandTagline->setObjectName(QStringLiteral("brandTagline"));

    brandTextLayout->addWidget(brandTitle);
    brandTextLayout->addWidget(brandTagline);

    brandLayout->addWidget(brandLogo);
    brandLayout->addLayout(brandTextLayout);
    sidebarMainLayout->addWidget(brandContainer);

    // Divider
    auto *divider = new QFrame(sidebar);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet(QStringLiteral("background-color: #1F2937; max-height: 1px; margin: 0px 16px;"));
    sidebarMainLayout->addWidget(divider);

    // 2. Scrollable Navigation List
    auto *navScrollArea = new QScrollArea(sidebar);
    navScrollArea->setWidgetResizable(true);
    navScrollArea->setFrameShape(QFrame::NoFrame);
    navScrollArea->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #374151; border-radius: 2px; }"
    ));

    auto *navContent = new QWidget(navScrollArea);
    auto *navLayout = new QVBoxLayout(navContent);
    navLayout->setContentsMargins(12, 12, 12, 12);
    navLayout->setSpacing(3);

    m_navButtonGroup = new QButtonGroup(this);
    m_navButtonGroup->setExclusive(true);

    auto addSection = [&](const QString &title) {
        auto *lbl = new QLabel(title, navContent);
        lbl->setObjectName(QStringLiteral("sectionHeader"));
        navLayout->addWidget(lbl);
    };

    // OVERVIEW
    addSection(QStringLiteral("OVERVIEW"));
    navLayout->addWidget(createNavButton(0, QStringLiteral("Dashboard"), InvestSphere::Ui::IconHelper::IconType::Dashboard));
    navLayout->addWidget(createNavButton(1, QStringLiteral("My Portfolio"), InvestSphere::Ui::IconHelper::IconType::Portfolio));

    // INVESTMENTS
    addSection(QStringLiteral("INVESTMENTS"));
    navLayout->addWidget(createNavButton(2, QStringLiteral("Stocks"), InvestSphere::Ui::IconHelper::IconType::Stocks));
    navLayout->addWidget(createNavButton(3, QStringLiteral("Mutual Funds"), InvestSphere::Ui::IconHelper::IconType::MutualFunds));
    navLayout->addWidget(createNavButton(4, QStringLiteral("Gold Holdings"), InvestSphere::Ui::IconHelper::IconType::Gold));
    navLayout->addWidget(createNavButton(5, QStringLiteral("Fixed Deposits"), InvestSphere::Ui::IconHelper::IconType::FixedDeposits));
    navLayout->addWidget(createNavButton(6, QStringLiteral("Bonds"), InvestSphere::Ui::IconHelper::IconType::Bonds));

    // ACTIVITY
    addSection(QStringLiteral("ACTIVITY"));
    navLayout->addWidget(createNavButton(7, QStringLiteral("Transactions"), InvestSphere::Ui::IconHelper::IconType::Transactions));
    navLayout->addWidget(createNavButton(8, QStringLiteral("Market Watch"), InvestSphere::Ui::IconHelper::IconType::MarketWatch));

    // FINANCIAL TOOLS
    addSection(QStringLiteral("FINANCIAL TOOLS"));
    navLayout->addWidget(createNavButton(9, QStringLiteral("SIP Calculator"), InvestSphere::Ui::IconHelper::IconType::SipCalculator));
    navLayout->addWidget(createNavButton(10, QStringLiteral("SWP Calculator"), InvestSphere::Ui::IconHelper::IconType::SwpCalculator));
    navLayout->addWidget(createNavButton(11, QStringLiteral("Maturity Tracker"), InvestSphere::Ui::IconHelper::IconType::MaturityTracker));

    // INSIGHTS
    addSection(QStringLiteral("INSIGHTS"));
    navLayout->addWidget(createNavButton(12, QStringLiteral("Portfolio Analytics"), InvestSphere::Ui::IconHelper::IconType::Analytics));
    navLayout->addWidget(createNavButton(13, QStringLiteral("Reports"), InvestSphere::Ui::IconHelper::IconType::Reports));

    navLayout->addStretch(1);

    // ACCOUNT
    addSection(QStringLiteral("ACCOUNT"));
    navLayout->addWidget(createNavButton(14, QStringLiteral("Settings"), InvestSphere::Ui::IconHelper::IconType::Settings));

    // Logout Action Button
    auto *logoutBtn = new QPushButton(QStringLiteral("  Logout"), navContent);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFixedHeight(38);
    logoutBtn->setIcon(InvestSphere::Ui::IconHelper::getIcon(InvestSphere::Ui::IconHelper::IconType::Logout, QColor(239, 68, 68), 16));
    logoutBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "   text-align: left;"
        "   padding-left: 14px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   color: #F87171;"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1F2937;"
        "   color: #EF4444;"
        "}"
    ));
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::handleLogout);
    navLayout->addWidget(logoutBtn);

    navScrollArea->setWidget(navContent);
    sidebarMainLayout->addWidget(navScrollArea, 1);

    return sidebar;
}

QPushButton* MainWindow::createNavButton(int pageIndex, const QString &text, InvestSphere::Ui::IconHelper::IconType icon)
{
    auto *btn = new QPushButton(QStringLiteral("  ") + text, this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(38);
    btn->setCheckable(true);
    btn->setIcon(InvestSphere::Ui::IconHelper::getIcon(icon, QColor(148, 163, 184), 16));

    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "   text-align: left;"
        "   padding-left: 14px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "   color: #94A3B8;"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1E293B;"
        "   color: #F8FAFC;"
        "}"
        "QPushButton:checked {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   font-weight: 600;"
        "}"
    ));

    m_navButtonGroup->addButton(btn, pageIndex);
    m_navButtons.push_back(btn);

    connect(btn, &QPushButton::clicked, this, [this, pageIndex]() {
        navigateToPage(pageIndex);
    });

    return btn;
}

QWidget* MainWindow::createHeader()
{
    auto *header = new QWidget(this);
    header->setFixedHeight(68);
    header->setStyleSheet(QStringLiteral(
        "background-color: #FFFFFF;"
        "border-bottom: 1px solid #E5E7EB;"
    ));

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(28, 0, 28, 0);
    headerLayout->setSpacing(16);

    // Page Title & Subtitle
    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    titleLayout->setAlignment(Qt::AlignVCenter);

    m_pageTitleLabel = new QLabel(QStringLiteral("Executive Dashboard"), header);
    m_pageTitleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 18px; font-weight: 700; color: #0F172A;"
    ).arg(InvestSphere::Ui::Theme::fontFamily()));

    m_pageSubtitleLabel = new QLabel(QStringLiteral("Overview / Real-time portfolio valuation and allocation summary"), header);
    m_pageSubtitleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 12px; color: #64748B;"
    ).arg(InvestSphere::Ui::Theme::fontFamily()));

    titleLayout->addWidget(m_pageTitleLabel);
    titleLayout->addWidget(m_pageSubtitleLabel);
    headerLayout->addLayout(titleLayout);

    headerLayout->addStretch(1);

    // Global Search Field
    auto *searchField = new InvestSphere::Ui::SearchField(QStringLiteral("Global search investments, symbols..."), header);
    searchField->setFixedWidth(280);
    headerLayout->addWidget(searchField);

    // Quick Add Action
    auto *quickAddBtn = new QPushButton(QStringLiteral("+ New Investment"), header);
    quickAddBtn->setCursor(Qt::PointingHandCursor);
    quickAddBtn->setStyleSheet(InvestSphere::Ui::Theme::primaryButtonStyle());
    quickAddBtn->setFixedHeight(36);
    connect(quickAddBtn, &QPushButton::clicked, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_topNotificationBanner) {
                m_topNotificationBanner->showMessage(
                    QStringLiteral("Investment holding recorded successfully."),
                    InvestSphere::Ui::NotificationBanner::BannerType::Success
                );
            }
        });
        dialog->exec();
    });
    headerLayout->addWidget(quickAddBtn);

    // Notification Icon Button
    auto *bellBtn = new QPushButton(header);
    bellBtn->setFixedSize(36, 36);
    bellBtn->setCursor(Qt::PointingHandCursor);
    bellBtn->setIcon(InvestSphere::Ui::IconHelper::getIcon(InvestSphere::Ui::IconHelper::IconType::Bell, QColor(100, 116, 139), 18));
    bellBtn->setToolTip(QStringLiteral("Notifications & System Alerts"));
    bellBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; }"
        "QPushButton:hover { background: #F1F5F9; border-color: #CBD5E1; }"
    ));
    connect(bellBtn, &QPushButton::clicked, this, [this]() {
        if (m_topNotificationBanner) {
            m_topNotificationBanner->showMessage(
                QStringLiteral("System Notification: All relational integrity triggers, audit logs, and SQLite database connections are running normally."),
                InvestSphere::Ui::NotificationBanner::BannerType::Success
            );
        }
    });
    headerLayout->addWidget(bellBtn);

    // User Profile Pill (Displays dynamic authenticated user name)
    auto *profilePill = new QFrame(header);
    profilePill->setCursor(Qt::PointingHandCursor);
    profilePill->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 18px;"
        "   padding: 4px 10px;"
        "}"
        "QFrame:hover {"
        "   background-color: #F1F5F9;"
        "   border-color: #CBD5E1;"
        "}"
    ));
    auto *pillLayout = new QHBoxLayout(profilePill);
    pillLayout->setContentsMargins(6, 2, 8, 2);
    pillLayout->setSpacing(8);

    auto *avatar = new QLabel(profilePill);
    avatar->setFixedSize(28, 28);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QStringLiteral("background-color: #2563EB; border-radius: 14px;"));
    avatar->setPixmap(InvestSphere::Ui::IconHelper::getPixmap(InvestSphere::Ui::IconHelper::IconType::User, Qt::white, 16));

    m_userNameLabel = new QLabel(QStringLiteral("Investor User"), profilePill);
    m_userNameLabel->setStyleSheet(QStringLiteral("font-weight: 600; color: #1F2937; font-size: 13px;"));

    auto *userRole = new QLabel(QStringLiteral("Active"), profilePill);
    userRole->setStyleSheet(QStringLiteral(
        "background-color: #DBEAFE; color: #1E40AF; font-size: 10px; font-weight: 700; "
        "padding: 1px 6px; border-radius: 4px;"
    ));

    pillLayout->addWidget(avatar);
    pillLayout->addWidget(m_userNameLabel);
    pillLayout->addWidget(userRole);

    headerLayout->addWidget(profilePill);

    return header;
}

QWidget* MainWindow::createContentArea()
{
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet(QStringLiteral("background: transparent;"));

    // Instantiate single instances of the 15 pages
    m_dashboardPage = new InvestSphere::Ui::DashboardPage(m_stackedWidget);
    m_portfolioPage = new InvestSphere::Ui::PortfolioPage(m_stackedWidget);
    auto *stocksPage = new InvestSphere::Ui::StocksPage(m_stackedWidget);
    auto *mutualFundsPage = new InvestSphere::Ui::MutualFundsPage(m_stackedWidget);
    auto *goldPage = new InvestSphere::Ui::GoldPage(m_stackedWidget);
    auto *fixedDepositsPage = new InvestSphere::Ui::FixedDepositsPage(m_stackedWidget);
    auto *bondsPage = new InvestSphere::Ui::BondsPage(m_stackedWidget);
    m_transactionsPage = new InvestSphere::Ui::TransactionsPage(m_stackedWidget);
    m_marketWatchPage = new InvestSphere::Ui::MarketWatchPage(m_stackedWidget);
    auto *sipCalcPage = new InvestSphere::Ui::SipCalculatorPage(m_stackedWidget);
    auto *swpCalcPage = new InvestSphere::Ui::SwpCalculatorPage(m_stackedWidget);
    m_maturityTrackerPage = new InvestSphere::Ui::MaturityTrackerPage(m_stackedWidget);
    m_analyticsPage = new InvestSphere::Ui::AnalyticsPage(m_stackedWidget);
    m_reportsPage = new InvestSphere::Ui::ReportsPage(m_stackedWidget);
    m_settingsPage = new InvestSphere::Ui::SettingsPage(m_stackedWidget);

    connect(m_settingsPage, &InvestSphere::Ui::SettingsPage::logoutRequested, this, &MainWindow::handleLogout);

    // Cross-page signal wiring for Quick Actions & updates
    connect(m_portfolioPage, &InvestSphere::Ui::PortfolioPage::investmentsChanged, this, [this]() {
        if (m_dashboardPage) m_dashboardPage->loadDashboardData();
        if (m_transactionsPage) m_transactionsPage->loadTransactions();
        if (m_analyticsPage) m_analyticsPage->loadAnalytics();
    });

    connect(m_transactionsPage, &InvestSphere::Ui::TransactionsPage::transactionsChanged, this, [this]() {
        if (m_dashboardPage) m_dashboardPage->loadDashboardData();
        if (m_portfolioPage) m_portfolioPage->loadInvestments();
        if (m_analyticsPage) m_analyticsPage->loadAnalytics();
    });

    connect(m_dashboardPage, &InvestSphere::Ui::DashboardPage::addInvestmentRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(m_dashboardPage, &InvestSphere::Ui::DashboardPage::recordTransactionRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::TransactionDialog(this);
        connect(dialog, &InvestSphere::Ui::TransactionDialog::transactionSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(m_dashboardPage, &InvestSphere::Ui::DashboardPage::openSipCalculatorRequested, this, [this]() {
        navigateToPage(9); // SIP Calculator
    });
    connect(m_dashboardPage, &InvestSphere::Ui::DashboardPage::openSwpCalculatorRequested, this, [this]() {
        navigateToPage(10); // SWP Calculator
    });

    connect(m_portfolioPage, &InvestSphere::Ui::PortfolioPage::addInvestmentRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(stocksPage, &InvestSphere::Ui::StocksPage::addStockRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        dialog->setCategory(QStringLiteral("STOCK"));
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(mutualFundsPage, &InvestSphere::Ui::MutualFundsPage::addMutualFundRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        dialog->setCategory(QStringLiteral("MUTUAL_FUND"));
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(goldPage, &InvestSphere::Ui::GoldPage::addGoldRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        dialog->setCategory(QStringLiteral("GOLD"));
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(fixedDepositsPage, &InvestSphere::Ui::FixedDepositsPage::addFixedDepositRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        dialog->setCategory(QStringLiteral("FIXED_DEPOSIT"));
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });
    connect(bondsPage, &InvestSphere::Ui::BondsPage::addBondRequested, this, [this]() {
        auto *dialog = new InvestSphere::Ui::InvestmentDialog(this);
        dialog->setCategory(QStringLiteral("BOND"));
        connect(dialog, &InvestSphere::Ui::InvestmentDialog::investmentSaved, this, [this]() {
            if (m_dashboardPage) m_dashboardPage->loadDashboardData();
            if (m_portfolioPage) m_portfolioPage->loadInvestments();
            if (m_transactionsPage) m_transactionsPage->loadTransactions();
        });
        dialog->exec();
    });

    // Add in sequential order matching indices 0 to 14
    m_stackedWidget->addWidget(m_dashboardPage);      // 0
    m_stackedWidget->addWidget(m_portfolioPage);      // 1
    m_stackedWidget->addWidget(stocksPage);           // 2
    m_stackedWidget->addWidget(mutualFundsPage);      // 3
    m_stackedWidget->addWidget(goldPage);             // 4
    m_stackedWidget->addWidget(fixedDepositsPage);    // 5
    m_stackedWidget->addWidget(bondsPage);            // 6
    m_stackedWidget->addWidget(m_transactionsPage);    // 7
    m_stackedWidget->addWidget(m_marketWatchPage);     // 8
    m_stackedWidget->addWidget(sipCalcPage);           // 9
    m_stackedWidget->addWidget(swpCalcPage);           // 10
    m_stackedWidget->addWidget(m_maturityTrackerPage); // 11
    m_stackedWidget->addWidget(m_analyticsPage);       // 12
    m_stackedWidget->addWidget(m_reportsPage);         // 13
    m_stackedWidget->addWidget(m_settingsPage);        // 14

    return m_stackedWidget;
}

void MainWindow::navigateToPage(int index)
{
    if (!m_stackedWidget || index < 0 || index >= m_stackedWidget->count()) {
        return;
    }

    m_stackedWidget->setCurrentIndex(index);
    updateHeader(index);

    if (index == 0 && m_dashboardPage) {
        m_dashboardPage->loadDashboardData();
    } else if (index == 1 && m_portfolioPage) {
        m_portfolioPage->loadInvestments();
    } else if (index == 7 && m_transactionsPage) {
        m_transactionsPage->loadTransactions();
    } else if (index == 8 && m_marketWatchPage) {
        m_marketWatchPage->loadMarketData();
    } else if (index == 11 && m_maturityTrackerPage) {
        m_maturityTrackerPage->loadData();
    } else if (index == 12 && m_analyticsPage) {
        m_analyticsPage->loadAnalytics();
    } else if (index == 13 && m_reportsPage) {
        m_reportsPage->loadSummaryData();
    }

    // Update checked state of the corresponding button
    if (m_navButtonGroup) {
        auto *btn = m_navButtonGroup->button(index);
        if (btn) {
            btn->setChecked(true);
        }
    }
}

void MainWindow::updateHeader(int index)
{
    if (index >= 0 && index < static_cast<int>(m_pageMetadata.size())) {
        if (m_pageTitleLabel) {
            m_pageTitleLabel->setText(m_pageMetadata[index].title);
        }
        if (m_pageSubtitleLabel) {
            m_pageSubtitleLabel->setText(m_pageMetadata[index].subtitle);
        }
    }
}

void MainWindow::handleLogout()
{
    bool confirmed = InvestSphere::Ui::ConfirmDialog::confirm(
        this,
        QStringLiteral("Confirm Logout"),
        QStringLiteral("Are you sure you want to sign out of your InvestSphere session? Any unsaved form entries will be cleared."),
        QStringLiteral("Logout"),
        true
    );

    if (confirmed) {
        InvestSphere::App::SessionManager::instance().logout();
        emit logoutRequested();
    }
}

#endif // INVESTSPHERE_HAS_QT

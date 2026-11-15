#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <vector>
#include <utility>

#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {
class DashboardPage;
class PortfolioPage;
class StocksPage;
class MutualFundsPage;
class GoldPage;
class FixedDepositsPage;
class BondsPage;
class TransactionsPage;
class MarketWatchPage;
class SipCalculatorPage;
class SwpCalculatorPage;
class MaturityTrackerPage;
class AnalyticsPage;
class ReportsPage;
class SettingsPage;
class NotificationBanner;
}
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void navigateToPage(int index);
    void refreshUserProfile();

signals:
    void logoutRequested();

private:
    void setupUi();
    QWidget* createSidebar();
    QWidget* createHeader();
    QWidget* createContentArea();
    QPushButton* createNavButton(int pageIndex, const QString &text, InvestSphere::Ui::IconHelper::IconType icon);
    void updateHeader(int index);
    void handleLogout();

    QLabel *m_pageTitleLabel{nullptr};
    QLabel *m_pageSubtitleLabel{nullptr};
    QLabel *m_userNameLabel{nullptr};
    QStackedWidget *m_stackedWidget{nullptr};
    QButtonGroup *m_navButtonGroup{nullptr};
    InvestSphere::Ui::NotificationBanner *m_topNotificationBanner{nullptr};
    InvestSphere::Ui::SettingsPage *m_settingsPage{nullptr};
    InvestSphere::Ui::DashboardPage *m_dashboardPage{nullptr};
    InvestSphere::Ui::PortfolioPage *m_portfolioPage{nullptr};
    InvestSphere::Ui::TransactionsPage *m_transactionsPage{nullptr};
    InvestSphere::Ui::AnalyticsPage *m_analyticsPage{nullptr};
    InvestSphere::Ui::MarketWatchPage *m_marketWatchPage{nullptr};
    InvestSphere::Ui::MaturityTrackerPage *m_maturityTrackerPage{nullptr};
    InvestSphere::Ui::ReportsPage *m_reportsPage{nullptr};

    struct PageMeta {
        QString title;
        QString subtitle;
    };
    std::vector<PageMeta> m_pageMetadata;
    std::vector<QPushButton*> m_navButtons;
};

#endif // INVESTSPHERE_HAS_QT
#endif // MAINWINDOW_H

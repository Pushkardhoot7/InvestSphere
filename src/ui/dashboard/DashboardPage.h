#ifndef INVESTSPHERE_DASHBOARDPAGE_H
#define INVESTSPHERE_DASHBOARDPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QScrollArea>
#include "SummaryCard.h"
#include "SectionHeader.h"
#include "ChartPlaceholder.h"
#include "ModernTableWidget.h"
#include "EmptyStateWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Main Dashboard page displaying portfolio executive summary cards,
 * interactive asset allocation and history charts, quick action bar,
 * top holdings, recent ledger transactions & upcoming financial events.
 */
class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

public slots:
    void loadDashboardData();

signals:
    void addInvestmentRequested();
    void recordTransactionRequested();
    void openSipCalculatorRequested();
    void openSwpCalculatorRequested();

private:
    void setupUi();
    QWidget* createQuickActionsBar();
    QWidget* createSummaryCardsRow();
    QWidget* createChartsRow();
    QWidget* createTopHoldingsSection();
    QWidget* createActivityRow();

    // 5 Executive Summary Cards
    SummaryCard *m_totalInvestedCard{nullptr};
    SummaryCard *m_currentValueCard{nullptr};
    SummaryCard *m_unrealizedPnlCard{nullptr};
    SummaryCard *m_realizedPnlCard{nullptr};
    SummaryCard *m_activeHoldingsCard{nullptr};

    // Chart Components
    ChartPlaceholder *m_allocationChart{nullptr};
    ChartPlaceholder *m_historyChart{nullptr};

    // Activity & Ledger Tables
    ModernTableWidget *m_topHoldingsTable{nullptr};
    ModernTableWidget *m_recentTransactionsTable{nullptr};
    ModernTableWidget *m_upcomingEventsTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_DASHBOARDPAGE_H

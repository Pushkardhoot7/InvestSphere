#ifndef INVESTSPHERE_ANALYTICSPAGE_H
#define INVESTSPHERE_ANALYTICSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QFrame>

#include "SummaryCard.h"
#include "ModernTableWidget.h"
#include "AnalyticsService.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Professional Portfolio Analytics Page.
 *
 * Integrates live cost basis, market valuations, category breakdowns,
 * top/worst performers, and snapshot-based historical portfolio performance.
 */
class AnalyticsPage : public QWidget {
    Q_OBJECT

public:
    explicit AnalyticsPage(QWidget *parent = nullptr);

public slots:
    void loadAnalytics();

private slots:
    void handleFilterChanged();
    void handleTakeSnapshot();

private:
    void setupUi();
    void setupHeader(QVBoxLayout *layout);
    void setupOverviewCards(QVBoxLayout *layout);
    void setupCategorySection(QVBoxLayout *layout);
    void setupPerformersSection(QVBoxLayout *layout);
    void setupHistorySection(QVBoxLayout *layout);

    void populateOverview(const Services::PortfolioAnalyticsDTO &dto);
    void populateCategories(const Services::PortfolioAnalyticsDTO &dto);
    void populatePerformers(const Services::PortfolioAnalyticsDTO &dto);
    void populateHistory(const Services::PortfolioAnalyticsDTO &dto);

    QString formatInrCents(int64_t cents) const;

    // ── Filters & Controls ───────────────────────────────────────────────────
    QComboBox   *m_categoryFilterCombo{nullptr};
    QComboBox   *m_timeHorizonCombo{nullptr};
    QPushButton *m_refreshBtn{nullptr};
    QPushButton *m_snapshotBtn{nullptr};
    QLabel      *m_statusLabel{nullptr};

    // ── Overview Summary Cards ───────────────────────────────────────────────
    SummaryCard *m_costBasisCard{nullptr};
    SummaryCard *m_currentValueCard{nullptr};
    SummaryCard *m_unrealizedPnlCard{nullptr};
    SummaryCard *m_realizedPnlCard{nullptr};
    SummaryCard *m_incomeCard{nullptr};
    SummaryCard *m_holdingsCountCard{nullptr};

    // ── Category Analysis Table ──────────────────────────────────────────────
    QTableWidget *m_categoryTable{nullptr};
    QLabel       *m_coverageLabel{nullptr};

    // ── Performers Tables ────────────────────────────────────────────────────
    QTableWidget *m_topPerformersTable{nullptr};
    QTableWidget *m_worstPerformersTable{nullptr};

    // ── Snapshot History Table & Empty State ─────────────────────────────────
    QTableWidget *m_historyTable{nullptr};
    QFrame       *m_historyEmptyCard{nullptr};
    QLabel       *m_historyEmptyMsg{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_ANALYTICSPAGE_H

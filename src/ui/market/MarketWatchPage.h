#ifndef INVESTSPHERE_MARKETWATCHPAGE_H
#define INVESTSPHERE_MARKETWATCHPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QTimer>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Market Watch page displaying major index cards, watchlist quotes with live price data
 * from MarketDataService, and filter-driven table updates.
 */
class MarketWatchPage : public QWidget {
    Q_OBJECT

public:
    explicit MarketWatchPage(QWidget *parent = nullptr);

public slots:
    void loadMarketData();

private slots:
    void onFilterChanged();
    void onRefreshClicked();

private:
    void setupUi();
    void populateTable(const QString &categoryFilter, const QString &searchQuery);
    static QString formatPriceCents(int64_t cents);

    // Summary cards (index cards)
    SummaryCard *m_niftyCard{nullptr};
    SummaryCard *m_sensexCard{nullptr};
    SummaryCard *m_goldCard{nullptr};

    FilterBar         *m_filterBar{nullptr};
    ModernTableWidget *m_watchlistTable{nullptr};

    QTimer *m_refreshTimer{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_MARKETWATCHPAGE_H


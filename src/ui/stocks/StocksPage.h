#ifndef INVESTSPHERE_STOCKSPAGE_H
#define INVESTSPHERE_STOCKSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Stocks & Equities Portfolio page with stock holdings table, performance badges, and filters.
 */
class StocksPage : public QWidget {
    Q_OBJECT

public:
    explicit StocksPage(QWidget *parent = nullptr);

signals:
    void addStockRequested();

private:
    void setupUi();

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_stocksTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_STOCKSPAGE_H

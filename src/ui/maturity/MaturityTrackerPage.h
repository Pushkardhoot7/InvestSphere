#ifndef INVESTSPHERE_MATURITYTRACKERPAGE_H
#define INVESTSPHERE_MATURITYTRACKERPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Maturity Tracker page wired to MaturityService showing upcoming redemption dates,
 * bond coupon schedules, and liquidity summary cards.
 */
class MaturityTrackerPage : public QWidget {
    Q_OBJECT

public:
    explicit MaturityTrackerPage(QWidget *parent = nullptr);

public slots:
    void loadData();

private slots:
    void onFilterChanged();
    void onExportClicked();

private:
    void setupUi();
    void populateTable();
    static QString formatCents(int64_t cents);
    static QString badgeText(const std::string &window);

    SummaryCard *m_in30DaysCard{nullptr};
    SummaryCard *m_in90DaysCard{nullptr};
    SummaryCard *m_totalInflowCard{nullptr};

    FilterBar         *m_filterBar{nullptr};
    ModernTableWidget *m_maturityTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_MATURITYTRACKERPAGE_H

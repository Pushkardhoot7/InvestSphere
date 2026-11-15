#ifndef INVESTSPHERE_MUTUALFUNDSPAGE_H
#define INVESTSPHERE_MUTUALFUNDSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Mutual Funds page displaying fund portfolios, NAV tracking, and SIP plans.
 */
class MutualFundsPage : public QWidget {
    Q_OBJECT

public:
    explicit MutualFundsPage(QWidget *parent = nullptr);

signals:
    void addMutualFundRequested();

private:
    void setupUi();

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_fundsTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_MUTUALFUNDSPAGE_H

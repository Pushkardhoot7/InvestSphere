#ifndef INVESTSPHERE_FIXEDDEPOSITSPAGE_H
#define INVESTSPHERE_FIXEDDEPOSITSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Fixed Deposits page tracking bank deposits, tenure, compounding, and maturities.
 */
class FixedDepositsPage : public QWidget {
    Q_OBJECT

public:
    explicit FixedDepositsPage(QWidget *parent = nullptr);

signals:
    void addFixedDepositRequested();

private:
    void setupUi();

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_fdTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_FIXEDDEPOSITSPAGE_H

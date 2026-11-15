#ifndef INVESTSPHERE_BONDSPAGE_H
#define INVESTSPHERE_BONDSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Bonds & Fixed Income Securities page for tracking government and corporate debt.
 */
class BondsPage : public QWidget {
    Q_OBJECT

public:
    explicit BondsPage(QWidget *parent = nullptr);

signals:
    void addBondRequested();

private:
    void setupUi();

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_bondsTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_BONDSPAGE_H

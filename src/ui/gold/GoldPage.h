#ifndef INVESTSPHERE_GOLDPAGE_H
#define INVESTSPHERE_GOLDPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Gold Holdings page for tracking physical bullion, Sovereign Gold Bonds (SGB), and digital gold.
 */
class GoldPage : public QWidget {
    Q_OBJECT

public:
    explicit GoldPage(QWidget *parent = nullptr);

signals:
    void addGoldRequested();

private:
    void setupUi();

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_goldTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_GOLDPAGE_H

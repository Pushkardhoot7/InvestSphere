#ifndef INVESTSPHERE_INVESTMENTDETAILSDIALOG_H
#define INVESTSPHERE_INVESTMENTDETAILSDIALOG_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "InvestmentService.h"
#include "ModernTableWidget.h"
#include "StatusBadge.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modal dialog presenting in-depth investment metadata, category attributes,
 * live market valuation status, and linked audit transaction ledger.
 */
class InvestmentDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvestmentDetailsDialog(int investmentId, QWidget *parent = nullptr);

signals:
    void investmentChanged();
    void investmentDeleted();

private slots:
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUi();
    void loadData();

    int m_investmentId{0};

    QLabel *m_titleLabel{nullptr};
    QLabel *m_symbolLabel{nullptr};
    QLabel *m_categoryLabel{nullptr};
    QLabel *m_statusBadgeLabel{nullptr};

    // Metric Labels
    QLabel *m_qtyValLabel{nullptr};
    QLabel *m_investedValLabel{nullptr};
    QLabel *m_priceValLabel{nullptr};
    QLabel *m_marketValLabel{nullptr};
    QLabel *m_pnlValLabel{nullptr};

    // Attribute layout
    QWidget *m_attributesWidget{nullptr};
    QVBoxLayout *m_attributesLayout{nullptr};

    // Linked Transactions Table
    ModernTableWidget *m_txTable{nullptr};

    QPushButton *m_editBtn{nullptr};
    QPushButton *m_deleteBtn{nullptr};
    QPushButton *m_closeBtn{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_INVESTMENTDETAILSDIALOG_H

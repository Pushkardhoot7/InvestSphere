#ifndef INVESTSPHERE_TRANSACTIONDETAILSDIALOG_H
#define INVESTSPHERE_TRANSACTIONDETAILSDIALOG_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include "TransactionModel.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modal dialog presenting in-depth transaction ledger metadata,
 * breakdown of fees/taxes, gross vs. net settlement amounts, and direct edit/delete actions.
 */
class TransactionDetailsDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransactionDetailsDialog(int transactionId, QWidget *parent = nullptr);

signals:
    void transactionChanged();
    void transactionDeleted();

private slots:
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUi();
    void loadData();

    int m_transactionId{0};

    QLabel *m_titleLabel{nullptr};
    QLabel *m_typeBadgeLabel{nullptr};
    QLabel *m_instrumentLabel{nullptr};
    QLabel *m_dateLabel{nullptr};
    QLabel *m_qtyLabel{nullptr};
    QLabel *m_priceLabel{nullptr};
    QLabel *m_grossLabel{nullptr};
    QLabel *m_chargesLabel{nullptr};
    QLabel *m_netLabel{nullptr};
    QLabel *m_notesLabel{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_TRANSACTIONDETAILSDIALOG_H

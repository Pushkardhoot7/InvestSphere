#ifndef INVESTSPHERE_TRANSACTIONDIALOG_H
#define INVESTSPHERE_TRANSACTIONDIALOG_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>

#include "TransactionService.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modal dialog for recording a new transaction or editing an existing transaction.
 * Supports dynamic type filtering, balance sufficiency display, and real-time net amount calculation.
 */
class TransactionDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransactionDialog(QWidget *parent = nullptr);
    explicit TransactionDialog(int transactionId, QWidget *parent = nullptr); // Edit mode

    void setPreselectedInvestment(int investmentId);

signals:
    void transactionSaved();

private slots:
    void onInvestmentChanged(int index);
    void onTypeChanged(int index);
    void onAmountFieldsChanged();
    void onSaveClicked();

private:
    void setupUi();
    void loadInvestmentOptions();
    void populateForEdit();
    void updateTypeOptionsForCategory(const QString &categoryCode);
    void recalculateNetAmount();
    void showError(const QString &msg);
    void clearError();

    bool m_isEditMode{false};
    int m_transactionId{0};
    int m_preselectedInvestmentId{0};

    std::vector<Services::UserInvestmentOptionDTO> m_investmentOptions;

    // Form inputs
    QComboBox *m_investmentCombo{nullptr};
    QLabel *m_holdingBalanceLabel{nullptr};
    QComboBox *m_typeCombo{nullptr};
    QDateEdit *m_dateEdit{nullptr};
    QDoubleSpinBox *m_qtySpin{nullptr};
    QDoubleSpinBox *m_priceSpin{nullptr};
    QDoubleSpinBox *m_grossSpin{nullptr};
    QDoubleSpinBox *m_feesSpin{nullptr};
    QDoubleSpinBox *m_taxesSpin{nullptr};
    QLabel *m_netAmountDisplay{nullptr};
    QLineEdit *m_notesEdit{nullptr};
    QLabel *m_errorLabel{nullptr};

    QPushButton *m_saveBtn{nullptr};
    QPushButton *m_cancelBtn{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_TRANSACTIONDIALOG_H

#ifndef INVESTSPHERE_INVESTMENTDIALOG_H
#define INVESTSPHERE_INVESTMENTDIALOG_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFormLayout>
#include <QGroupBox>

#include "InvestmentService.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Modal dialog for adding or editing an investment holding with dynamic category fields.
 * Supports Stocks, Mutual Funds, Gold, Fixed Deposits, and Bonds.
 */
class InvestmentDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvestmentDialog(QWidget *parent = nullptr);
    explicit InvestmentDialog(int investmentId, QWidget *parent = nullptr); // Edit mode

    void setCategory(const QString &categoryCode);

signals:
    void investmentSaved();

private slots:
    void onCategoryChanged(int index);
    void onSaveClicked();
    void updateFdMaturityCalculation();

private:
    void setupUi();
    void populateForEdit();
    void showError(const QString &msg);
    void clearError();

    bool m_isEditMode{false};
    int m_investmentId{0};

    // Category Selector
    QComboBox *m_categoryCombo{nullptr};

    // Common Inputs
    QLineEdit *m_symbolEdit{nullptr};
    QLineEdit *m_nameEdit{nullptr};
    QDateEdit *m_purchaseDateEdit{nullptr};
    QDoubleSpinBox *m_quantitySpin{nullptr};
    QDoubleSpinBox *m_priceSpin{nullptr};
    QComboBox *m_tagCombo{nullptr};
    QComboBox *m_statusCombo{nullptr};
    QTextEdit *m_notesEdit{nullptr};

    // Category Specific Form Containers
    QWidget *m_stocksGroup{nullptr};
    QWidget *m_mfGroup{nullptr};
    QWidget *m_goldGroup{nullptr};
    QWidget *m_fdGroup{nullptr};
    QWidget *m_bondsGroup{nullptr};

    // Stocks fields
    QLineEdit *m_exchangeEdit{nullptr};
    QLineEdit *m_sectorEdit{nullptr};
    QLineEdit *m_industryEdit{nullptr};
    QLineEdit *m_isinEdit{nullptr};
    QComboBox *m_mcapCombo{nullptr};

    // MF fields
    QLineEdit *m_amcEdit{nullptr};
    QComboBox *m_mfCategoryCombo{nullptr};
    QComboBox *m_planTypeCombo{nullptr};
    QDoubleSpinBox *m_expenseRatioSpin{nullptr};

    // Gold fields
    QComboBox *m_goldFormCombo{nullptr};
    QDoubleSpinBox *m_puritySpin{nullptr};
    QLineEdit *m_storageEdit{nullptr};

    // FD fields
    QLineEdit *m_fdBankEdit{nullptr};
    QLineEdit *m_fdAccountEdit{nullptr};
    QDoubleSpinBox *m_fdPrincipalSpin{nullptr};
    QDoubleSpinBox *m_fdRateSpin{nullptr};
    QComboBox *m_fdCompoundingCombo{nullptr};
    QComboBox *m_fdPayoutCombo{nullptr};
    QDateEdit *m_fdStartDateEdit{nullptr};
    QDateEdit *m_fdMaturityDateEdit{nullptr};
    QLabel *m_fdProjectedMaturityLabel{nullptr};

    // Bond fields
    QLineEdit *m_bondIssuerEdit{nullptr};
    QComboBox *m_bondTypeCombo{nullptr};
    QDoubleSpinBox *m_bondFaceValueSpin{nullptr};
    QDoubleSpinBox *m_bondCouponRateSpin{nullptr};
    QComboBox *m_bondCouponFreqCombo{nullptr};
    QDateEdit *m_bondIssueDateEdit{nullptr};
    QDateEdit *m_bondMaturityDateEdit{nullptr};
    QLineEdit *m_bondCreditRatingEdit{nullptr};

    // Error banner & Action buttons
    QLabel *m_errorLabel{nullptr};
    QPushButton *m_saveBtn{nullptr};
    QPushButton *m_cancelBtn{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_INVESTMENTDIALOG_H

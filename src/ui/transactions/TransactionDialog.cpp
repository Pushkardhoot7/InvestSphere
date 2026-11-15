#include "TransactionDialog.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QMessageBox>

#include "Theme.h"
#include "SessionManager.h"
#include "TransactionService.h"

namespace InvestSphere {
namespace Ui {

static QString formatMoney(int64_t cents)
{
    bool negative = cents < 0;
    int64_t absCents = negative ? -cents : cents;
    int64_t whole = absCents / 100;
    int64_t frac = absCents % 100;
    QString wholeStr = QString::number(whole);
    
    int n = wholeStr.length();
    if (n > 3) {
        QString lastThree = wholeStr.right(3);
        QString rest = wholeStr.left(n - 3);
        QString formatted;
        while (rest.length() > 2) {
            formatted = QStringLiteral(",") + rest.right(2) + formatted;
            rest.chop(2);
        }
        wholeStr = rest + formatted + QStringLiteral(",") + lastThree;
    }
    return (negative ? QStringLiteral("-₹") : QStringLiteral("₹")) + wholeStr + QStringLiteral(".") + QString("%1").arg(frac, 2, 10, QChar('0'));
}

TransactionDialog::TransactionDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false)
{
    setupUi();
    loadInvestmentOptions();
}

TransactionDialog::TransactionDialog(int transactionId, QWidget *parent)
    : QDialog(parent), m_isEditMode(true), m_transactionId(transactionId)
{
    setupUi();
    loadInvestmentOptions();
    populateForEdit();
}

void TransactionDialog::setPreselectedInvestment(int investmentId)
{
    m_preselectedInvestmentId = investmentId;
    for (int i = 0; i < m_investmentCombo->count(); ++i) {
        if (m_investmentCombo->itemData(i).toInt() == investmentId) {
            m_investmentCombo->setCurrentIndex(i);
            break;
        }
    }
}

void TransactionDialog::setupUi()
{
    setWindowTitle(m_isEditMode ? QStringLiteral("Edit Transaction Record") : QStringLiteral("Record New Financial Transaction"));
    setMinimumWidth(540);
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #FFFFFF; font-family: %1; }"
        "QLabel { color: #1E293B; font-size: 13px; font-weight: 500; }"
        "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox {"
        "   border: 1px solid #CBD5E1; border-radius: 6px; padding: 6px 10px; font-size: 13px; background: #F8FAFC;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus {"
        "   border-color: #2563EB; background: #FFFFFF;"
        "}"
    ).arg(Theme::fontFamily()));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(16);

    // Header
    auto *titleLabel = new QLabel(
        m_isEditMode ? QStringLiteral("Modify Transaction") : QStringLiteral("New Transaction Entry"),
        this
    );
    titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #0F172A;"));
    mainLayout->addWidget(titleLabel);

    auto *descLabel = new QLabel(
        QStringLiteral("Record buy, sell, dividend, interest, or cash movement in the double-entry ledger."),
        this
    );
    descLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; margin-bottom: 4px;"));
    mainLayout->addWidget(descLabel);

    // Error banner
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(QStringLiteral(
        "background-color: #FEF2F2; color: #DC2626; border: 1px solid #FECACA; border-radius: 6px; padding: 8px 12px; font-size: 12px;"
    ));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    mainLayout->addWidget(m_errorLabel);

    // Form Container
    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    // 1. Investment selector
    m_investmentCombo = new QComboBox(this);
    connect(m_investmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TransactionDialog::onInvestmentChanged);
    formLayout->addRow(QStringLiteral("Asset / Holding *"), m_investmentCombo);

    // Holding balance hint
    m_holdingBalanceLabel = new QLabel(this);
    m_holdingBalanceLabel->setStyleSheet(QStringLiteral("color: #2563EB; font-size: 11px; font-weight: 600; margin-left: 2px;"));
    formLayout->addRow(QString(), m_holdingBalanceLabel);

    // 2. Transaction Type
    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TransactionDialog::onTypeChanged);
    formLayout->addRow(QStringLiteral("Transaction Type *"), m_typeCombo);

    // 3. Transaction Date
    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    formLayout->addRow(QStringLiteral("Transaction Date *"), m_dateEdit);

    // 4. Quantity / Units
    m_qtySpin = new QDoubleSpinBox(this);
    m_qtySpin->setDecimals(4);
    m_qtySpin->setRange(0.0000, 1000000.0000);
    m_qtySpin->setValue(1.0);
    connect(m_qtySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TransactionDialog::onAmountFieldsChanged);
    formLayout->addRow(QStringLiteral("Quantity / Units *"), m_qtySpin);

    // 5. Unit Price
    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setRange(0.00, 100000000.00);
    m_priceSpin->setPrefix(QStringLiteral("₹ "));
    connect(m_priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TransactionDialog::onAmountFieldsChanged);
    formLayout->addRow(QStringLiteral("Unit Price *"), m_priceSpin);

    // 6. Gross Amount
    m_grossSpin = new QDoubleSpinBox(this);
    m_grossSpin->setDecimals(2);
    m_grossSpin->setRange(0.00, 1000000000.00);
    m_grossSpin->setPrefix(QStringLiteral("₹ "));
    connect(m_grossSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TransactionDialog::onAmountFieldsChanged);
    formLayout->addRow(QStringLiteral("Gross Amount *"), m_grossSpin);

    // 7. Brokerage & Charges
    m_feesSpin = new QDoubleSpinBox(this);
    m_feesSpin->setDecimals(2);
    m_feesSpin->setRange(0.00, 1000000.00);
    m_feesSpin->setPrefix(QStringLiteral("₹ "));
    connect(m_feesSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TransactionDialog::onAmountFieldsChanged);
    formLayout->addRow(QStringLiteral("Brokerage / Fees"), m_feesSpin);

    // 8. Taxes (STT, GST)
    m_taxesSpin = new QDoubleSpinBox(this);
    m_taxesSpin->setDecimals(2);
    m_taxesSpin->setRange(0.00, 1000000.00);
    m_taxesSpin->setPrefix(QStringLiteral("₹ "));
    connect(m_taxesSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TransactionDialog::onAmountFieldsChanged);
    formLayout->addRow(QStringLiteral("Taxes (STT / GST)"), m_taxesSpin);

    // 9. Net Settlement Amount (Read-only live calculated)
    m_netAmountDisplay = new QLabel(QStringLiteral("₹ 0.00"), this);
    m_netAmountDisplay->setStyleSheet(QStringLiteral(
        "font-size: 15px; font-weight: 700; color: #16A34A; padding: 4px 0px;"
    ));
    formLayout->addRow(QStringLiteral("Net Settlement Amount"), m_netAmountDisplay);

    // 10. Notes / Reference
    m_notesEdit = new QLineEdit(this);
    m_notesEdit->setPlaceholderText(QStringLiteral("Broker order ID, contract note ref, or memo"));
    formLayout->addRow(QStringLiteral("Notes / Reference"), m_notesEdit);

    mainLayout->addLayout(formLayout);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    btnLayout->addStretch(1);

    m_cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    m_cancelBtn->setStyleSheet(Theme::secondaryButtonStyle());
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton(
        m_isEditMode ? QStringLiteral("Save Changes") : QStringLiteral("Confirm & Record"),
        this
    );
    m_saveBtn->setStyleSheet(Theme::primaryButtonStyle());
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveBtn, &QPushButton::clicked, this, &TransactionDialog::onSaveClicked);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

void TransactionDialog::loadInvestmentOptions()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    m_investmentOptions = Services::TransactionService::instance().getUserInvestmentOptions(userId);
    m_investmentCombo->clear();

    for (const auto &opt : m_investmentOptions) {
        QString display = QString::fromStdString(opt.symbol.empty() ? opt.instrumentName : opt.symbol + " - " + opt.instrumentName);
        display += QStringLiteral(" (") + QString::fromStdString(opt.categoryName) + QStringLiteral(")");
        m_investmentCombo->addItem(display, opt.investmentId);
    }

    if (m_preselectedInvestmentId > 0) {
        setPreselectedInvestment(m_preselectedInvestmentId);
    } else if (!m_investmentOptions.empty()) {
        onInvestmentChanged(0);
    }
}

void TransactionDialog::onInvestmentChanged(int index)
{
    if (index < 0 || index >= static_cast<int>(m_investmentOptions.size())) {
        m_holdingBalanceLabel->clear();
        return;
    }

    const auto &opt = m_investmentOptions[index];
    m_holdingBalanceLabel->setText(
        QStringLiteral("Current Holding Balance: %1 units @ Avg Cost %2")
            .arg(QString::number(opt.currentHoldingQuantity, 'f', 2))
            .arg(formatMoney(opt.averageCostPerUnitCents))
    );

    updateTypeOptionsForCategory(QString::fromStdString(opt.categoryCode));
    recalculateNetAmount();
}

void TransactionDialog::updateTypeOptionsForCategory(const QString &categoryCode)
{
    QString currentType = m_typeCombo->currentText();
    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();

    if (categoryCode == QStringLiteral("STOCK")) {
        m_typeCombo->addItem(QStringLiteral("BUY"), QStringLiteral("BUY"));
        m_typeCombo->addItem(QStringLiteral("SELL"), QStringLiteral("SELL"));
        m_typeCombo->addItem(QStringLiteral("DIVIDEND"), QStringLiteral("DIVIDEND"));
        m_typeCombo->addItem(QStringLiteral("FEE"), QStringLiteral("FEE"));
    } else if (categoryCode == QStringLiteral("MUTUAL_FUND")) {
        m_typeCombo->addItem(QStringLiteral("BUY"), QStringLiteral("BUY"));
        m_typeCombo->addItem(QStringLiteral("REDEMPTION"), QStringLiteral("REDEMPTION"));
        m_typeCombo->addItem(QStringLiteral("DIVIDEND"), QStringLiteral("DIVIDEND"));
        m_typeCombo->addItem(QStringLiteral("FEE"), QStringLiteral("FEE"));
    } else if (categoryCode == QStringLiteral("GOLD")) {
        m_typeCombo->addItem(QStringLiteral("BUY"), QStringLiteral("BUY"));
        m_typeCombo->addItem(QStringLiteral("SELL"), QStringLiteral("SELL"));
        m_typeCombo->addItem(QStringLiteral("FEE"), QStringLiteral("FEE"));
    } else if (categoryCode == QStringLiteral("FIXED_DEPOSIT")) {
        m_typeCombo->addItem(QStringLiteral("DEPOSIT"), QStringLiteral("DEPOSIT"));
        m_typeCombo->addItem(QStringLiteral("WITHDRAWAL"), QStringLiteral("WITHDRAWAL"));
        m_typeCombo->addItem(QStringLiteral("INTEREST"), QStringLiteral("INTEREST"));
        m_typeCombo->addItem(QStringLiteral("FEE"), QStringLiteral("FEE"));
    } else if (categoryCode == QStringLiteral("BOND")) {
        m_typeCombo->addItem(QStringLiteral("BUY"), QStringLiteral("BUY"));
        m_typeCombo->addItem(QStringLiteral("SELL"), QStringLiteral("SELL"));
        m_typeCombo->addItem(QStringLiteral("INTEREST"), QStringLiteral("INTEREST"));
        m_typeCombo->addItem(QStringLiteral("REDEMPTION"), QStringLiteral("REDEMPTION"));
        m_typeCombo->addItem(QStringLiteral("FEE"), QStringLiteral("FEE"));
    } else {
        m_typeCombo->addItem(QStringLiteral("BUY"), QStringLiteral("BUY"));
        m_typeCombo->addItem(QStringLiteral("SELL"), QStringLiteral("SELL"));
    }

    int idx = m_typeCombo->findText(currentType);
    if (idx >= 0) {
        m_typeCombo->setCurrentIndex(idx);
    } else {
        m_typeCombo->setCurrentIndex(0);
    }
    m_typeCombo->blockSignals(false);
    onTypeChanged(m_typeCombo->currentIndex());
}

void TransactionDialog::onTypeChanged(int)
{
    QString type = m_typeCombo->currentData().toString();
    bool isUnitBased = (type == QStringLiteral("BUY") || type == QStringLiteral("SELL") || type == QStringLiteral("REDEMPTION"));
    
    m_qtySpin->setEnabled(isUnitBased);
    m_priceSpin->setEnabled(isUnitBased);

    if (!isUnitBased) {
        m_qtySpin->setValue(1.0);
    }
    recalculateNetAmount();
}

void TransactionDialog::onAmountFieldsChanged()
{
    QString type = m_typeCombo->currentData().toString();
    bool isUnitBased = (type == QStringLiteral("BUY") || type == QStringLiteral("SELL") || type == QStringLiteral("REDEMPTION"));

    if (isUnitBased && sender() != m_grossSpin) {
        double qty = m_qtySpin->value();
        double price = m_priceSpin->value();
        m_grossSpin->blockSignals(true);
        m_grossSpin->setValue(qty * price);
        m_grossSpin->blockSignals(false);
    }

    recalculateNetAmount();
}

void TransactionDialog::recalculateNetAmount()
{
    QString type = m_typeCombo->currentData().toString();
    double gross = m_grossSpin->value();
    double fees = m_feesSpin->value();
    double taxes = m_taxesSpin->value();
    double net = 0.0;

    if (type == QStringLiteral("BUY") || type == QStringLiteral("DEPOSIT")) {
        net = gross + fees + taxes;
    } else {
        net = std::max(0.0, gross - fees - taxes);
    }

    int64_t netCents = static_cast<int64_t>(std::round(net * 100.0));
    m_netAmountDisplay->setText(formatMoney(netCents));
}

void TransactionDialog::populateForEdit()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0 || m_transactionId <= 0) return;

    auto txOpt = Services::TransactionService::instance().getTransactionDetail(userId, m_transactionId);
    if (!txOpt) {
        showError(QStringLiteral("Could not load transaction record."));
        return;
    }

    const auto &tx = *txOpt;
    setPreselectedInvestment(tx.investmentId);
    m_investmentCombo->setEnabled(false); // Locking investment on edit to preserve relational integrity

    int typeIdx = m_typeCombo->findData(QString::fromStdString(tx.transactionType));
    if (typeIdx >= 0) m_typeCombo->setCurrentIndex(typeIdx);

    m_dateEdit->setDate(QDate::fromString(QString::fromStdString(tx.transactionDate).left(10), QStringLiteral("yyyy-MM-dd")));
    m_qtySpin->setValue(tx.quantity);
    m_priceSpin->setValue(tx.unitPriceCents / 100.0);
    m_grossSpin->setValue(tx.grossAmountCents / 100.0);
    m_feesSpin->setValue(tx.feesCents / 100.0);
    m_taxesSpin->setValue(tx.taxesCents / 100.0);
    m_notesEdit->setText(QString::fromStdString(tx.notes));

    recalculateNetAmount();
}

void TransactionDialog::onSaveClicked()
{
    clearError();
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) {
        showError(QStringLiteral("Session expired. Please log in again."));
        return;
    }

    int invId = m_investmentCombo->currentData().toInt();
    if (invId <= 0) {
        showError(QStringLiteral("Please select an investment holding."));
        return;
    }

    Services::TransactionInputDTO input;
    input.investmentId = invId;
    input.transactionType = m_typeCombo->currentData().toString().toStdString();
    input.transactionDate = m_dateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();
    input.quantity = m_qtySpin->value();
    input.unitPriceCents = static_cast<int64_t>(std::round(m_priceSpin->value() * 100.0));
    input.grossAmountCents = static_cast<int64_t>(std::round(m_grossSpin->value() * 100.0));
    input.feesCents = static_cast<int64_t>(std::round(m_feesSpin->value() * 100.0));
    input.taxesCents = static_cast<int64_t>(std::round(m_taxesSpin->value() * 100.0));

    if (input.transactionType == "BUY" || input.transactionType == "DEPOSIT") {
        input.netAmountCents = input.grossAmountCents + input.feesCents + input.taxesCents;
    } else {
        input.netAmountCents = input.grossAmountCents - input.feesCents - input.taxesCents;
    }
    input.notes = m_notesEdit->text().trimmed().toStdString();

    std::pair<bool, std::string> res;
    if (m_isEditMode) {
        res = Services::TransactionService::instance().updateTransaction(userId, m_transactionId, input);
    } else {
        res = Services::TransactionService::instance().recordTransaction(userId, input);
    }

    if (!res.first) {
        showError(QString::fromStdString(res.second));
        return;
    }

    emit transactionSaved();
    accept();
}

void TransactionDialog::showError(const QString &msg)
{
    m_errorLabel->setText(msg);
    m_errorLabel->setVisible(true);
}

void TransactionDialog::clearError()
{
    m_errorLabel->clear();
    m_errorLabel->setVisible(false);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

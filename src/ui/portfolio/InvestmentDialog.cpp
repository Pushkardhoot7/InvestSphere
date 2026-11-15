#include "InvestmentDialog.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QDateTime>

#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"
#include "CompoundingEngine.h"

namespace InvestSphere {
namespace Ui {

InvestmentDialog::InvestmentDialog(QWidget *parent)
    : QDialog(parent)
    , m_isEditMode(false)
    , m_investmentId(0)
{
    setupUi();
    setWindowTitle(QStringLiteral("Add New Investment — InvestSphere"));
}

InvestmentDialog::InvestmentDialog(int investmentId, QWidget *parent)
    : QDialog(parent)
    , m_isEditMode(true)
    , m_investmentId(investmentId)
{
    setupUi();
    setWindowTitle(QStringLiteral("Edit Investment Holding — InvestSphere"));
    populateForEdit();
}

void InvestmentDialog::setupUi()
{
    setFixedSize(580, 680);
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #F8FAFC; font-family: %1; }"
        "QLabel { color: #334155; font-size: 12px; font-weight: 500; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit, QTextEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   padding: 6px 10px;"
        "   color: #0F172A;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QTextEdit:focus {"
        "   border: 1.5px solid #2563EB;"
        "}"
        "QGroupBox {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   margin-top: 14px;"
        "   padding: 14px;"
        "   font-weight: 600;"
        "   color: #1E293B;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 0 6px;"
        "   color: #2563EB;"
        "   font-size: 12px;"
        "}"
    ).arg(Theme::fontFamily()));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(14);

    // Dialog Header
    auto *headerLabel = new QLabel(m_isEditMode ? QStringLiteral("Edit Holding Details") : QStringLiteral("Record New Investment"), this);
    headerLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #0F172A;"));
    rootLayout->addWidget(headerLabel);

    auto *subLabel = new QLabel(QStringLiteral("Choose category and supply accurate acquisition attributes for portfolio tracking."), this);
    subLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; margin-bottom: 4px;"));
    rootLayout->addWidget(subLabel);

    // Error banner
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet(QStringLiteral(
        "background-color: #FEE2E2; color: #991B1B; border: 1px solid #F87171; "
        "border-radius: 6px; padding: 8px 12px; font-size: 12px; font-weight: 500;"
    ));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    rootLayout->addWidget(m_errorLabel);

    // Scrollable Form Container
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *formContainer = new QWidget(scrollArea);
    auto *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(0, 0, 8, 0);
    formLayout->setSpacing(14);

    // 1. Category Selection Row
    auto *catBox = new QHBoxLayout();
    auto *catLabel = new QLabel(QStringLiteral("Asset Category:"), formContainer);
    catLabel->setFixedWidth(120);
    catLabel->setStyleSheet(QStringLiteral("font-weight: 700; color: #1E293B;"));

    m_categoryCombo = new QComboBox(formContainer);
    m_categoryCombo->addItem(QStringLiteral("Equity Stocks"), QStringLiteral("STOCK"));
    m_categoryCombo->addItem(QStringLiteral("Mutual Funds"), QStringLiteral("MUTUAL_FUND"));
    m_categoryCombo->addItem(QStringLiteral("Gold & Precious Metals"), QStringLiteral("GOLD"));
    m_categoryCombo->addItem(QStringLiteral("Fixed Deposits (FD)"), QStringLiteral("FIXED_DEPOSIT"));
    m_categoryCombo->addItem(QStringLiteral("Bonds & Debentures"), QStringLiteral("BOND"));

    if (m_isEditMode) {
        m_categoryCombo->setEnabled(false); // Category cannot be changed on edit
    }

    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvestmentDialog::onCategoryChanged);

    catBox->addWidget(catLabel);
    catBox->addWidget(m_categoryCombo, 1);
    formLayout->addLayout(catBox);

    // 2. Common Fields Group
    auto *commonGroup = new QGroupBox(QStringLiteral("Basic Information"), formContainer);
    auto *commonLayout = new QFormLayout(commonGroup);
    commonLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(commonGroup);
    m_nameEdit->setPlaceholderText(QStringLiteral("e.g. Tata Consultancy Services, HDFC FD..."));
    commonLayout->addRow(QStringLiteral("Investment Name *:"), m_nameEdit);

    m_symbolEdit = new QLineEdit(commonGroup);
    m_symbolEdit->setPlaceholderText(QStringLiteral("e.g. TCS, VFIAX, GOLD-24K..."));
    commonLayout->addRow(QStringLiteral("Symbol / Code *:"), m_symbolEdit);

    m_purchaseDateEdit = new QDateEdit(QDate::currentDate(), commonGroup);
    m_purchaseDateEdit->setCalendarPopup(true);
    m_purchaseDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    commonLayout->addRow(QStringLiteral("Acquisition Date *:"), m_purchaseDateEdit);

    m_quantitySpin = new QDoubleSpinBox(commonGroup);
    m_quantitySpin->setRange(0.0001, 10000000.0);
    m_quantitySpin->setDecimals(4);
    m_quantitySpin->setValue(1.0);
    commonLayout->addRow(QStringLiteral("Quantity / Units *:"), m_quantitySpin);

    m_priceSpin = new QDoubleSpinBox(commonGroup);
    m_priceSpin->setRange(0.01, 100000000.0);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setPrefix(QStringLiteral("₹ "));
    m_priceSpin->setValue(100.0);
    commonLayout->addRow(QStringLiteral("Price / Unit *:"), m_priceSpin);

    m_tagCombo = new QComboBox(commonGroup);
    m_tagCombo->addItems({QStringLiteral("DEFAULT"), QStringLiteral("GROWTH"), QStringLiteral("INCOME"),
                          QStringLiteral("RETIREMENT"), QStringLiteral("EMERGENCY"), QStringLiteral("HEDGE")});
    commonLayout->addRow(QStringLiteral("Portfolio Bucket:"), m_tagCombo);

    if (m_isEditMode) {
        m_statusCombo = new QComboBox(commonGroup);
        m_statusCombo->addItems({QStringLiteral("ACTIVE"), QStringLiteral("CLOSED"), QStringLiteral("WATCHLIST")});
        commonLayout->addRow(QStringLiteral("Status:"), m_statusCombo);
    }

    formLayout->addWidget(commonGroup);

    // 3. Category Specific Groups
    // STOCKS
    m_stocksGroup = new QGroupBox(QStringLiteral("Stock Equity Attributes"), formContainer);
    auto *stocksLayout = new QFormLayout(m_stocksGroup);
    m_exchangeEdit = new QLineEdit(QStringLiteral("NSE"), m_stocksGroup);
    m_sectorEdit = new QLineEdit(m_stocksGroup);
    m_industryEdit = new QLineEdit(m_stocksGroup);
    m_isinEdit = new QLineEdit(m_stocksGroup);
    m_mcapCombo = new QComboBox(m_stocksGroup);
    m_mcapCombo->addItems({QStringLiteral("LARGE_CAP"), QStringLiteral("MID_CAP"), QStringLiteral("SMALL_CAP"), QStringLiteral("MICRO_CAP")});

    stocksLayout->addRow(QStringLiteral("Exchange:"), m_exchangeEdit);
    stocksLayout->addRow(QStringLiteral("Sector:"), m_sectorEdit);
    stocksLayout->addRow(QStringLiteral("Industry:"), m_industryEdit);
    stocksLayout->addRow(QStringLiteral("ISIN (optional):"), m_isinEdit);
    stocksLayout->addRow(QStringLiteral("Market Cap:"), m_mcapCombo);
    formLayout->addWidget(m_stocksGroup);

    // MUTUAL FUNDS
    m_mfGroup = new QGroupBox(QStringLiteral("Mutual Fund Attributes"), formContainer);
    auto *mfLayout = new QFormLayout(m_mfGroup);
    m_amcEdit = new QLineEdit(m_mfGroup);
    m_mfCategoryCombo = new QComboBox(m_mfGroup);
    m_mfCategoryCombo->addItems({QStringLiteral("EQUITY"), QStringLiteral("DEBT"), QStringLiteral("HYBRID"), QStringLiteral("INDEX"), QStringLiteral("ELSS"), QStringLiteral("LIQUID")});
    m_planTypeCombo = new QComboBox(m_mfGroup);
    m_planTypeCombo->addItems({QStringLiteral("GROWTH"), QStringLiteral("DIVIDEND_PAYOUT"), QStringLiteral("DIVIDEND_REINVEST")});
    m_expenseRatioSpin = new QDoubleSpinBox(m_mfGroup);
    m_expenseRatioSpin->setRange(0.0, 10.0);
    m_expenseRatioSpin->setDecimals(2);
    m_expenseRatioSpin->setSuffix(QStringLiteral(" %"));
    m_expenseRatioSpin->setValue(0.5);

    mfLayout->addRow(QStringLiteral("AMC Name:"), m_amcEdit);
    mfLayout->addRow(QStringLiteral("Category:"), m_mfCategoryCombo);
    mfLayout->addRow(QStringLiteral("Plan Type:"), m_planTypeCombo);
    mfLayout->addRow(QStringLiteral("Expense Ratio:"), m_expenseRatioSpin);
    m_mfGroup->hide();
    formLayout->addWidget(m_mfGroup);

    // GOLD
    m_goldGroup = new QGroupBox(QStringLiteral("Gold Holding Attributes"), formContainer);
    auto *goldLayout = new QFormLayout(m_goldGroup);
    m_goldFormCombo = new QComboBox(m_goldGroup);
    m_goldFormCombo->addItem(QStringLiteral("Physical Coin"), QStringLiteral("PHYSICAL_COIN"));
    m_goldFormCombo->addItem(QStringLiteral("Physical Bar"), QStringLiteral("PHYSICAL_BAR"));
    m_goldFormCombo->addItem(QStringLiteral("Digital Gold"), QStringLiteral("DIGITAL_GOLD"));
    m_goldFormCombo->addItem(QStringLiteral("Sovereign Gold Bond (SGB)"), QStringLiteral("SOVEREIGN_BOND"));
    m_goldFormCombo->addItem(QStringLiteral("Gold ETF"), QStringLiteral("GOLD_ETF"));

    m_puritySpin = new QDoubleSpinBox(m_goldGroup);
    m_puritySpin->setRange(1.0, 24.0);
    m_puritySpin->setDecimals(1);
    m_puritySpin->setSuffix(QStringLiteral(" K"));
    m_puritySpin->setValue(24.0);

    m_storageEdit = new QLineEdit(m_goldGroup);
    m_storageEdit->setPlaceholderText(QStringLiteral("e.g. Bank Locker #24, Home Safe..."));

    goldLayout->addRow(QStringLiteral("Gold Form:"), m_goldFormCombo);
    goldLayout->addRow(QStringLiteral("Purity (Karats):"), m_puritySpin);
    goldLayout->addRow(QStringLiteral("Storage Location:"), m_storageEdit);
    m_goldGroup->hide();
    formLayout->addWidget(m_goldGroup);

    // FIXED DEPOSITS
    m_fdGroup = new QGroupBox(QStringLiteral("Term Deposit Attributes"), formContainer);
    auto *fdLayout = new QFormLayout(m_fdGroup);
    m_fdBankEdit = new QLineEdit(m_fdGroup);
    m_fdAccountEdit = new QLineEdit(m_fdGroup);
    m_fdRateSpin = new QDoubleSpinBox(m_fdGroup);
    m_fdRateSpin->setRange(0.0, 25.0);
    m_fdRateSpin->setDecimals(2);
    m_fdRateSpin->setSuffix(QStringLiteral(" %"));
    m_fdRateSpin->setValue(7.10);

    m_fdCompoundingCombo = new QComboBox(m_fdGroup);
    m_fdCompoundingCombo->addItems({QStringLiteral("QUARTERLY"), QStringLiteral("MONTHLY"), QStringLiteral("HALF_YEARLY"), QStringLiteral("ANNUALLY"), QStringLiteral("SIMPLE")});

    m_fdPayoutCombo = new QComboBox(m_fdGroup);
    m_fdPayoutCombo->addItems({QStringLiteral("CUMULATIVE"), QStringLiteral("PERIODIC_MONTHLY"), QStringLiteral("PERIODIC_QUARTERLY")});

    m_fdStartDateEdit = new QDateEdit(QDate::currentDate(), m_fdGroup);
    m_fdStartDateEdit->setCalendarPopup(true);
    m_fdStartDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    m_fdMaturityDateEdit = new QDateEdit(QDate::currentDate().addYears(1), m_fdGroup);
    m_fdMaturityDateEdit->setCalendarPopup(true);
    m_fdMaturityDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    m_fdProjectedMaturityLabel = new QLabel(QStringLiteral("₹ 0.00"), m_fdGroup);
    m_fdProjectedMaturityLabel->setStyleSheet(QStringLiteral("font-weight: 700; color: #10B981; font-size: 13px;"));

    connect(m_priceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvestmentDialog::updateFdMaturityCalculation);
    connect(m_fdRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InvestmentDialog::updateFdMaturityCalculation);
    connect(m_fdCompoundingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestmentDialog::updateFdMaturityCalculation);
    connect(m_fdPayoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestmentDialog::updateFdMaturityCalculation);
    connect(m_fdStartDateEdit, &QDateEdit::dateChanged, this, &InvestmentDialog::updateFdMaturityCalculation);
    connect(m_fdMaturityDateEdit, &QDateEdit::dateChanged, this, &InvestmentDialog::updateFdMaturityCalculation);

    fdLayout->addRow(QStringLiteral("Bank / Institution *:"), m_fdBankEdit);
    fdLayout->addRow(QStringLiteral("FD Account #:"), m_fdAccountEdit);
    fdLayout->addRow(QStringLiteral("Interest Rate *:"), m_fdRateSpin);
    fdLayout->addRow(QStringLiteral("Compounding:"), m_fdCompoundingCombo);
    fdLayout->addRow(QStringLiteral("Payout Mode:"), m_fdPayoutCombo);
    fdLayout->addRow(QStringLiteral("Start Date:"), m_fdStartDateEdit);
    fdLayout->addRow(QStringLiteral("Maturity Date *:"), m_fdMaturityDateEdit);
    fdLayout->addRow(QStringLiteral("Est. Maturity Value:"), m_fdProjectedMaturityLabel);
    m_fdGroup->hide();
    formLayout->addWidget(m_fdGroup);

    // BONDS
    m_bondsGroup = new QGroupBox(QStringLiteral("Bond & Debt Attributes"), formContainer);
    auto *bondsLayout = new QFormLayout(m_bondsGroup);
    m_bondIssuerEdit = new QLineEdit(m_bondsGroup);
    m_bondTypeCombo = new QComboBox(m_bondsGroup);
    m_bondTypeCombo->addItems({QStringLiteral("GOVERNMENT"), QStringLiteral("CORPORATE"), QStringLiteral("MUNICIPAL"), QStringLiteral("SOVEREIGN_GOLD")});
    m_bondCouponRateSpin = new QDoubleSpinBox(m_bondsGroup);
    m_bondCouponRateSpin->setRange(0.0, 30.0);
    m_bondCouponRateSpin->setDecimals(2);
    m_bondCouponRateSpin->setSuffix(QStringLiteral(" %"));
    m_bondCouponRateSpin->setValue(7.50);

    m_bondCouponFreqCombo = new QComboBox(m_bondsGroup);
    m_bondCouponFreqCombo->addItems({QStringLiteral("SEMI_ANNUALLY"), QStringLiteral("ANNUALLY"), QStringLiteral("QUARTERLY"), QStringLiteral("MONTHLY"), QStringLiteral("ZERO_COUPON")});

    m_bondIssueDateEdit = new QDateEdit(QDate::currentDate().addYears(-1), m_bondsGroup);
    m_bondIssueDateEdit->setCalendarPopup(true);
    m_bondIssueDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    m_bondMaturityDateEdit = new QDateEdit(QDate::currentDate().addYears(5), m_bondsGroup);
    m_bondMaturityDateEdit->setCalendarPopup(true);
    m_bondMaturityDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    m_bondCreditRatingEdit = new QLineEdit(QStringLiteral("AAA"), m_bondsGroup);

    bondsLayout->addRow(QStringLiteral("Issuer Name *:"), m_bondIssuerEdit);
    bondsLayout->addRow(QStringLiteral("Bond Type:"), m_bondTypeCombo);
    bondsLayout->addRow(QStringLiteral("Coupon Rate:"), m_bondCouponRateSpin);
    bondsLayout->addRow(QStringLiteral("Coupon Frequency:"), m_bondCouponFreqCombo);
    bondsLayout->addRow(QStringLiteral("Issue Date:"), m_bondIssueDateEdit);
    bondsLayout->addRow(QStringLiteral("Maturity Date *:"), m_bondMaturityDateEdit);
    bondsLayout->addRow(QStringLiteral("Credit Rating:"), m_bondCreditRatingEdit);
    m_bondsGroup->hide();
    formLayout->addWidget(m_bondsGroup);

    // 4. Notes Section
    auto *notesGroup = new QGroupBox(QStringLiteral("Investment Strategy & Notes"), formContainer);
    auto *notesLayout = new QVBoxLayout(notesGroup);
    m_notesEdit = new QTextEdit(notesGroup);
    m_notesEdit->setFixedHeight(60);
    m_notesEdit->setPlaceholderText(QStringLiteral("Record rationale, horizon, tax bracket notes..."));
    notesLayout->addWidget(m_notesEdit);
    formLayout->addWidget(notesGroup);

    scrollArea->setWidget(formContainer);
    rootLayout->addWidget(scrollArea, 1);

    // Dialog Button Row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);
    btnRow->addStretch(1);

    m_cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(m_cancelBtn);

    m_saveBtn = new QPushButton(m_isEditMode ? QStringLiteral("Save Changes") : QStringLiteral("Add Investment"), this);
    m_saveBtn->setFixedHeight(36);
    m_saveBtn->setStyleSheet(Theme::primaryButtonStyle());
    connect(m_saveBtn, &QPushButton::clicked, this, &InvestmentDialog::onSaveClicked);
    btnRow->addWidget(m_saveBtn);

    rootLayout->addLayout(btnRow);
}

void InvestmentDialog::setCategory(const QString &categoryCode)
{
    int idx = m_categoryCombo->findData(categoryCode);
    if (idx != -1) {
        m_categoryCombo->setCurrentIndex(idx);
    }
}

void InvestmentDialog::onCategoryChanged(int index)
{
    clearError();
    QString cat = m_categoryCombo->itemData(index).toString();

    m_stocksGroup->setVisible(cat == QStringLiteral("STOCK"));
    m_mfGroup->setVisible(cat == QStringLiteral("MUTUAL_FUND"));
    m_goldGroup->setVisible(cat == QStringLiteral("GOLD"));
    m_fdGroup->setVisible(cat == QStringLiteral("FIXED_DEPOSIT"));
    m_bondsGroup->setVisible(cat == QStringLiteral("BOND"));

    // Contextual label adjustments
    if (cat == QStringLiteral("FIXED_DEPOSIT")) {
        m_quantitySpin->setValue(1.0);
        m_quantitySpin->setEnabled(false);
        m_priceSpin->setPrefix(QStringLiteral("Principal: ₹ "));
        updateFdMaturityCalculation();
    } else {
        m_quantitySpin->setEnabled(true);
        m_priceSpin->setPrefix(QStringLiteral("₹ "));
    }
}

void InvestmentDialog::updateFdMaturityCalculation()
{
    int64_t principalCents = static_cast<int64_t>(m_priceSpin->value() * 100.0);
    double rate = m_fdRateSpin->value();
    QDate sDate = m_fdStartDateEdit->date();
    QDate mDate = m_fdMaturityDateEdit->date();

    int months = static_cast<int>(sDate.daysTo(mDate) / 30.4375);
    if (months <= 0) months = 1;

    QString freq = m_fdCompoundingCombo->currentText();
    QString payout = m_fdPayoutCombo->currentText();

    int64_t maturityCents = Finance::CompoundingEngine::calculateMaturityAmount(
        principalCents, rate, months, freq.toStdString(), payout.toStdString()
    );

    double maturityRupees = maturityCents / 100.0;
    m_fdProjectedMaturityLabel->setText(QStringLiteral("₹ %1").arg(QLocale().toString(maturityRupees, 'f', 2)));
}

void InvestmentDialog::populateForEdit()
{
    auto &session = App::SessionManager::instance();
    auto detail = Services::InvestmentService::instance().getInvestmentDetail(session.currentUserId(), m_investmentId);
    if (!detail) {
        showError(QStringLiteral("Unable to locate investment holding."));
        return;
    }

    setCategory(QString::fromStdString(detail->summary.categoryCode));
    m_nameEdit->setText(QString::fromStdString(detail->summary.name));
    m_symbolEdit->setText(QString::fromStdString(detail->summary.symbol));
    m_symbolEdit->setEnabled(false); // Symbol immutable on edit

    m_quantitySpin->setValue(detail->summary.quantity);
    m_quantitySpin->setEnabled(false); // Units change via transactions

    double price = detail->summary.quantity > 0.0001
        ? (detail->summary.investedAmountCents / 100.0) / detail->summary.quantity
        : (detail->summary.investedAmountCents / 100.0);
    m_priceSpin->setValue(price);
    m_priceSpin->setEnabled(false);

    m_purchaseDateEdit->setDate(QDate::fromString(QString::fromStdString(detail->summary.purchaseDate).left(10), QStringLiteral("yyyy-MM-dd")));
    m_purchaseDateEdit->setEnabled(false);

    int tagIdx = m_tagCombo->findText(QString::fromStdString(detail->summary.portfolioTag));
    if (tagIdx != -1) m_tagCombo->setCurrentIndex(tagIdx);

    if (m_statusCombo) {
        int statIdx = m_statusCombo->findText(QString::fromStdString(detail->summary.status));
        if (statIdx != -1) m_statusCombo->setCurrentIndex(statIdx);
    }

    m_notesEdit->setPlainText(QString::fromStdString(detail->summary.notes));

    // Specializations
    if (detail->stockDetails) {
        m_exchangeEdit->setText(QString::fromStdString(detail->stockDetails->marketCapCategory));
        m_sectorEdit->setText(QString::fromStdString(detail->stockDetails->sector));
        m_industryEdit->setText(QString::fromStdString(detail->stockDetails->industry));
        m_isinEdit->setText(QString::fromStdString(detail->stockDetails->isin));
    } else if (detail->mfDetails) {
        m_amcEdit->setText(QString::fromStdString(detail->mfDetails->amcName));
        m_expenseRatioSpin->setValue(detail->mfDetails->expenseRatio);
    } else if (detail->goldDetails) {
        m_storageEdit->setText(QString::fromStdString(detail->goldDetails->storageLocation));
        m_puritySpin->setValue(detail->goldDetails->purityKarat);
    } else if (detail->fdDetails) {
        m_fdBankEdit->setText(QString::fromStdString(detail->fdDetails->institutionName));
        m_fdAccountEdit->setText(QString::fromStdString(detail->fdDetails->accountNumber));
        m_fdRateSpin->setValue(detail->fdDetails->annualInterestRate);
        m_fdMaturityDateEdit->setDate(QDate::fromString(QString::fromStdString(detail->fdDetails->maturityDate), QStringLiteral("yyyy-MM-dd")));
    } else if (detail->bondDetails) {
        m_bondIssuerEdit->setText(QString::fromStdString(detail->bondDetails->issuerName));
        m_bondCouponRateSpin->setValue(detail->bondDetails->couponRate);
        m_bondCreditRatingEdit->setText(QString::fromStdString(detail->bondDetails->creditRating));
    }
}

void InvestmentDialog::onSaveClicked()
{
    clearError();
    auto &session = App::SessionManager::instance();
    int userId = session.currentUserId();

    if (m_isEditMode) {
        // Edit Mode: update notes, status, portfolio tag
        QString notes = m_notesEdit->toPlainText().trimmed();
        QString status = m_statusCombo ? m_statusCombo->currentText() : QStringLiteral("ACTIVE");
        QString tag = m_tagCombo->currentText();

        auto res = Services::InvestmentService::instance().updateInvestment(
            userId, m_investmentId, notes.toStdString(), status.toStdString(), tag.toStdString()
        );

        if (!res.first) {
            showError(QString::fromStdString(res.second));
            return;
        }

        emit investmentSaved();
        accept();
    } else {
        // Add Mode: validate and assemble DTO
        Services::InvestmentInputDTO dto;
        dto.categoryCode = m_categoryCombo->currentData().toString().toStdString();
        dto.instrumentName = m_nameEdit->text().trimmed().toStdString();
        dto.symbol = m_symbolEdit->text().trimmed().toStdString();
        dto.purchaseDate = m_purchaseDateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();
        dto.quantity = m_quantitySpin->value();
        dto.unitPriceCents = static_cast<int64_t>(m_priceSpin->value() * 100.0);
        dto.portfolioTag = m_tagCombo->currentText().toStdString();
        dto.notes = m_notesEdit->toPlainText().trimmed().toStdString();

        if (dto.categoryCode == "STOCK") {
            dto.exchange = m_exchangeEdit->text().trimmed().toStdString();
            dto.sector = m_sectorEdit->text().trimmed().toStdString();
            dto.industry = m_industryEdit->text().trimmed().toStdString();
            dto.isin = m_isinEdit->text().trimmed().toStdString();
            dto.marketCapCategory = m_mcapCombo->currentText().toStdString();
        } else if (dto.categoryCode == "MUTUAL_FUND") {
            dto.amcName = m_amcEdit->text().trimmed().toStdString();
            dto.fundCategory = m_mfCategoryCombo->currentText().toStdString();
            dto.planType = m_planTypeCombo->currentText().toStdString();
            dto.expenseRatio = m_expenseRatioSpin->value();
        } else if (dto.categoryCode == "GOLD") {
            dto.goldForm = m_goldFormCombo->currentData().toString().toStdString();
            dto.purityKarat = m_puritySpin->value();
            dto.storageLocation = m_storageEdit->text().trimmed().toStdString();
        } else if (dto.categoryCode == "FIXED_DEPOSIT") {
            dto.institutionName = m_fdBankEdit->text().trimmed().toStdString();
            dto.accountNumber = m_fdAccountEdit->text().trimmed().toStdString();
            dto.principalAmountCents = dto.unitPriceCents;
            dto.annualInterestRate = m_fdRateSpin->value();
            dto.compoundingFrequency = m_fdCompoundingCombo->currentText().toStdString();
            dto.payoutType = m_fdPayoutCombo->currentText().toStdString();
            dto.startDate = m_fdStartDateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();
            dto.maturityDate = m_fdMaturityDateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();

            QDate s = m_fdStartDateEdit->date();
            QDate m = m_fdMaturityDateEdit->date();
            int months = static_cast<int>(s.daysTo(m) / 30.4375);
            dto.durationMonths = (months > 0) ? months : 12;

            dto.estimatedMaturityAmountCents = Finance::CompoundingEngine::calculateMaturityAmount(
                dto.principalAmountCents, dto.annualInterestRate, dto.durationMonths, dto.compoundingFrequency, dto.payoutType
            );
        } else if (dto.categoryCode == "BOND") {
            dto.issuerName = m_bondIssuerEdit->text().trimmed().toStdString();
            dto.bondType = m_bondTypeCombo->currentText().toStdString();
            dto.faceValueCents = dto.unitPriceCents;
            dto.couponRate = m_bondCouponRateSpin->value();
            dto.couponFrequency = m_bondCouponFreqCombo->currentText().toStdString();
            dto.issueDate = m_bondIssueDateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();
            dto.bondMaturityDate = m_bondMaturityDateEdit->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString();
            dto.creditRating = m_bondCreditRatingEdit->text().trimmed().toStdString();
        }

        auto res = Services::InvestmentService::instance().addInvestment(userId, dto);
        if (!res.first) {
            showError(QString::fromStdString(res.second));
            return;
        }

        emit investmentSaved();
        accept();
    }
}

void InvestmentDialog::showError(const QString &msg)
{
    m_errorLabel->setText(msg);
    m_errorLabel->show();
}

void InvestmentDialog::clearError()
{
    m_errorLabel->clear();
    m_errorLabel->hide();
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

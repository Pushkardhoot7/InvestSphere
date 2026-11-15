#include "InvestmentDetailsDialog.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>
#include <QMessageBox>

#include "Theme.h"
#include "IconHelper.h"
#include "ConfirmDialog.h"
#include "SessionManager.h"
#include "InvestmentDialog.h"

namespace InvestSphere {
namespace Ui {

InvestmentDetailsDialog::InvestmentDetailsDialog(int investmentId, QWidget *parent)
    : QDialog(parent)
    , m_investmentId(investmentId)
{
    setupUi();
    loadData();
    setWindowTitle(QStringLiteral("Investment Position Overview — InvestSphere"));
}

void InvestmentDetailsDialog::setupUi()
{
    setFixedSize(680, 720);
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #F8FAFC; font-family: %1; }"
        "QLabel { color: #1E293B; }"
    ).arg(Theme::fontFamily()));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 20, 24, 20);
    rootLayout->setSpacing(16);

    // Header Container
    auto *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral("background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 10px; padding: 12px 16px;"));
    auto *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(4);

    m_titleLabel = new QLabel(QStringLiteral("Holding Name"), headerFrame);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #0F172A;"));

    auto *tagSubLayout = new QHBoxLayout();
    tagSubLayout->setSpacing(8);

    m_symbolLabel = new QLabel(QStringLiteral("SYMBOL"), headerFrame);
    m_symbolLabel->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 700; color: #2563EB;"));

    m_categoryLabel = new QLabel(QStringLiteral("Category"), headerFrame);
    m_categoryLabel->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B;"));

    tagSubLayout->addWidget(m_symbolLabel);
    tagSubLayout->addWidget(new QLabel(QStringLiteral("•"), headerFrame));
    tagSubLayout->addWidget(m_categoryLabel);
    tagSubLayout->addStretch(1);

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addLayout(tagSubLayout);
    headerLayout->addLayout(titleLayout);

    headerLayout->addStretch(1);

    m_statusBadgeLabel = new QLabel(QStringLiteral("ACTIVE"), headerFrame);
    m_statusBadgeLabel->setStyleSheet(QStringLiteral(
        "background-color: #DEF7EC; color: #03543F; font-size: 11px; font-weight: 700; "
        "padding: 4px 10px; border-radius: 12px;"
    ));
    headerLayout->addWidget(m_statusBadgeLabel);

    rootLayout->addWidget(headerFrame);

    // Financial Metrics Row (4 Cards)
    auto *metricsFrame = new QFrame(this);
    metricsFrame->setStyleSheet(QStringLiteral("background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 10px; padding: 12px;"));
    auto *metricsLayout = new QGridLayout(metricsFrame);
    metricsLayout->setSpacing(12);

    auto createMetric = [&](const QString &label, QLabel *&valRef, int row, int col, const QString &color = QStringLiteral("#0F172A")) {
        auto *lbl = new QLabel(label, metricsFrame);
        lbl->setStyleSheet(QStringLiteral("font-size: 11px; color: #64748B; font-weight: 600; text-transform: uppercase;"));
        valRef = new QLabel(QStringLiteral("—"), metricsFrame);
        valRef->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: %1;").arg(color));
        
        auto *box = new QVBoxLayout();
        box->setSpacing(2);
        box->addWidget(lbl);
        box->addWidget(valRef);
        metricsLayout->addLayout(box, row, col);
    };

    createMetric(QStringLiteral("Quantity Held"), m_qtyValLabel, 0, 0);
    createMetric(QStringLiteral("Total Invested Cost"), m_investedValLabel, 0, 1);
    createMetric(QStringLiteral("Latest Price / NAV"), m_priceValLabel, 0, 2);
    createMetric(QStringLiteral("Current Value"), m_marketValLabel, 1, 0);
    createMetric(QStringLiteral("Unrealized P&L"), m_pnlValLabel, 1, 1, QStringLiteral("#10B981"));

    rootLayout->addWidget(metricsFrame);

    // Category-Specific Attributes Section
    m_attributesWidget = new QWidget(this);
    m_attributesLayout = new QVBoxLayout(m_attributesWidget);
    m_attributesLayout->setContentsMargins(0, 0, 0, 0);
    m_attributesLayout->setSpacing(6);
    rootLayout->addWidget(m_attributesWidget);

    // Linked Transactions Ledger Table
    auto *txHeader = new QLabel(QStringLiteral("TRANSACTION AUDIT LEDGER"), this);
    txHeader->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 0.5px;"));
    rootLayout->addWidget(txHeader);

    m_txTable = new ModernTableWidget(this);
    m_txTable->setHeaders({QStringLiteral("Date"), QStringLiteral("Type"), QStringLiteral("Quantity"), QStringLiteral("Price"), QStringLiteral("Net Amount"), QStringLiteral("Notes")});
    m_txTable->setEmptyMessage(QStringLiteral("No transaction records found for this investment."));
    m_txTable->setFixedHeight(160);
    rootLayout->addWidget(m_txTable, 1);

    // Action Buttons Row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    m_deleteBtn = new QPushButton(QStringLiteral("Delete Position"), this);
    m_deleteBtn->setFixedHeight(36);
    m_deleteBtn->setStyleSheet(Theme::destructiveButtonStyle());
    connect(m_deleteBtn, &QPushButton::clicked, this, &InvestmentDetailsDialog::onDeleteClicked);
    btnRow->addWidget(m_deleteBtn);

    btnRow->addStretch(1);

    m_editBtn = new QPushButton(QStringLiteral("Edit Holding"), this);
    m_editBtn->setFixedHeight(36);
    m_editBtn->setStyleSheet(Theme::secondaryButtonStyle());
    connect(m_editBtn, &QPushButton::clicked, this, &InvestmentDetailsDialog::onEditClicked);
    btnRow->addWidget(m_editBtn);

    m_closeBtn = new QPushButton(QStringLiteral("Close"), this);
    m_closeBtn->setFixedHeight(36);
    m_closeBtn->setStyleSheet(Theme::primaryButtonStyle());
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(m_closeBtn);

    rootLayout->addLayout(btnRow);
}

void InvestmentDetailsDialog::loadData()
{
    auto &session = App::SessionManager::instance();
    auto detail = Services::InvestmentService::instance().getInvestmentDetail(session.currentUserId(), m_investmentId);
    if (!detail) {
        return;
    }

    m_titleLabel->setText(QString::fromStdString(detail->summary.name));
    m_symbolLabel->setText(QString::fromStdString(detail->summary.symbol));
    m_categoryLabel->setText(QString::fromStdString(detail->summary.categoryName));
    m_statusBadgeLabel->setText(QString::fromStdString(detail->summary.status));

    // Metrics
    m_qtyValLabel->setText(QString::number(detail->summary.quantity, 'f', 4));
    m_investedValLabel->setText(QStringLiteral("₹ ") + QLocale().toString(detail->summary.investedAmountCents / 100.0, 'f', 2));

    if (detail->summary.hasMarketPrice) {
        m_priceValLabel->setText(QStringLiteral("₹ ") + QLocale().toString(detail->summary.latestPriceCents / 100.0, 'f', 2));
    } else {
        m_priceValLabel->setText(QStringLiteral("Unavailable (No Quote)"));
        m_priceValLabel->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 500; color: #94A3B8;"));
    }

    if (detail->summary.hasCurrentValue) {
        m_marketValLabel->setText(QStringLiteral("₹ ") + QLocale().toString(detail->summary.currentValueCents / 100.0, 'f', 2));
    } else {
        m_marketValLabel->setText(QStringLiteral("Unavailable"));
        m_marketValLabel->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 500; color: #94A3B8;"));
    }

    if (detail->summary.hasPnl) {
        QString sign = detail->summary.unrealizedPnlCents >= 0 ? QStringLiteral("+₹ ") : QStringLiteral("-₹ ");
        double absPnl = std::abs(detail->summary.unrealizedPnlCents) / 100.0;
        QString pnlText = sign + QLocale().toString(absPnl, 'f', 2) +
                          QStringLiteral(" (%1%)").arg(QLocale().toString(detail->summary.unrealizedPnlPercent, 'f', 2));
        m_pnlValLabel->setText(pnlText);
        m_pnlValLabel->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 700; color: %1;")
            .arg(detail->summary.unrealizedPnlCents >= 0 ? QStringLiteral("#10B981") : QStringLiteral("#EF4444")));
    } else {
        m_pnlValLabel->setText(QStringLiteral("Unavailable"));
        m_pnlValLabel->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 500; color: #94A3B8;"));
    }

    // Populate Category Attributes
    QLayoutItem *item;
    while ((item = m_attributesLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    auto *attrBox = new QFrame(m_attributesWidget);
    attrBox->setStyleSheet(QStringLiteral("background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 10px 14px;"));
    auto *attrGrid = new QGridLayout(attrBox);
    attrGrid->setSpacing(8);

    auto addAttr = [&](int row, int col, const QString &label, const QString &val) {
        auto *l = new QLabel(label, attrBox);
        l->setStyleSheet(QStringLiteral("font-size: 11px; color: #64748B; font-weight: 600;"));
        auto *v = new QLabel(val.isEmpty() ? QStringLiteral("—") : val, attrBox);
        v->setStyleSheet(QStringLiteral("font-size: 12px; color: #1E293B; font-weight: 500;"));
        auto *colLayout = new QVBoxLayout();
        colLayout->setSpacing(2);
        colLayout->addWidget(l);
        colLayout->addWidget(v);
        attrGrid->addLayout(colLayout, row, col);
    };

    if (detail->stockDetails) {
        addAttr(0, 0, QStringLiteral("Sector"), QString::fromStdString(detail->stockDetails->sector));
        addAttr(0, 1, QStringLiteral("Industry"), QString::fromStdString(detail->stockDetails->industry));
        addAttr(0, 2, QStringLiteral("Market Cap"), QString::fromStdString(detail->stockDetails->marketCapCategory));
        addAttr(1, 0, QStringLiteral("ISIN"), QString::fromStdString(detail->stockDetails->isin));
        addAttr(1, 1, QStringLiteral("Portfolio Bucket"), QString::fromStdString(detail->summary.portfolioTag));
        addAttr(1, 2, QStringLiteral("Strategy Notes"), QString::fromStdString(detail->summary.notes));
    } else if (detail->mfDetails) {
        addAttr(0, 0, QStringLiteral("AMC Name"), QString::fromStdString(detail->mfDetails->amcName));
        addAttr(0, 1, QStringLiteral("Fund Category"), QString::fromStdString(detail->mfDetails->fundCategory));
        addAttr(0, 2, QStringLiteral("Plan Type"), QString::fromStdString(detail->mfDetails->planType));
        addAttr(1, 0, QStringLiteral("Expense Ratio"), QString::number(detail->mfDetails->expenseRatio, 'f', 2) + QStringLiteral("%"));
        addAttr(1, 1, QStringLiteral("Benchmark"), QString::fromStdString(detail->mfDetails->benchmarkIndex));
        addAttr(1, 2, QStringLiteral("Portfolio Bucket"), QString::fromStdString(detail->summary.portfolioTag));
    } else if (detail->goldDetails) {
        addAttr(0, 0, QStringLiteral("Gold Form"), QString::fromStdString(detail->goldDetails->goldForm));
        addAttr(0, 1, QStringLiteral("Purity"), QString::number(detail->goldDetails->purityKarat, 'f', 1) + QStringLiteral(" Karats"));
        addAttr(0, 2, QStringLiteral("Storage Location"), QString::fromStdString(detail->goldDetails->storageLocation));
        addAttr(1, 0, QStringLiteral("Portfolio Bucket"), QString::fromStdString(detail->summary.portfolioTag));
        addAttr(1, 1, QStringLiteral("Strategy Notes"), QString::fromStdString(detail->summary.notes));
    } else if (detail->fdDetails) {
        addAttr(0, 0, QStringLiteral("Bank / Institution"), QString::fromStdString(detail->fdDetails->institutionName));
        addAttr(0, 1, QStringLiteral("Account #"), QString::fromStdString(detail->fdDetails->accountNumber));
        addAttr(0, 2, QStringLiteral("Interest Rate"), QString::number(detail->fdDetails->annualInterestRate, 'f', 2) + QStringLiteral("%"));
        addAttr(1, 0, QStringLiteral("Compounding"), QString::fromStdString(detail->fdDetails->compoundingFrequency));
        addAttr(1, 1, QStringLiteral("Maturity Date"), QString::fromStdString(detail->fdDetails->maturityDate));
        addAttr(1, 2, QStringLiteral("Est. Maturity Value"), QStringLiteral("₹ ") + QLocale().toString(detail->fdDetails->estimatedMaturityAmountCents / 100.0, 'f', 2));
    } else if (detail->bondDetails) {
        addAttr(0, 0, QStringLiteral("Issuer"), QString::fromStdString(detail->bondDetails->issuerName));
        addAttr(0, 1, QStringLiteral("Bond Type"), QString::fromStdString(detail->bondDetails->bondType));
        addAttr(0, 2, QStringLiteral("Coupon Rate"), QString::number(detail->bondDetails->couponRate, 'f', 2) + QStringLiteral("%"));
        addAttr(1, 0, QStringLiteral("Coupon Frequency"), QString::fromStdString(detail->bondDetails->couponFrequency));
        addAttr(1, 1, QStringLiteral("Maturity Date"), QString::fromStdString(detail->bondDetails->maturityDate));
        addAttr(1, 2, QStringLiteral("Credit Rating"), QString::fromStdString(detail->bondDetails->creditRating));
    }

    m_attributesLayout->addWidget(attrBox);

    // Populate Transactions Table
    m_txTable->setRowCount(0);
    for (const auto &tx : detail->transactions) {
        int r = m_txTable->rowCount();
        m_txTable->insertRow(r);

        QString date = QString::fromStdString(tx.transactionDate.length() >= 10 ? tx.transactionDate.substr(0, 10) : tx.transactionDate);
        m_txTable->setTextCell(r, 0, date);

        StatusBadge::BadgeVariant variant = StatusBadge::BadgeVariant::Success;
        if (tx.transactionType == "SELL" || tx.transactionType == "REDEMPTION") {
            variant = StatusBadge::BadgeVariant::Danger;
        } else if (tx.transactionType == "DIVIDEND" || tx.transactionType == "INTEREST") {
            variant = StatusBadge::BadgeVariant::Info;
        }
        m_txTable->setBadgeCell(r, 1, QString::fromStdString(tx.transactionType), variant);

        m_txTable->setTextCell(r, 2, QString::number(tx.quantity, 'f', 4));
        m_txTable->setCurrencyCell(r, 3, tx.unitPriceCents / 100.0);
        m_txTable->setCurrencyCell(r, 4, tx.netAmountCents / 100.0);
        m_txTable->setTextCell(r, 5, QString::fromStdString(tx.notes));
    }
}

void InvestmentDetailsDialog::onEditClicked()
{
    InvestmentDialog dlg(m_investmentId, this);
    if (dlg.exec() == QDialog::Accepted) {
        loadData();
        emit investmentChanged();
    }
}

void InvestmentDetailsDialog::onDeleteClicked()
{
    auto &session = App::SessionManager::instance();
    int userId = session.currentUserId();

    int txCount = Services::InvestmentService::instance().getTransactionCount(userId, m_investmentId);

    QString promptMsg;
    if (txCount > 0) {
        promptMsg = QStringLiteral("This investment has %1 associated transaction ledger records.\n\n"
                                   "Are you sure you want to permanently delete this investment and its complete transaction history?\n"
                                   "This action cannot be undone.").arg(txCount);
    } else {
        promptMsg = QStringLiteral("Are you sure you want to permanently delete this investment holding?");
    }

    bool confirmed = ConfirmDialog::confirm(
        this,
        QStringLiteral("Confirm Investment Deletion"),
        promptMsg,
        QStringLiteral("Delete Position"),
        true
    );

    if (confirmed) {
        auto res = Services::InvestmentService::instance().deleteInvestment(userId, m_investmentId, true);
        if (!res.first) {
            QMessageBox::critical(this, QStringLiteral("Deletion Error"), QString::fromStdString(res.second));
            return;
        }

        emit investmentDeleted();
        accept();
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

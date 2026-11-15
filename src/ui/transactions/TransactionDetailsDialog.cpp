#include "TransactionDetailsDialog.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QMessageBox>

#include "Theme.h"
#include "SessionManager.h"
#include "TransactionService.h"
#include "TransactionDialog.h"
#include "ConfirmDialog.h"
#include "StatusBadge.h"

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

TransactionDetailsDialog::TransactionDetailsDialog(int transactionId, QWidget *parent)
    : QDialog(parent), m_transactionId(transactionId)
{
    setupUi();
    loadData();
}

void TransactionDetailsDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Transaction Audit Inspector"));
    setMinimumWidth(500);
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #FFFFFF; font-family: %1; }"
        "QLabel { color: #1E293B; font-size: 13px; }"
    ).arg(Theme::fontFamily()));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(16);

    // Title row
    auto *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(QStringLiteral("Transaction Details"), this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700; color: #0F172A;"));
    headerLayout->addWidget(m_titleLabel);

    headerLayout->addStretch(1);
    m_typeBadgeLabel = new QLabel(this);
    headerLayout->addWidget(m_typeBadgeLabel);
    mainLayout->addLayout(headerLayout);

    // Card frame for details
    auto *cardFrame = new QFrame(this);
    cardFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 8px; padding: 12px; }"
    ));
    auto *formLayout = new QFormLayout(cardFrame);
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    auto addField = [&](const QString &label, QLabel *&target) {
        target = new QLabel(QStringLiteral("—"), cardFrame);
        target->setStyleSheet(QStringLiteral("font-weight: 600; color: #0F172A;"));
        formLayout->addRow(label, target);
    };

    addField(QStringLiteral("Instrument / Asset:"), m_instrumentLabel);
    addField(QStringLiteral("Execution Date:"), m_dateLabel);
    addField(QStringLiteral("Quantity / Units:"), m_qtyLabel);
    addField(QStringLiteral("Execution Unit Price:"), m_priceLabel);
    addField(QStringLiteral("Gross Amount:"), m_grossLabel);
    addField(QStringLiteral("Brokerage & Taxes:"), m_chargesLabel);
    addField(QStringLiteral("Net Settlement Amount:"), m_netLabel);
    addField(QStringLiteral("Notes / Memo:"), m_notesLabel);

    mainLayout->addWidget(cardFrame);

    // Action Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    auto *deleteBtn = new QPushButton(QStringLiteral("Delete / Reverse"), this);
    deleteBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #FEF2F2; color: #DC2626; border: 1px solid #FECACA; border-radius: 6px; padding: 6px 14px; font-size: 12px; font-weight: 600; }"
        "QPushButton:hover { background: #FEE2E2; }"
    ));
    deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(deleteBtn, &QPushButton::clicked, this, &TransactionDetailsDialog::onDeleteClicked);
    btnLayout->addWidget(deleteBtn);

    btnLayout->addStretch(1);

    auto *editBtn = new QPushButton(QStringLiteral("Edit Transaction"), this);
    editBtn->setStyleSheet(Theme::secondaryButtonStyle());
    editBtn->setCursor(Qt::PointingHandCursor);
    connect(editBtn, &QPushButton::clicked, this, &TransactionDetailsDialog::onEditClicked);
    btnLayout->addWidget(editBtn);

    auto *closeBtn = new QPushButton(QStringLiteral("Close"), this);
    closeBtn->setStyleSheet(Theme::primaryButtonStyle());
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);
}

void TransactionDetailsDialog::loadData()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0 || m_transactionId <= 0) return;

    auto txOpt = Services::TransactionService::instance().getTransactionDetail(userId, m_transactionId);
    if (!txOpt) {
        QMessageBox::warning(this, QStringLiteral("Record Not Found"), QStringLiteral("Could not load transaction details."));
        reject();
        return;
    }

    const auto &tx = *txOpt;
    m_titleLabel->setText(QStringLiteral("Transaction #TXN-%1").arg(tx.id));

    // Type badge
    QString typeStr = QString::fromStdString(tx.transactionType);
    m_typeBadgeLabel->setText(typeStr);
    QString badgeBg = QStringLiteral("#DBEAFE");
    QString badgeFg = QStringLiteral("#1D4ED8");
    if (typeStr == QStringLiteral("SELL") || typeStr == QStringLiteral("REDEMPTION")) {
        badgeBg = QStringLiteral("#DCFCE7");
        badgeFg = QStringLiteral("#15803D");
    } else if (typeStr == QStringLiteral("DIVIDEND") || typeStr == QStringLiteral("INTEREST")) {
        badgeBg = QStringLiteral("#FEF3C7");
        badgeFg = QStringLiteral("#B45309");
    } else if (typeStr == QStringLiteral("FEE")) {
        badgeBg = QStringLiteral("#FEE2E2");
        badgeFg = QStringLiteral("#B91C1C");
    }
    m_typeBadgeLabel->setStyleSheet(QStringLiteral(
        "background-color: %1; color: %2; font-size: 11px; font-weight: 700; padding: 3px 8px; border-radius: 4px;"
    ).arg(badgeBg, badgeFg));

    m_instrumentLabel->setText(QString::fromStdString(tx.symbol.empty() ? tx.instrumentName : tx.symbol + " - " + tx.instrumentName));
    m_dateLabel->setText(QString::fromStdString(tx.transactionDate));
    m_qtyLabel->setText(QString::number(tx.quantity, 'f', 2) + QStringLiteral(" Units"));
    m_priceLabel->setText(formatMoney(tx.unitPriceCents));
    m_grossLabel->setText(formatMoney(tx.grossAmountCents));
    
    int64_t totalCharges = tx.feesCents + tx.taxesCents;
    m_chargesLabel->setText(formatMoney(totalCharges) + QStringLiteral(" (Fees: ") + formatMoney(tx.feesCents) + QStringLiteral(", Taxes: ") + formatMoney(tx.taxesCents) + QStringLiteral(")"));
    
    m_netLabel->setText(formatMoney(tx.netAmountCents));
    m_netLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 700; color: #16A34A;"));

    m_notesLabel->setText(QString::fromStdString(tx.notes.empty() ? "None recorded" : tx.notes));
}

void TransactionDetailsDialog::onEditClicked()
{
    auto *dialog = new TransactionDialog(m_transactionId, this);
    connect(dialog, &TransactionDialog::transactionSaved, this, [this]() {
        loadData();
        emit transactionChanged();
    });
    dialog->exec();
}

void TransactionDetailsDialog::onDeleteClicked()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    bool confirmed = ConfirmDialog::confirm(
        this,
        QStringLiteral("Confirm Transaction Reversal"),
        QStringLiteral("Are you sure you want to delete / reverse this transaction record? This will adjust holding balances, WAC cost basis, and realized P&L calculations."),
        QStringLiteral("Reverse / Delete"),
        true
    );

    if (confirmed) {
        auto res = Services::TransactionService::instance().deleteTransaction(userId, m_transactionId);
        if (res.first) {
            emit transactionDeleted();
            accept();
        } else {
            QMessageBox::warning(this, QStringLiteral("Cannot Delete Transaction"), QString::fromStdString(res.second));
        }
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

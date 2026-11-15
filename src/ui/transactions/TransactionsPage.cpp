#include "TransactionsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QDate>
#include <QMessageBox>

#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"
#include "TransactionService.h"
#include "TransactionDialog.h"
#include "TransactionDetailsDialog.h"
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

TransactionsPage::TransactionsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadTransactions();
}

void TransactionsPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(20);

    // 1. Summary Cards Row
    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);

    m_txnCountCard = new SummaryCard(
        QStringLiteral("Total Ledger Entries"),
        QStringLiteral("0 Records"),
        QStringLiteral("Audit Ledger"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Transactions,
        container
    );
    m_txnCountCard->setSubtitle(QStringLiteral("All-time trade and flow records"));
    summaryRow->addWidget(m_txnCountCard);

    m_inflowCard = new SummaryCard(
        QStringLiteral("Capital Outlay"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Purchases"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        container
    );
    m_inflowCard->setSubtitle(QStringLiteral("Total capital deployed"));
    summaryRow->addWidget(m_inflowCard);

    m_payoutCard = new SummaryCard(
        QStringLiteral("Disposals & Income"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Realized"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::TrendUp,
        container
    );
    m_payoutCard->setSubtitle(QStringLiteral("Sales proceeds & dividend/interest income"));
    summaryRow->addWidget(m_payoutCard);

    layout->addLayout(summaryRow);

    // 2. Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search transactions by symbol, notes, or reference..."),
        {
            QStringLiteral("All Types"),
            QStringLiteral("BUY"),
            QStringLiteral("SELL"),
            QStringLiteral("DIVIDEND"),
            QStringLiteral("INTEREST"),
            QStringLiteral("REDEMPTION"),
            QStringLiteral("DEPOSIT"),
            QStringLiteral("WITHDRAWAL"),
            QStringLiteral("FEE")
        },
        {
            QStringLiteral("All Periods"),
            QStringLiteral("Last 30 Days"),
            QStringLiteral("Last 90 Days"),
            QStringLiteral("This Year")
        },
        QStringLiteral("+ Record Transaction"),
        container
    );
    connect(m_filterBar, &FilterBar::searchChanged, this, &TransactionsPage::onSearchChanged);
    connect(m_filterBar, &FilterBar::filterChanged, this, &TransactionsPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::actionClicked, this, &TransactionsPage::onRecordTransactionClicked);
    layout->addWidget(m_filterBar);

    // 3. Transactions Table
    m_txnTable = new ModernTableWidget(container);
    m_txnTable->setHeaders({
        QStringLiteral("Date"),
        QStringLiteral("Reference"),
        QStringLiteral("Instrument"),
        QStringLiteral("Category"),
        QStringLiteral("Type"),
        QStringLiteral("Quantity"),
        QStringLiteral("Unit Price"),
        QStringLiteral("Charges / Tax"),
        QStringLiteral("Net Settlement"),
        QStringLiteral("Actions")
    });
    m_txnTable->setEmptyMessage(QStringLiteral("No financial transactions match the current filters. Click '+ Record Transaction' to add an entry."));
    m_txnTable->setMinimumHeight(380);
    m_txnTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_txnTable->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    layout->addWidget(m_txnTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void TransactionsPage::loadTransactions()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    QString query = m_filterBar ? m_filterBar->searchText().trimmed() : QString();
    QString typeSelected = m_filterBar ? m_filterBar->selectedCategory() : QString();
    QString periodSelected = m_filterBar ? m_filterBar->selectedStatus() : QString();

    std::string typeFilter = "ALL";
    if (typeSelected != QStringLiteral("All Types") && !typeSelected.isEmpty()) {
        typeFilter = typeSelected.toStdString();
    }

    std::string startDate;
    std::string endDate;
    QDate today = QDate::currentDate();
    if (periodSelected.contains(QStringLiteral("30 Days"))) {
        startDate = today.addDays(-30).toString(QStringLiteral("yyyy-MM-dd")).toStdString();
        endDate = today.toString(QStringLiteral("yyyy-MM-dd")).toStdString();
    } else if (periodSelected.contains(QStringLiteral("90 Days"))) {
        startDate = today.addDays(-90).toString(QStringLiteral("yyyy-MM-dd")).toStdString();
        endDate = today.toString(QStringLiteral("yyyy-MM-dd")).toStdString();
    } else if (periodSelected.contains(QStringLiteral("This Year"))) {
        startDate = QDate(today.year(), 1, 1).toString(QStringLiteral("yyyy-MM-dd")).toStdString();
        endDate = today.toString(QStringLiteral("yyyy-MM-dd")).toStdString();
    }

    m_currentItems = Services::TransactionService::instance().getTransactions(
        userId,
        query.toStdString(),
        typeFilter,
        startDate,
        endDate,
        "DATE",
        false
    );

    updateSummaryMetrics(m_currentItems);

    m_txnTable->setRowCount(static_cast<int>(m_currentItems.size()));
    for (int i = 0; i < static_cast<int>(m_currentItems.size()); ++i) {
        const auto &tx = m_currentItems[i];
        int txId = tx.id;

        // 0. Date
        m_txnTable->setTextCell(i, 0, QString::fromStdString(tx.transactionDate));

        // 1. Reference
        m_txnTable->setTextCell(i, 1, QStringLiteral("#TXN-%1").arg(tx.id));

        // 2. Instrument
        QString instDisplay = QString::fromStdString(tx.symbol.empty() ? tx.instrumentName : tx.symbol + " - " + tx.instrumentName);
        m_txnTable->setTextCell(i, 2, instDisplay);

        // 3. Category
        m_txnTable->setBadgeCell(i, 3, QString::fromStdString(tx.categoryName.empty() ? tx.categoryCode : tx.categoryName), StatusBadge::BadgeVariant::Neutral);

        // 4. Type
        StatusBadge::BadgeVariant typeBadge = StatusBadge::BadgeVariant::Neutral;
        if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
            typeBadge = StatusBadge::BadgeVariant::Primary;
        } else if (tx.transactionType == "SELL" || tx.transactionType == "REDEMPTION" || tx.transactionType == "WITHDRAWAL") {
            typeBadge = StatusBadge::BadgeVariant::Success;
        } else if (tx.transactionType == "DIVIDEND" || tx.transactionType == "INTEREST") {
            typeBadge = StatusBadge::BadgeVariant::Warning;
        } else if (tx.transactionType == "FEE") {
            typeBadge = StatusBadge::BadgeVariant::Danger;
        }
        m_txnTable->setBadgeCell(i, 4, QString::fromStdString(tx.transactionType), typeBadge);

        // 5. Quantity
        m_txnTable->setTextCell(i, 5, QString::number(tx.quantity, 'f', 2), Qt::AlignRight | Qt::AlignVCenter);

        // 6. Unit Price
        if (tx.unitPriceCents > 0) {
            m_txnTable->setTextCell(i, 6, formatMoney(tx.unitPriceCents), Qt::AlignRight | Qt::AlignVCenter);
        } else {
            m_txnTable->setTextCell(i, 6, QStringLiteral("—"), Qt::AlignCenter);
        }

        // 7. Charges / Tax
        int64_t totalCharges = tx.feesCents + tx.taxesCents;
        m_txnTable->setTextCell(i, 7, formatMoney(totalCharges), Qt::AlignRight | Qt::AlignVCenter);

        // 8. Net Settlement
        m_txnTable->setTextCell(i, 8, formatMoney(tx.netAmountCents), Qt::AlignRight | Qt::AlignVCenter);

        // 9. Actions
        auto *actionWidget = new QWidget(m_txnTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(6);

        auto *viewBtn = new QPushButton(QStringLiteral("View"), actionWidget);
        viewBtn->setCursor(Qt::PointingHandCursor);
        viewBtn->setFixedHeight(26);
        viewBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #EFF6FF; color: #1D4ED8; border: 1px solid #BFDBFE; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #DBEAFE; }"
        ));
        connect(viewBtn, &QPushButton::clicked, this, [this, txId]() { onViewTransactionClicked(txId); });
        actionLayout->addWidget(viewBtn);

        auto *editBtn = new QPushButton(QStringLiteral("Edit"), actionWidget);
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setFixedHeight(26);
        editBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #F8FAFC; color: #374151; border: 1px solid #D1D5DB; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #F3F4F6; }"
        ));
        connect(editBtn, &QPushButton::clicked, this, [this, txId]() { onEditTransactionClicked(txId); });
        actionLayout->addWidget(editBtn);

        auto *delBtn = new QPushButton(QStringLiteral("Delete"), actionWidget);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setFixedHeight(26);
        delBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #FEF2F2; color: #DC2626; border: 1px solid #FECACA; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #FEE2E2; }"
        ));
        connect(delBtn, &QPushButton::clicked, this, [this, txId]() { onDeleteTransactionClicked(txId); });
        actionLayout->addWidget(delBtn);

        m_txnTable->setCellWidget(i, 9, actionWidget);
    }
}

void TransactionsPage::updateSummaryMetrics(const std::vector<Models::Transaction> &items)
{
    int64_t totalOutlay = 0;
    int64_t totalProceedsIncome = 0;

    for (const auto &tx : items) {
        if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
            totalOutlay += tx.netAmountCents;
        } else if (tx.transactionType == "SELL" || tx.transactionType == "REDEMPTION" || tx.transactionType == "WITHDRAWAL") {
            totalProceedsIncome += tx.netAmountCents;
        } else if (tx.transactionType == "DIVIDEND" || tx.transactionType == "INTEREST") {
            totalProceedsIncome += tx.netAmountCents;
        }
    }

    m_txnCountCard->setValue(QString::number(items.size()) + QStringLiteral(" Records"));
    m_txnCountCard->setSubtitle(QStringLiteral("Matching active criteria"));

    m_inflowCard->setValue(formatMoney(totalOutlay));
    m_payoutCard->setValue(formatMoney(totalProceedsIncome));
}

void TransactionsPage::onSearchChanged(const QString &)
{
    loadTransactions();
}

void TransactionsPage::onFilterChanged()
{
    loadTransactions();
}

void TransactionsPage::onRecordTransactionClicked()
{
    auto *dialog = new TransactionDialog(this);
    connect(dialog, &TransactionDialog::transactionSaved, this, [this]() {
        loadTransactions();
        emit transactionsChanged();
    });
    dialog->exec();
}

void TransactionsPage::onViewTransactionClicked(int transactionId)
{
    auto *dialog = new TransactionDetailsDialog(transactionId, this);
    connect(dialog, &TransactionDetailsDialog::transactionChanged, this, [this]() {
        loadTransactions();
        emit transactionsChanged();
    });
    connect(dialog, &TransactionDetailsDialog::transactionDeleted, this, [this]() {
        loadTransactions();
        emit transactionsChanged();
    });
    dialog->exec();
}

void TransactionsPage::onEditTransactionClicked(int transactionId)
{
    auto *dialog = new TransactionDialog(transactionId, this);
    connect(dialog, &TransactionDialog::transactionSaved, this, [this]() {
        loadTransactions();
        emit transactionsChanged();
    });
    dialog->exec();
}

void TransactionsPage::onDeleteTransactionClicked(int transactionId)
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    bool confirmed = ConfirmDialog::confirm(
        this,
        QStringLiteral("Confirm Transaction Deletion / Reversal"),
        QStringLiteral("Are you sure you want to permanently delete/reverse transaction #TXN-%1?\n\n"
                       "This will automatically adjust your remaining holding quantity, WAC cost basis, and realized P&L.").arg(transactionId),
        QStringLiteral("Delete Transaction"),
        true
    );

    if (confirmed) {
        auto res = Services::TransactionService::instance().deleteTransaction(userId, transactionId);
        if (res.first) {
            loadTransactions();
            emit transactionsChanged();
        } else {
            QMessageBox::warning(this, QStringLiteral("Cannot Delete Transaction"), QString::fromStdString(res.second));
        }
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

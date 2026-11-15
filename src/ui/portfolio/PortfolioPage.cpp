#include "PortfolioPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>

#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"
#include "StatusBadge.h"
#include "ConfirmDialog.h"
#include "InvestmentDialog.h"
#include "InvestmentDetailsDialog.h"

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

PortfolioPage::PortfolioPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadInvestments();
}

void PortfolioPage::setupUi()
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

    m_totalCard = new SummaryCard(
        QStringLiteral("Total Portfolio Cost"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Initial Invested"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        container
    );
    m_totalCard->setSubtitle(QStringLiteral("Consolidated across all holdings"));
    summaryRow->addWidget(m_totalCard);

    m_equitiesCard = new SummaryCard(
        QStringLiteral("Equities & MF"),
        QStringLiteral("₹0.00"),
        QStringLiteral("0.0%"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Stocks,
        container
    );
    m_equitiesCard->setSubtitle(QStringLiteral("Stocks & Mutual Funds"));
    summaryRow->addWidget(m_equitiesCard);

    m_debtGoldCard = new SummaryCard(
        QStringLiteral("Debt & Precious Metals"),
        QStringLiteral("₹0.00"),
        QStringLiteral("0.0%"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Gold,
        container
    );
    m_debtGoldCard->setSubtitle(QStringLiteral("Gold, Fixed Deposits & Bonds"));
    summaryRow->addWidget(m_debtGoldCard);

    layout->addLayout(summaryRow);

    // 2. Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search portfolio by name, symbol, or notes..."),
        {
            QStringLiteral("All Asset Classes"),
            QStringLiteral("Stocks"),
            QStringLiteral("Mutual Funds"),
            QStringLiteral("Gold"),
            QStringLiteral("Fixed Deposits"),
            QStringLiteral("Bonds")
        },
        {
            QStringLiteral("Active Holdings"),
            QStringLiteral("Sold / Closed"),
            QStringLiteral("All Statuses")
        },
        QStringLiteral("+ Add Investment"),
        container
    );
    connect(m_filterBar, &FilterBar::searchChanged, this, &PortfolioPage::onSearchChanged);
    connect(m_filterBar, &FilterBar::filterChanged, this, &PortfolioPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::actionClicked, this, &PortfolioPage::onAddClicked);
    layout->addWidget(m_filterBar);

    // 3. Master Holdings Table
    m_holdingsTable = new ModernTableWidget(container);
    m_holdingsTable->setHeaders({
        QStringLiteral("Asset / Symbol"),
        QStringLiteral("Asset Class"),
        QStringLiteral("Holding Qty"),
        QStringLiteral("Avg Cost"),
        QStringLiteral("Current Price / NAV"),
        QStringLiteral("Market Value"),
        QStringLiteral("Unrealized P&L"),
        QStringLiteral("Status"),
        QStringLiteral("Actions")
    });
    m_holdingsTable->setEmptyMessage(QStringLiteral("Your portfolio is empty. Click '+ Add Investment' to record your first asset holding."));
    m_holdingsTable->setMinimumHeight(380);
    m_holdingsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_holdingsTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    layout->addWidget(m_holdingsTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void PortfolioPage::loadInvestments()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) {
        return;
    }

    QString query = m_filterBar ? m_filterBar->searchText().trimmed() : QString();
    QString catSelected = m_filterBar ? m_filterBar->selectedCategory() : QString();
    QString statusSelected = m_filterBar ? m_filterBar->selectedStatus() : QString();

    std::string categoryFilter = "ALL";
    if (catSelected.contains(QStringLiteral("Stock"))) {
        categoryFilter = "STOCK";
    } else if (catSelected.contains(QStringLiteral("Mutual Fund"))) {
        categoryFilter = "MUTUAL_FUND";
    } else if (catSelected.contains(QStringLiteral("Gold"))) {
        categoryFilter = "GOLD";
    } else if (catSelected.contains(QStringLiteral("Fixed Deposit"))) {
        categoryFilter = "FIXED_DEPOSIT";
    } else if (catSelected.contains(QStringLiteral("Bond"))) {
        categoryFilter = "BOND";
    }

    std::string statusFilter = "ACTIVE";
    if (statusSelected.contains(QStringLiteral("Sold")) || statusSelected.contains(QStringLiteral("Closed"))) {
        statusFilter = "SOLD";
    } else if (statusSelected.contains(QStringLiteral("All"))) {
        statusFilter = "ALL";
    }

    m_currentItems = Services::InvestmentService::instance().getUserInvestments(
        userId,
        query.toStdString(),
        categoryFilter,
        statusFilter,
        "NAME",
        true
    );

    updateSummaryMetrics(m_currentItems);

    m_holdingsTable->setRowCount(static_cast<int>(m_currentItems.size()));
    for (int i = 0; i < static_cast<int>(m_currentItems.size()); ++i) {
        const auto &item = m_currentItems[i];
        int invId = item.investmentId;

        // 0. Asset / Symbol
        QString displayAsset = QString::fromStdString(item.symbol.empty() ? item.name : item.symbol + " - " + item.name);
        m_holdingsTable->setTextCell(i, 0, displayAsset);

        // 1. Asset Class Badge
        StatusBadge::BadgeVariant catBadge = StatusBadge::BadgeVariant::Neutral;
        if (item.categoryCode == "STOCK") catBadge = StatusBadge::BadgeVariant::Primary;
        else if (item.categoryCode == "MUTUAL_FUND") catBadge = StatusBadge::BadgeVariant::Success;
        else if (item.categoryCode == "GOLD") catBadge = StatusBadge::BadgeVariant::Warning;
        else if (item.categoryCode == "FIXED_DEPOSIT") catBadge = StatusBadge::BadgeVariant::Neutral;
        else if (item.categoryCode == "BOND") catBadge = StatusBadge::BadgeVariant::Primary;
        m_holdingsTable->setBadgeCell(i, 1, QString::fromStdString(item.categoryName), catBadge);

        // 2. Holding Quantity
        m_holdingsTable->setTextCell(i, 2, QString::number(item.quantity, 'f', 2), Qt::AlignRight | Qt::AlignVCenter);

        // 3. Avg Cost
        int64_t avgCostCents = 0;
        if (item.quantity > 0.00001) {
            avgCostCents = static_cast<int64_t>(item.investedAmountCents / item.quantity);
        }
        m_holdingsTable->setTextCell(i, 3, formatMoney(avgCostCents), Qt::AlignRight | Qt::AlignVCenter);

        // 4. Current Price / NAV (Strictly non-fabricated)
        if (item.hasMarketPrice) {
            m_holdingsTable->setTextCell(i, 4, formatMoney(item.latestPriceCents), Qt::AlignRight | Qt::AlignVCenter);
        } else {
            m_holdingsTable->setTextCell(i, 4, QStringLiteral("Unavailable"), Qt::AlignCenter);
        }

        // 5. Market Value (Strictly non-fabricated)
        if (item.hasCurrentValue) {
            m_holdingsTable->setTextCell(i, 5, formatMoney(item.currentValueCents), Qt::AlignRight | Qt::AlignVCenter);
        } else {
            m_holdingsTable->setTextCell(i, 5, QStringLiteral("Unavailable"), Qt::AlignCenter);
        }

        // 6. Unrealized P&L
        if (item.hasPnl) {
            QString pnlStr = formatMoney(item.unrealizedPnlCents) + QStringLiteral(" (") +
                (item.unrealizedPnlPercent >= 0 ? QStringLiteral("+") : QString()) +
                QString::number(item.unrealizedPnlPercent, 'f', 2) + QStringLiteral("%)");
            m_holdingsTable->setBadgeCell(
                i, 6, pnlStr,
                item.unrealizedPnlCents >= 0 ? StatusBadge::BadgeVariant::Success : StatusBadge::BadgeVariant::Danger
            );
        } else {
            m_holdingsTable->setTextCell(i, 6, QStringLiteral("N/A"), Qt::AlignCenter);
        }

        // 7. Status Badge
        StatusBadge::BadgeVariant statusVariant = StatusBadge::BadgeVariant::Neutral;
        if (item.status == "ACTIVE") {
            statusVariant = StatusBadge::BadgeVariant::Success;
        } else if (item.status == "SOLD") {
            statusVariant = StatusBadge::BadgeVariant::Danger;
        }
        m_holdingsTable->setBadgeCell(i, 7, QString::fromStdString(item.status), statusVariant);

        // 8. Actions (View, Edit, Delete)
        auto *actionWidget = new QWidget(m_holdingsTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(6, 2, 6, 2);
        actionLayout->setSpacing(6);

        auto *viewBtn = new QPushButton(QStringLiteral("View"), actionWidget);
        viewBtn->setCursor(Qt::PointingHandCursor);
        viewBtn->setFixedHeight(26);
        viewBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #EFF6FF; color: #1D4ED8; border: 1px solid #BFDBFE; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #DBEAFE; }"
        ));
        connect(viewBtn, &QPushButton::clicked, this, [this, invId]() { onViewClicked(invId); });
        actionLayout->addWidget(viewBtn);

        auto *editBtn = new QPushButton(QStringLiteral("Edit"), actionWidget);
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setFixedHeight(26);
        editBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #F8FAFC; color: #374151; border: 1px solid #D1D5DB; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #F3F4F6; }"
        ));
        connect(editBtn, &QPushButton::clicked, this, [this, invId]() { onEditClicked(invId); });
        actionLayout->addWidget(editBtn);

        auto *delBtn = new QPushButton(QStringLiteral("Delete"), actionWidget);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setFixedHeight(26);
        delBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #FEF2F2; color: #DC2626; border: 1px solid #FECACA; border-radius: 4px; font-size: 11px; font-weight: 600; padding: 0 8px; }"
            "QPushButton:hover { background: #FEE2E2; }"
        ));
        connect(delBtn, &QPushButton::clicked, this, [this, invId]() { onDeleteClicked(invId); });
        actionLayout->addWidget(delBtn);

        m_holdingsTable->setCellWidget(i, 8, actionWidget);
    }
}

void PortfolioPage::updateSummaryMetrics(const std::vector<Services::InvestmentListItemDTO> &items)
{
    int64_t totalCostCents = 0;
    int64_t equitiesCostCents = 0;
    int64_t debtGoldCostCents = 0;

    for (const auto &item : items) {
        totalCostCents += item.investedAmountCents;
        if (item.categoryCode == "STOCK" || item.categoryCode == "MUTUAL_FUND") {
            equitiesCostCents += item.investedAmountCents;
        } else {
            debtGoldCostCents += item.investedAmountCents;
        }
    }

    m_totalCard->setValue(formatMoney(totalCostCents));
    m_totalCard->setSubtitle(QString::number(items.size()) + QStringLiteral(" holding(s) listed"));

    m_equitiesCard->setValue(formatMoney(equitiesCostCents));
    double eqPct = (totalCostCents > 0) ? (static_cast<double>(equitiesCostCents) / totalCostCents * 100.0) : 0.0;
    m_equitiesCard->setTrend(QString::number(eqPct, 'f', 1) + QStringLiteral("%"), SummaryCard::TrendType::Neutral);

    m_debtGoldCard->setValue(formatMoney(debtGoldCostCents));
    double dgPct = (totalCostCents > 0) ? (static_cast<double>(debtGoldCostCents) / totalCostCents * 100.0) : 0.0;
    m_debtGoldCard->setTrend(QString::number(dgPct, 'f', 1) + QStringLiteral("%"), SummaryCard::TrendType::Neutral);
}

void PortfolioPage::onSearchChanged(const QString &)
{
    loadInvestments();
}

void PortfolioPage::onFilterChanged()
{
    loadInvestments();
}

void PortfolioPage::onAddClicked()
{
    auto *dialog = new InvestmentDialog(this);
    connect(dialog, &InvestmentDialog::investmentSaved, this, [this]() {
        loadInvestments();
        emit investmentsChanged();
    });
    dialog->exec();
}

void PortfolioPage::onViewClicked(int investmentId)
{
    auto *dialog = new InvestmentDetailsDialog(investmentId, this);
    connect(dialog, &InvestmentDetailsDialog::investmentChanged, this, [this]() {
        loadInvestments();
        emit investmentsChanged();
    });
    connect(dialog, &InvestmentDetailsDialog::investmentDeleted, this, [this]() {
        loadInvestments();
        emit investmentsChanged();
    });
    dialog->exec();
}

void PortfolioPage::onEditClicked(int investmentId)
{
    auto *dialog = new InvestmentDialog(investmentId, this);
    connect(dialog, &InvestmentDialog::investmentSaved, this, [this]() {
        loadInvestments();
        emit investmentsChanged();
    });
    dialog->exec();
}

void PortfolioPage::onDeleteClicked(int investmentId)
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    int txCount = Services::InvestmentService::instance().getTransactionCount(userId, investmentId);
    bool confirmed = false;
    bool cascade = false;

    if (txCount > 0) {
        confirmed = ConfirmDialog::confirm(
            this,
            QStringLiteral("Restricted Deletion Warning"),
            QStringLiteral("This investment has %1 linked transaction ledger record(s).\n\n"
                           "In accordance with relational integrity policies, deleting it will also remove its associated transaction history and cash flow events.\n\n"
                           "Do you wish to proceed with cascading deletion?").arg(txCount),
            QStringLiteral("Delete All Records"),
            true
        );
        cascade = true;
    } else {
        confirmed = ConfirmDialog::confirm(
            this,
            QStringLiteral("Confirm Investment Deletion"),
            QStringLiteral("Are you sure you want to permanently delete this investment holding?"),
            QStringLiteral("Delete Holding"),
            true
        );
        cascade = false;
    }

    if (confirmed) {
        auto res = Services::InvestmentService::instance().deleteInvestment(userId, investmentId, cascade);
        if (res.first) {
            loadInvestments();
            emit investmentsChanged();
        }
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

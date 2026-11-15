#include "DashboardPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"
#include "DashboardService.h"
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

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadDashboardData();
}

void DashboardPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(24);

    // 1. Quick Actions Bar
    layout->addWidget(createQuickActionsBar());

    // 2. Top Summary Cards (5 Cards: Total Invested, Current Value, Unrealized P&L, Realized P&L, Active Holdings)
    layout->addWidget(createSummaryCardsRow());

    // 3. Chart Area (Asset Allocation Donut & Portfolio Value History)
    layout->addWidget(createChartsRow());

    // 4. Top Holdings Table Section
    layout->addWidget(createTopHoldingsSection());

    // 5. Activity Area (Recent Transactions & Upcoming Financial Events)
    layout->addWidget(createActivityRow());

    layout->addStretch(1);
    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

QWidget* DashboardPage::createQuickActionsBar()
{
    auto *bar = new QFrame(this);
    bar->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 10px;"
        "   padding: 4px 8px;"
        "}"
    ));

    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(12);

    auto *label = new QLabel(QStringLiteral("QUICK ACTIONS:"), bar);
    label->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 11px; font-weight: 700; color: #64748B; letter-spacing: 0.5px;"
    ).arg(Theme::fontFamily()));
    layout->addWidget(label);

    auto createActionBtn = [&](const QString &text, IconHelper::IconType icon, bool primary = false) {
        auto *btn = new QPushButton(text, bar);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(32);
        btn->setIcon(IconHelper::getIcon(icon, primary ? Qt::white : QColor(37, 99, 235), 14));
        btn->setStyleSheet(primary ? Theme::primaryButtonStyle() : Theme::secondaryButtonStyle());
        return btn;
    };

    auto *addInvBtn = createActionBtn(QStringLiteral("+ Add Investment"), IconHelper::IconType::Plus, true);
    connect(addInvBtn, &QPushButton::clicked, this, &DashboardPage::addInvestmentRequested);
    layout->addWidget(addInvBtn);

    auto *recTxnBtn = createActionBtn(QStringLiteral("Record Transaction"), IconHelper::IconType::Transactions);
    connect(recTxnBtn, &QPushButton::clicked, this, &DashboardPage::recordTransactionRequested);
    layout->addWidget(recTxnBtn);

    auto *sipBtn = createActionBtn(QStringLiteral("SIP Calculator"), IconHelper::IconType::SipCalculator);
    connect(sipBtn, &QPushButton::clicked, this, &DashboardPage::openSipCalculatorRequested);
    layout->addWidget(sipBtn);

    auto *swpBtn = createActionBtn(QStringLiteral("SWP Calculator"), IconHelper::IconType::SwpCalculator);
    connect(swpBtn, &QPushButton::clicked, this, &DashboardPage::openSwpCalculatorRequested);
    layout->addWidget(swpBtn);

    layout->addStretch(1);

    auto *refreshBtn = new QPushButton(QStringLiteral(" Refresh"), bar);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setFixedHeight(32);
    refreshBtn->setIcon(IconHelper::getIcon(IconHelper::IconType::Analytics, QColor(75, 85, 99), 13));
    refreshBtn->setStyleSheet(Theme::secondaryButtonStyle());
    refreshBtn->setToolTip(QStringLiteral("Reload real-time calculations from SQLite database"));
    connect(refreshBtn, &QPushButton::clicked, this, &DashboardPage::loadDashboardData);
    layout->addWidget(refreshBtn);

    return bar;
}

QWidget* DashboardPage::createSummaryCardsRow()
{
    auto *row = new QWidget(this);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);

    // 1. Total Invested
    m_totalInvestedCard = new SummaryCard(
        QStringLiteral("Total Invested"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Initial Cost"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        row
    );
    m_totalInvestedCard->setSubtitle(QStringLiteral("Cumulative capital across assets"));
    layout->addWidget(m_totalInvestedCard);

    // 2. Current Estimated Value
    m_currentValueCard = new SummaryCard(
        QStringLiteral("Current Estimated Value"),
        QStringLiteral("₹0.00"),
        QStringLiteral("0.0%"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Analytics,
        row
    );
    m_currentValueCard->setSubtitle(QStringLiteral("Real-time valuation"));
    layout->addWidget(m_currentValueCard);

    // 3. Unrealized P&L
    m_unrealizedPnlCard = new SummaryCard(
        QStringLiteral("Unrealized Profit/Loss"),
        QStringLiteral("₹0.00"),
        QStringLiteral("0.0%"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::TrendUp,
        row
    );
    m_unrealizedPnlCard->setSubtitle(QStringLiteral("Holding paper returns"));
    layout->addWidget(m_unrealizedPnlCard);

    // 4. Realized P&L
    m_realizedPnlCard = new SummaryCard(
        QStringLiteral("Realized Profit/Loss"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Closed Trades"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Bonds,
        row
    );
    m_realizedPnlCard->setSubtitle(QStringLiteral("Closed position returns"));
    layout->addWidget(m_realizedPnlCard);

    // 5. Active Holdings
    m_activeHoldingsCard = new SummaryCard(
        QStringLiteral("Active Holdings"),
        QStringLiteral("0"),
        QStringLiteral("Portfolio"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Stocks,
        row
    );
    m_activeHoldingsCard->setSubtitle(QStringLiteral("Across 5 asset classes"));
    layout->addWidget(m_activeHoldingsCard);

    return row;
}

QWidget* DashboardPage::createChartsRow()
{
    auto *row = new QWidget(this);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // Asset Allocation vector Donut Chart
    m_allocationChart = new ChartPlaceholder(
        QStringLiteral("Asset Allocation Breakdown"),
        ChartPlaceholder::ChartType::PieAllocation,
        QStringLiteral("Asset distribution chart ready for portfolio records."),
        row
    );
    layout->addWidget(m_allocationChart, 1);

    // Growth trajectory line chart
    m_historyChart = new ChartPlaceholder(
        QStringLiteral("Portfolio Growth Trajectory"),
        ChartPlaceholder::ChartType::LineTrend,
        QStringLiteral("Historical trajectory will chart automatically as transactions occur."),
        row
    );
    layout->addWidget(m_historyChart, 2);

    return row;
}

QWidget* DashboardPage::createTopHoldingsSection()
{
    auto *section = new QWidget(this);
    auto *layout = new QVBoxLayout(section);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto *header = new SectionHeader(
        QStringLiteral("Top Portfolio Holdings"),
        QStringLiteral("Major positions sorted by invested capital and market value"),
        QString(),
        section
    );
    layout->addWidget(header);

    m_topHoldingsTable = new ModernTableWidget(section);
    m_topHoldingsTable->setHeaders({
        QStringLiteral("Asset / Symbol"),
        QStringLiteral("Category"),
        QStringLiteral("Quantity"),
        QStringLiteral("Invested Capital"),
        QStringLiteral("Current Value"),
        QStringLiteral("Unrealized P&L")
    });
    m_topHoldingsTable->setEmptyMessage(QStringLiteral("No portfolio holdings recorded yet. Use + Add Investment to get started."));
    m_topHoldingsTable->setMinimumHeight(170);
    layout->addWidget(m_topHoldingsTable);

    return section;
}

QWidget* DashboardPage::createActivityRow()
{
    auto *row = new QWidget(this);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // Left Column: Recent Transactions
    auto *leftCol = new QWidget(row);
    auto *leftLayout = new QVBoxLayout(leftCol);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    auto *txHeader = new SectionHeader(QStringLiteral("Recent Transactions"),
                                       QStringLiteral("Latest buy/sell ledger activities"),
                                       QStringLiteral("Record New"),
                                       leftCol);
    connect(txHeader, &SectionHeader::actionClicked, this, &DashboardPage::recordTransactionRequested);
    leftLayout->addWidget(txHeader);

    m_recentTransactionsTable = new ModernTableWidget(leftCol);
    m_recentTransactionsTable->setHeaders({QStringLiteral("Date"), QStringLiteral("Instrument"), QStringLiteral("Type"), QStringLiteral("Units"), QStringLiteral("Amount")});
    m_recentTransactionsTable->setEmptyMessage(QStringLiteral("Your portfolio is ready to begin. No transactions recorded yet."));
    m_recentTransactionsTable->setMinimumHeight(180);
    leftLayout->addWidget(m_recentTransactionsTable);
    layout->addWidget(leftCol, 3);

    // Right Column: Upcoming Financial Events & Maturities
    auto *rightCol = new QWidget(row);
    auto *rightLayout = new QVBoxLayout(rightCol);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    auto *evHeader = new SectionHeader(QStringLiteral("Upcoming Financial Events"),
                                       QStringLiteral("FD maturities, SIP dates, & bond coupons"),
                                       QStringLiteral("Tracker"),
                                       rightCol);
    rightLayout->addWidget(evHeader);

    m_upcomingEventsTable = new ModernTableWidget(rightCol);
    m_upcomingEventsTable->setHeaders({QStringLiteral("Date"), QStringLiteral("Event Type"), QStringLiteral("Details"), QStringLiteral("Est. Flow")});
    m_upcomingEventsTable->setEmptyMessage(QStringLiteral("No upcoming cash flow events or maturities in the next 30 days."));
    m_upcomingEventsTable->setMinimumHeight(180);
    rightLayout->addWidget(m_upcomingEventsTable);
    layout->addWidget(rightCol, 2);

    return row;
}

void DashboardPage::loadDashboardData()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) {
        return;
    }

    auto data = Services::DashboardService::instance().getDashboardData(userId);

    // 1. Total Invested Card
    m_totalInvestedCard->setValue(formatMoney(data.summary.totalInvestedCents));

    // 2. Current Estimated Value Card (Strict non-fabrication)
    if (data.summary.hasValuationData) {
        m_currentValueCard->setValue(formatMoney(data.summary.currentEstimatedValueCents));
        double deltaVal = data.summary.unrealizedPnlPercent;
        QString deltaTxt = (deltaVal >= 0 ? QStringLiteral("+") : QString()) + QString::number(deltaVal, 'f', 2) + QStringLiteral("%");
        m_currentValueCard->setTrend(deltaTxt, deltaVal >= 0 ? SummaryCard::TrendType::Positive : SummaryCard::TrendType::Negative);
    } else {
        m_currentValueCard->setValue(QStringLiteral("Unavailable"));
        m_currentValueCard->setTrend(QStringLiteral("Pending"), SummaryCard::TrendType::Neutral);
    }

    // 3. Unrealized P&L Card
    if (data.summary.hasValuationData) {
        m_unrealizedPnlCard->setValue(formatMoney(data.summary.unrealizedPnlCents));
        QString pnlDelta = (data.summary.unrealizedPnlPercent >= 0 ? QStringLiteral("+") : QString()) + QString::number(data.summary.unrealizedPnlPercent, 'f', 2) + QStringLiteral("%");
        m_unrealizedPnlCard->setTrend(
            pnlDelta,
            data.summary.unrealizedPnlCents > 0 ? SummaryCard::TrendType::Positive :
            (data.summary.unrealizedPnlCents < 0 ? SummaryCard::TrendType::Negative : SummaryCard::TrendType::Neutral)
        );
    } else {
        m_unrealizedPnlCard->setValue(QStringLiteral("N/A"));
        m_unrealizedPnlCard->setTrend(QStringLiteral("Unquoted"), SummaryCard::TrendType::Neutral);
    }

    // 4. Realized P&L Card
    m_realizedPnlCard->setValue(formatMoney(data.summary.realizedPnlCents));
    m_realizedPnlCard->setTrend(
        data.summary.realizedPnlCents >= 0 ? QStringLiteral("Net Gain") : QStringLiteral("Net Loss"),
        data.summary.realizedPnlCents > 0 ? SummaryCard::TrendType::Positive :
        (data.summary.realizedPnlCents < 0 ? SummaryCard::TrendType::Negative : SummaryCard::TrendType::Neutral)
    );

    // 5. Active Holdings Card
    m_activeHoldingsCard->setValue(QString::number(data.summary.activeHoldingsCount));
    QString unquotedNote = data.summary.unquotedHoldingsCount > 0 ?
        QStringLiteral("%1 unquoted").arg(data.summary.unquotedHoldingsCount) :
        QStringLiteral("All quoted");
    m_activeHoldingsCard->setTrend(QString::number(data.summary.activeHoldingsCount) + QStringLiteral(" Active"), SummaryCard::TrendType::Neutral);
    m_activeHoldingsCard->setSubtitle(unquotedNote);

    // 6. Vector Donut Allocation Chart
    QVector<ChartPlaceholder::SliceData> slices;
    for (const auto &item : data.allocation) {
        ChartPlaceholder::SliceData slice;
        slice.label = QString::fromStdString(item.categoryName);
        slice.percentage = item.percentage;
        slice.value = item.investedAmountCents / 100.0;
        slice.color = QColor(QString::fromStdString(item.colorHex));
        slices.append(slice);
    }
    m_allocationChart->setAllocationSlices(slices);

    // 7. Top Holdings Table
    m_topHoldingsTable->setRowCount(static_cast<int>(data.topHoldings.size()));
    for (int i = 0; i < static_cast<int>(data.topHoldings.size()); ++i) {
        const auto &h = data.topHoldings[i];
        QString instrumentName = QString::fromStdString(h.symbol.empty() ? h.name : h.symbol + " - " + h.name);
        m_topHoldingsTable->setTextCell(i, 0, instrumentName);
        m_topHoldingsTable->setBadgeCell(i, 1, QString::fromStdString(h.categoryName), StatusBadge::BadgeVariant::Neutral);
        m_topHoldingsTable->setTextCell(i, 2, QString::number(h.quantity, 'f', 2), Qt::AlignRight | Qt::AlignVCenter);
        m_topHoldingsTable->setTextCell(i, 3, formatMoney(h.investedCostCents), Qt::AlignRight | Qt::AlignVCenter);
        
        if (h.hasValuation) {
            m_topHoldingsTable->setTextCell(i, 4, formatMoney(h.currentValueCents), Qt::AlignRight | Qt::AlignVCenter);
        } else {
            m_topHoldingsTable->setTextCell(i, 4, QStringLiteral("Unavailable"), Qt::AlignCenter);
        }

        if (h.hasPnl) {
            QString pnlStr = formatMoney(h.unrealizedPnlCents) + QStringLiteral(" (") +
                (h.unrealizedPnlPercent >= 0 ? QStringLiteral("+") : QString()) +
                QString::number(h.unrealizedPnlPercent, 'f', 2) + QStringLiteral("%)");
            m_topHoldingsTable->setBadgeCell(
                i, 5, pnlStr,
                h.unrealizedPnlCents >= 0 ? StatusBadge::BadgeVariant::Success : StatusBadge::BadgeVariant::Danger
            );
        } else {
            m_topHoldingsTable->setTextCell(i, 5, QStringLiteral("N/A"), Qt::AlignCenter);
        }
    }

    // 8. Recent Transactions Table
    m_recentTransactionsTable->setRowCount(static_cast<int>(data.recentTransactions.size()));
    for (int i = 0; i < static_cast<int>(data.recentTransactions.size()); ++i) {
        const auto &tx = data.recentTransactions[i];
        m_recentTransactionsTable->setTextCell(i, 0, QString::fromStdString(tx.date));
        m_recentTransactionsTable->setTextCell(i, 1, QString::fromStdString(tx.symbol.empty() ? tx.instrumentName : tx.symbol));
        
        StatusBadge::BadgeVariant badgeVar = StatusBadge::BadgeVariant::Neutral;
        if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
            badgeVar = StatusBadge::BadgeVariant::Primary;
        } else if (tx.transactionType == "SELL" || tx.transactionType == "WITHDRAWAL") {
            badgeVar = StatusBadge::BadgeVariant::Success;
        } else if (tx.transactionType == "DIVIDEND" || tx.transactionType == "INTEREST") {
            badgeVar = StatusBadge::BadgeVariant::Warning;
        }
        m_recentTransactionsTable->setBadgeCell(i, 2, QString::fromStdString(tx.transactionType), badgeVar);
        m_recentTransactionsTable->setTextCell(i, 3, QString::number(tx.quantity, 'f', 2), Qt::AlignRight | Qt::AlignVCenter);
        m_recentTransactionsTable->setTextCell(i, 4, formatMoney(tx.netAmountCents), Qt::AlignRight | Qt::AlignVCenter);
    }

    // 9. Upcoming Events Table
    m_upcomingEventsTable->setRowCount(static_cast<int>(data.upcomingEvents.size()));
    for (int i = 0; i < static_cast<int>(data.upcomingEvents.size()); ++i) {
        const auto &ev = data.upcomingEvents[i];
        m_upcomingEventsTable->setTextCell(i, 0, QString::fromStdString(ev.eventDate));
        m_upcomingEventsTable->setBadgeCell(i, 1, QString::fromStdString(ev.typeDisplay), StatusBadge::BadgeVariant::Warning);
        m_upcomingEventsTable->setTextCell(i, 2, QString::fromStdString(ev.description));
        m_upcomingEventsTable->setTextCell(i, 3, formatMoney(ev.amountCents), Qt::AlignRight | Qt::AlignVCenter);
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

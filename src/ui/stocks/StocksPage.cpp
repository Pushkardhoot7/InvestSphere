#include "StocksPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

StocksPage::StocksPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void StocksPage::setupUi()
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

    // Summary Cards Row
    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);

    auto *totalStockCard = new SummaryCard(QStringLiteral("Total Equity Value"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Stocks, container);
    totalStockCard->setSubtitle(QStringLiteral("0 active stock positions"));
    summaryRow->addWidget(totalStockCard);

    auto *dayGainCard = new SummaryCard(QStringLiteral("Today's Equity Change"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::TrendUp, container);
    dayGainCard->setSubtitle(QStringLiteral("Real-time trading day change"));
    summaryRow->addWidget(dayGainCard);

    auto *totalGainCard = new SummaryCard(QStringLiteral("Overall Equity Returns"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Analytics, container);
    totalGainCard->setSubtitle(QStringLiteral("Cumulative unrealized profit"));
    summaryRow->addWidget(totalGainCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search stocks by symbol or company name (e.g. RELIANCE, TCS)..."),
        {QStringLiteral("All Sectors"), QStringLiteral("Technology"), QStringLiteral("Financials"), QStringLiteral("Energy"), QStringLiteral("Healthcare"), QStringLiteral("Consumer")},
        {QStringLiteral("All Caps"), QStringLiteral("Large Cap"), QStringLiteral("Mid Cap"), QStringLiteral("Small Cap")},
        QStringLiteral("+ Add Stock"),
        container
    );
    connect(m_filterBar, &FilterBar::actionClicked, this, &StocksPage::addStockRequested);
    layout->addWidget(m_filterBar);

    // Stocks Holdings Table
    m_stocksTable = new ModernTableWidget(container);
    m_stocksTable->setHeaders({
        QStringLiteral("Symbol"),
        QStringLiteral("Company Name"),
        QStringLiteral("Sector"),
        QStringLiteral("Shares"),
        QStringLiteral("Avg Price"),
        QStringLiteral("CMP"),
        QStringLiteral("Current Value"),
        QStringLiteral("Day Change %"),
        QStringLiteral("Overall P&L %")
    });
    m_stocksTable->setEmptyMessage(QStringLiteral("No stock investments recorded yet. Add your first stock to begin tracking equities."));
    m_stocksTable->setMinimumHeight(350);
    layout->addWidget(m_stocksTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

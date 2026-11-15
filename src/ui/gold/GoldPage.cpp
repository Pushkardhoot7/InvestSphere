#include "GoldPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

GoldPage::GoldPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void GoldPage::setupUi()
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

    auto *goldValCard = new SummaryCard(QStringLiteral("Total Gold Valuation"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Gold, container);
    goldValCard->setSubtitle(QStringLiteral("Physical, SGB, & Digital"));
    summaryRow->addWidget(goldValCard);

    auto *gramsCard = new SummaryCard(QStringLiteral("Total Gold Weight"), QStringLiteral("0.000 g"), QStringLiteral("24K / 22K"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Gold, container);
    gramsCard->setSubtitle(QStringLiteral("Cumulative pure weight held"));
    summaryRow->addWidget(gramsCard);

    auto *goldReturnCard = new SummaryCard(QStringLiteral("Precious Metals P&L"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::TrendUp, container);
    goldReturnCard->setSubtitle(QStringLiteral("Appreciation against purchase"));
    summaryRow->addWidget(goldReturnCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search gold holdings or sovereign series..."),
        {QStringLiteral("All Forms"), QStringLiteral("Sovereign Gold Bond (SGB)"), QStringLiteral("Physical Coin / Bar"), QStringLiteral("Digital Gold")},
        {QStringLiteral("All Purities"), QStringLiteral("24 Karat (99.9%)"), QStringLiteral("22 Karat (91.6%)")},
        QStringLiteral("+ Add Gold Holding"),
        container
    );
    connect(m_filterBar, &FilterBar::actionClicked, this, &GoldPage::addGoldRequested);
    layout->addWidget(m_filterBar);

    // Gold Holdings Table
    m_goldTable = new ModernTableWidget(container);
    m_goldTable->setHeaders({
        QStringLiteral("Holding Name / Series"),
        QStringLiteral("Form"),
        QStringLiteral("Purity"),
        QStringLiteral("Weight (g)"),
        QStringLiteral("Storage / Depository"),
        QStringLiteral("Avg Cost / g"),
        QStringLiteral("Current Price / g"),
        QStringLiteral("Current Value"),
        QStringLiteral("Returns %")
    });
    m_goldTable->setEmptyMessage(QStringLiteral("No gold holdings recorded yet. Add physical gold, SGBs, or digital gold."));
    m_goldTable->setMinimumHeight(350);
    layout->addWidget(m_goldTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

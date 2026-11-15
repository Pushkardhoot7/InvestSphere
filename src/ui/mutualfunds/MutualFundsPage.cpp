#include "MutualFundsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

MutualFundsPage::MutualFundsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void MutualFundsPage::setupUi()
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

    auto *totalMfCard = new SummaryCard(QStringLiteral("Mutual Fund Corpus"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::MutualFunds, container);
    totalMfCard->setSubtitle(QStringLiteral("Total valuation across funds"));
    summaryRow->addWidget(totalMfCard);

    auto *activeSipCard = new SummaryCard(QStringLiteral("Active SIP Plans"), QStringLiteral("0 Plans"), QStringLiteral("Monthly"), SummaryCard::TrendType::Neutral, IconHelper::IconType::SipCalculator, container);
    activeSipCard->setSubtitle(QStringLiteral("₹0.00 committed per month"));
    summaryRow->addWidget(activeSipCard);

    auto *mfReturnCard = new SummaryCard(QStringLiteral("Mutual Fund Returns"), QStringLiteral("₹0.00"), QStringLiteral("0.0%"), SummaryCard::TrendType::Neutral, IconHelper::IconType::TrendUp, container);
    mfReturnCard->setSubtitle(QStringLiteral("Consolidated XIRR returns"));
    summaryRow->addWidget(mfReturnCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search mutual fund schemes (e.g. Parag Parikh Flexi Cap)..."),
        {QStringLiteral("All Categories"), QStringLiteral("Equity - Flexi Cap"), QStringLiteral("Equity - Large Cap"), QStringLiteral("Debt - Liquid"), QStringLiteral("ELSS Tax Saver"), QStringLiteral("Hybrid")},
        {QStringLiteral("All Plans"), QStringLiteral("Direct - Growth"), QStringLiteral("Regular - Growth"), QStringLiteral("IDCW")},
        QStringLiteral("+ Add Mutual Fund"),
        container
    );
    connect(m_filterBar, &FilterBar::actionClicked, this, &MutualFundsPage::addMutualFundRequested);
    layout->addWidget(m_filterBar);

    // Mutual Funds Table
    m_fundsTable = new ModernTableWidget(container);
    m_fundsTable->setHeaders({
        QStringLiteral("Scheme Name"),
        QStringLiteral("Category"),
        QStringLiteral("Option"),
        QStringLiteral("Units Held"),
        QStringLiteral("Avg NAV"),
        QStringLiteral("Current NAV"),
        QStringLiteral("Invested Amount"),
        QStringLiteral("Current Value"),
        QStringLiteral("Gain / Loss %")
    });
    m_fundsTable->setEmptyMessage(QStringLiteral("No mutual funds recorded yet. Add your first fund scheme or SIP to begin tracking."));
    m_fundsTable->setMinimumHeight(350);
    layout->addWidget(m_fundsTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

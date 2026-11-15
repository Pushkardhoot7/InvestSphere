#include "FixedDepositsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

FixedDepositsPage::FixedDepositsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void FixedDepositsPage::setupUi()
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

    auto *fdPrincipalCard = new SummaryCard(QStringLiteral("Total FD Principal"), QStringLiteral("₹0.00"), QStringLiteral("Guaranteed"), SummaryCard::TrendType::Neutral, IconHelper::IconType::FixedDeposits, container);
    fdPrincipalCard->setSubtitle(QStringLiteral("Active bank & NBFC deposits"));
    summaryRow->addWidget(fdPrincipalCard);

    auto *fdMaturityCard = new SummaryCard(QStringLiteral("Estimated Maturity Value"), QStringLiteral("₹0.00"), QStringLiteral("Compounded"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Calendar, container);
    fdMaturityCard->setSubtitle(QStringLiteral("Projected maturity cash payout"));
    summaryRow->addWidget(fdMaturityCard);

    auto *fdRateCard = new SummaryCard(QStringLiteral("Weighted Average ROI"), QStringLiteral("0.00%"), QStringLiteral("p.a."), SummaryCard::TrendType::Neutral, IconHelper::IconType::Analytics, container);
    fdRateCard->setSubtitle(QStringLiteral("Portfolio average interest yield"));
    summaryRow->addWidget(fdRateCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search fixed deposits by bank or account number..."),
        {QStringLiteral("All Institutions"), QStringLiteral("HDFC Bank"), QStringLiteral("State Bank of India"), QStringLiteral("ICICI Bank"), QStringLiteral("Axis Bank"), QStringLiteral("Bajaj Finance")},
        {QStringLiteral("All Statuses"), QStringLiteral("Active"), QStringLiteral("Matured")},
        QStringLiteral("+ Add Fixed Deposit"),
        container
    );
    connect(m_filterBar, &FilterBar::actionClicked, this, &FixedDepositsPage::addFixedDepositRequested);
    layout->addWidget(m_filterBar);

    // Fixed Deposits Table
    m_fdTable = new ModernTableWidget(container);
    m_fdTable->setHeaders({
        QStringLiteral("Institution / Bank"),
        QStringLiteral("Account Number"),
        QStringLiteral("Principal Amount"),
        QStringLiteral("Interest Rate"),
        QStringLiteral("Compounding"),
        QStringLiteral("Start Date"),
        QStringLiteral("Maturity Date"),
        QStringLiteral("Maturity Value"),
        QStringLiteral("Status")
    });
    m_fdTable->setEmptyMessage(QStringLiteral("No fixed deposits recorded yet. Add bank or corporate FDs to track maturity timelines."));
    m_fdTable->setMinimumHeight(350);
    layout->addWidget(m_fdTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

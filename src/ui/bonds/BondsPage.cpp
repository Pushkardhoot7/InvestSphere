#include "BondsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

BondsPage::BondsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void BondsPage::setupUi()
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

    auto *bondValCard = new SummaryCard(QStringLiteral("Total Bond Valuation"), QStringLiteral("₹0.00"), QStringLiteral("Fixed Income"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Bonds, container);
    bondValCard->setSubtitle(QStringLiteral("Government & Corporate issues"));
    summaryRow->addWidget(bondValCard);

    auto *couponCard = new SummaryCard(QStringLiteral("Annual Coupon Income"), QStringLiteral("₹0.00"), QStringLiteral("Periodic"), SummaryCard::TrendType::Neutral, IconHelper::IconType::Calendar, container);
    couponCard->setSubtitle(QStringLiteral("Total regular coupon cash inflow"));
    summaryRow->addWidget(couponCard);

    auto *ytmCard = new SummaryCard(QStringLiteral("Average Coupon Rate"), QStringLiteral("0.00%"), QStringLiteral("p.a."), SummaryCard::TrendType::Neutral, IconHelper::IconType::TrendUp, container);
    ytmCard->setSubtitle(QStringLiteral("Weighted portfolio yield"));
    summaryRow->addWidget(ytmCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search bonds by issuer or ISIN..."),
        {QStringLiteral("All Categories"), QStringLiteral("Government Security (G-Sec)"), QStringLiteral("State Development Loan (SDL)"), QStringLiteral("PSU Bond"), QStringLiteral("Corporate Bond")},
        {QStringLiteral("All Ratings"), QStringLiteral("Sovereign"), QStringLiteral("AAA"), QStringLiteral("AA+"), QStringLiteral("AA")},
        QStringLiteral("+ Add Bond"),
        container
    );
    connect(m_filterBar, &FilterBar::actionClicked, this, &BondsPage::addBondRequested);
    layout->addWidget(m_filterBar);

    // Bonds Table
    m_bondsTable = new ModernTableWidget(container);
    m_bondsTable->setHeaders({
        QStringLiteral("Issuer / Security"),
        QStringLiteral("Bond Type"),
        QStringLiteral("Rating"),
        QStringLiteral("Face Value"),
        QStringLiteral("Coupon Rate"),
        QStringLiteral("Frequency"),
        QStringLiteral("Issue Date"),
        QStringLiteral("Maturity Date"),
        QStringLiteral("Status")
    });
    m_bondsTable->setEmptyMessage(QStringLiteral("No bond holdings recorded yet. Add sovereign or corporate bonds to track coupon inflows."));
    m_bondsTable->setMinimumHeight(350);
    layout->addWidget(m_bondsTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

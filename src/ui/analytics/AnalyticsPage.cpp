#include "AnalyticsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QFrame>
#include <QDate>
#include <QLabel>
#include <QMessageBox>

#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"

namespace InvestSphere {
namespace Ui {

AnalyticsPage::AnalyticsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadAnalytics();
}

QString AnalyticsPage::formatInrCents(int64_t cents) const
{
    double val = static_cast<double>(cents) / 100.0;
    return QStringLiteral("₹%L1").arg(val, 0, 'f', 2);
}

void AnalyticsPage::setupUi()
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

    setupHeader(layout);
    setupOverviewCards(layout);
    setupCategorySection(layout);
    setupPerformersSection(layout);
    setupHistorySection(layout);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void AnalyticsPage::setupHeader(QVBoxLayout *layout)
{
    auto *headerCard = new QFrame(this);
    headerCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 16px 20px;"
        "}"
    ));
    auto *hLayout = new QHBoxLayout(headerCard);
    hLayout->setSpacing(14);

    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(4);

    auto *title = new QLabel(QStringLiteral("Portfolio Performance & Asset Analytics"), headerCard);
    title->setStyleSheet(QStringLiteral("font-family: %1; font-size: 18px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));

    auto *subtitle = new QLabel(QStringLiteral("Live valuation metrics, category distribution, risk exposure, and historical snapshots."), headerCard);
    subtitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 13px; color: #64748B;").arg(Theme::fontFamily()));

    titleCol->addWidget(title);
    titleCol->addWidget(subtitle);
    hLayout->addLayout(titleCol, 1);

    // Filter controls
    auto *filterRow = new QHBoxLayout();
    filterRow->setSpacing(10);

    auto *catLabel = new QLabel(QStringLiteral("Asset Class:"), headerCard);
    catLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 600; color: #475569;"));
    filterRow->addWidget(catLabel);

    m_categoryFilterCombo = new QComboBox(headerCard);
    m_categoryFilterCombo->addItem(QStringLiteral("All Categories"), QStringLiteral(""));
    m_categoryFilterCombo->addItem(QStringLiteral("Stocks (EQUITY)"), QStringLiteral("EQUITY"));
    m_categoryFilterCombo->addItem(QStringLiteral("Mutual Funds (MUTUAL_FUND)"), QStringLiteral("MUTUAL_FUND"));
    m_categoryFilterCombo->addItem(QStringLiteral("Gold (COMMODITY)"), QStringLiteral("COMMODITY"));
    m_categoryFilterCombo->addItem(QStringLiteral("Fixed Deposits (FIXED_DEPOSIT)"), QStringLiteral("FIXED_DEPOSIT"));
    m_categoryFilterCombo->addItem(QStringLiteral("Bonds (BOND)"), QStringLiteral("BOND"));
    m_categoryFilterCombo->setStyleSheet(Theme::inputFieldStyle());
    m_categoryFilterCombo->setMinimumWidth(160);
    connect(m_categoryFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsPage::handleFilterChanged);
    filterRow->addWidget(m_categoryFilterCombo);

    auto *timeLabel = new QLabel(QStringLiteral("Time Horizon:"), headerCard);
    timeLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 600; color: #475569;"));
    filterRow->addWidget(timeLabel);

    m_timeHorizonCombo = new QComboBox(headerCard);
    m_timeHorizonCombo->addItem(QStringLiteral("All Time"), 0);
    m_timeHorizonCombo->addItem(QStringLiteral("Last 1 Month"), 30);
    m_timeHorizonCombo->addItem(QStringLiteral("Last 3 Months"), 90);
    m_timeHorizonCombo->addItem(QStringLiteral("Last 6 Months"), 180);
    m_timeHorizonCombo->addItem(QStringLiteral("Last 1 Year"), 365);
    m_timeHorizonCombo->setStyleSheet(Theme::inputFieldStyle());
    connect(m_timeHorizonCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsPage::handleFilterChanged);
    filterRow->addWidget(m_timeHorizonCombo);

    m_refreshBtn = new QPushButton(QStringLiteral("Refresh"), headerCard);
    m_refreshBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #334155;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E2E8F0;"
        "}"
    ));
    connect(m_refreshBtn, &QPushButton::clicked, this, &AnalyticsPage::loadAnalytics);
    filterRow->addWidget(m_refreshBtn);

    m_snapshotBtn = new QPushButton(QStringLiteral("Take Snapshot"), headerCard);
    m_snapshotBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "   background-color: #2563EB;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1D4ED8;"
        "}"
    ));
    connect(m_snapshotBtn, &QPushButton::clicked, this, &AnalyticsPage::handleTakeSnapshot);
    filterRow->addWidget(m_snapshotBtn);

    hLayout->addLayout(filterRow);
    layout->addWidget(headerCard);
}

void AnalyticsPage::setupOverviewCards(QVBoxLayout *layout)
{
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    m_costBasisCard = new SummaryCard(
        QStringLiteral("Total Cost Basis"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Capital deployed"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        this
    );
    grid->addWidget(m_costBasisCard, 0, 0);

    m_currentValueCard = new SummaryCard(
        QStringLiteral("Estimated Portfolio Value"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Latest valuations"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::Analytics,
        this
    );
    grid->addWidget(m_currentValueCard, 0, 1);

    m_unrealizedPnlCard = new SummaryCard(
        QStringLiteral("Unrealized Profit / Loss"),
        QStringLiteral("₹0.00"),
        QStringLiteral("0.0% return"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::TrendUp,
        this
    );
    grid->addWidget(m_unrealizedPnlCard, 0, 2);

    m_realizedPnlCard = new SummaryCard(
        QStringLiteral("Realized Profit / Loss"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Completed sales"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Transactions,
        this
    );
    grid->addWidget(m_realizedPnlCard, 1, 0);

    m_incomeCard = new SummaryCard(
        QStringLiteral("Dividends & Interest"),
        QStringLiteral("₹0.00"),
        QStringLiteral("Passive income"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::TrendUp,
        this
    );
    grid->addWidget(m_incomeCard, 1, 1);

    m_holdingsCountCard = new SummaryCard(
        QStringLiteral("Active Holdings"),
        QStringLiteral("0"),
        QStringLiteral("0 with live quote"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Filter,
        this
    );
    grid->addWidget(m_holdingsCountCard, 1, 2);

    layout->addLayout(grid);
}

void AnalyticsPage::setupCategorySection(QVBoxLayout *layout)
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 20px 24px;"
        "}"
    ));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(14);

    auto *headerRow = new QHBoxLayout();
    auto *secTitle = new QLabel(QStringLiteral("Asset Allocation & Category Breakdown"), card);
    secTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    headerRow->addWidget(secTitle);
    headerRow->addStretch(1);

    m_coverageLabel = new QLabel(QStringLiteral("Valuation Coverage: 100%"), card);
    m_coverageLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 600; color: #475569;"));
    headerRow->addWidget(m_coverageLabel);
    cardLayout->addLayout(headerRow);

    m_categoryTable = new QTableWidget(card);
    m_categoryTable->setColumnCount(8);
    m_categoryTable->setHorizontalHeaderLabels({
        QStringLiteral("Asset Class"),
        QStringLiteral("Holdings"),
        QStringLiteral("Cost Basis (₹)"),
        QStringLiteral("Estimated Value (₹)"),
        QStringLiteral("Unrealized P&L (₹)"),
        QStringLiteral("Return %"),
        QStringLiteral("Realized P&L (₹)"),
        QStringLiteral("Valued %")
    });
    m_categoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_categoryTable->setAlternatingRowColors(true);
    m_categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_categoryTable->verticalHeader()->setVisible(false);
    m_categoryTable->setMinimumHeight(180);
    m_categoryTable->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   gridline-color: #F1F5F9;"
        "   background-color: #FFFFFF;"
        "   alternate-background-color: #F8FAFC;"
        "   font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 10px;"
        "   border: none;"
        "   border-bottom: 2px solid #E2E8F0;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px 10px;"
        "   color: #1E293B;"
        "}"
    ));
    cardLayout->addWidget(m_categoryTable);

    layout->addWidget(card);
}

void AnalyticsPage::setupPerformersSection(QVBoxLayout *layout)
{
    auto *row = new QHBoxLayout();
    row->setSpacing(20);

    // Left: Top Performers
    auto *topCard = new QFrame(this);
    topCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 18px 20px;"
        "}"
    ));
    auto *topLayout = new QVBoxLayout(topCard);
    topLayout->setSpacing(12);

    auto *topTitle = new QLabel(QStringLiteral("🚀 Top Performing Holdings"), topCard);
    topTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 15px; font-weight: 700; color: #16A34A;").arg(Theme::fontFamily()));
    topLayout->addWidget(topTitle);

    m_topPerformersTable = new QTableWidget(topCard);
    m_topPerformersTable->setColumnCount(4);
    m_topPerformersTable->setHorizontalHeaderLabels({
        QStringLiteral("Symbol"),
        QStringLiteral("Cost Basis (₹)"),
        QStringLiteral("Value (₹)"),
        QStringLiteral("Return %")
    });
    m_topPerformersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_topPerformersTable->verticalHeader()->setVisible(false);
    m_topPerformersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_topPerformersTable->setMinimumHeight(150);
    m_topPerformersTable->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   background-color: #FFFFFF;"
        "   font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   padding: 8px;"
        "   border-bottom: 1px solid #E2E8F0;"
        "}"
    ));
    topLayout->addWidget(m_topPerformersTable);
    row->addWidget(topCard, 1);

    // Right: Worst Performers
    auto *worstCard = new QFrame(this);
    worstCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 18px 20px;"
        "}"
    ));
    auto *worstLayout = new QVBoxLayout(worstCard);
    worstLayout->setSpacing(12);

    auto *worstTitle = new QLabel(QStringLiteral("🔻 Lagging Holdings"), worstCard);
    worstTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 15px; font-weight: 700; color: #DC2626;").arg(Theme::fontFamily()));
    worstLayout->addWidget(worstTitle);

    m_worstPerformersTable = new QTableWidget(worstCard);
    m_worstPerformersTable->setColumnCount(4);
    m_worstPerformersTable->setHorizontalHeaderLabels({
        QStringLiteral("Symbol"),
        QStringLiteral("Cost Basis (₹)"),
        QStringLiteral("Value (₹)"),
        QStringLiteral("Return %")
    });
    m_worstPerformersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_worstPerformersTable->verticalHeader()->setVisible(false);
    m_worstPerformersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_worstPerformersTable->setMinimumHeight(150);
    m_worstPerformersTable->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   background-color: #FFFFFF;"
        "   font-size: 12px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   padding: 8px;"
        "   border-bottom: 1px solid #E2E8F0;"
        "}"
    ));
    worstLayout->addWidget(m_worstPerformersTable);
    row->addWidget(worstCard, 1);

    layout->addLayout(row);
}

void AnalyticsPage::setupHistorySection(QVBoxLayout *layout)
{
    auto *card = new QFrame(this);
    card->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 20px 24px;"
        "}"
    ));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("Historical Valuation Snapshots"), card);
    title->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    cardLayout->addWidget(title);

    // Empty state container
    m_historyEmptyCard = new QFrame(card);
    m_historyEmptyCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #F8FAFC;"
        "   border: 1px dashed #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 24px;"
        "}"
    ));
    auto *emptyLayout = new QVBoxLayout(m_historyEmptyCard);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setSpacing(8);

    auto *emptyIcon = new QLabel(QStringLiteral("📈"), m_historyEmptyCard);
    emptyIcon->setStyleSheet(QStringLiteral("font-size: 28px;"));
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyIcon);

    m_historyEmptyMsg = new QLabel(
        QStringLiteral("No historical snapshots recorded yet. Click 'Take Snapshot' above or record new transactions to start building historical tracking."),
        m_historyEmptyCard
    );
    m_historyEmptyMsg->setWordWrap(true);
    m_historyEmptyMsg->setAlignment(Qt::AlignCenter);
    m_historyEmptyMsg->setStyleSheet(QStringLiteral("color: #64748B; font-size: 13px;"));
    emptyLayout->addWidget(m_historyEmptyMsg);

    cardLayout->addWidget(m_historyEmptyCard);

    // Table for snapshots
    m_historyTable = new QTableWidget(card);
    m_historyTable->setColumnCount(6);
    m_historyTable->setHorizontalHeaderLabels({
        QStringLiteral("Snapshot Date"),
        QStringLiteral("Total Invested (₹)"),
        QStringLiteral("Estimated Value (₹)"),
        QStringLiteral("Unrealized P&L (₹)"),
        QStringLiteral("Realized P&L (₹)"),
        QStringLiteral("Notes")
    });
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setMinimumHeight(160);
    m_historyTable->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   background-color: #FFFFFF;"
        "   font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 10px;"
        "   border-bottom: 2px solid #E2E8F0;"
        "}"
    ));
    m_historyTable->setVisible(false);
    cardLayout->addWidget(m_historyTable);

    layout->addWidget(card);
}

void AnalyticsPage::handleFilterChanged()
{
    loadAnalytics();
}

void AnalyticsPage::handleTakeSnapshot()
{
    auto userOpt = App::SessionManager::instance().getCurrentUser();
    if (!userOpt) {
        QMessageBox::warning(this, QStringLiteral("Authentication Required"),
                             QStringLiteral("Please log in to record a portfolio snapshot."));
        return;
    }

    bool recorded = Services::AnalyticsService::instance().recordSnapshotIfValuationAvailable(userOpt->userId);
    if (recorded) {
        QMessageBox::information(this, QStringLiteral("Snapshot Recorded"),
                                 QStringLiteral("A portfolio snapshot has been saved for today."));
        loadAnalytics();
    } else {
        QMessageBox::warning(this, QStringLiteral("Cannot Record Snapshot"),
                             QStringLiteral("Snapshots require at least one holding with a valid market valuation."));
    }
}

void AnalyticsPage::loadAnalytics()
{
    auto userOpt = App::SessionManager::instance().getCurrentUser();
    if (!userOpt) {
        return;
    }

    int userId = userOpt->userId;
    std::string catFilter = m_categoryFilterCombo
        ? m_categoryFilterCombo->currentData().toString().toStdString()
        : "";

    std::string startDate;
    int days = m_timeHorizonCombo ? m_timeHorizonCombo->currentData().toInt() : 0;
    if (days > 0) {
        startDate = QDate::currentDate().addDays(-days).toString(QStringLiteral("yyyy-MM-dd")).toStdString();
    }

    auto dto = Services::AnalyticsService::instance().computeAnalytics(userId, catFilter, startDate, "");

    populateOverview(dto);
    populateCategories(dto);
    populatePerformers(dto);
    populateHistory(dto);
}

void AnalyticsPage::populateOverview(const Services::PortfolioAnalyticsDTO &dto)
{
    const auto &s = dto.summary;

    m_costBasisCard->setValue(formatInrCents(s.totalInvestedCostBasisCents));

    if (s.quotedHoldingsCount > 0) {
        m_currentValueCard->setValue(formatInrCents(s.totalCurrentEstimatedValueCents));
    } else {
        m_currentValueCard->setValue(QStringLiteral("Unavailable"));
        m_currentValueCard->setSubtitle(QStringLiteral("No market quote data"));
    }

    m_unrealizedPnlCard->setValue(formatInrCents(s.totalUnrealizedPnlCents));
    if (s.totalUnrealizedPnlCents >= 0) {
        m_unrealizedPnlCard->setTrend(QStringLiteral("+%1%").arg(s.unrealizedPnlPercent, 0, 'f', 2), SummaryCard::TrendType::Positive);
    } else {
        m_unrealizedPnlCard->setTrend(QStringLiteral("%1%").arg(s.unrealizedPnlPercent, 0, 'f', 2), SummaryCard::TrendType::Negative);
    }

    m_realizedPnlCard->setValue(formatInrCents(s.totalRealizedPnlCents));
    if (s.totalRealizedPnlCents >= 0) {
        m_realizedPnlCard->setTrend(QStringLiteral("Net Gain"), SummaryCard::TrendType::Positive);
    } else {
        m_realizedPnlCard->setTrend(QStringLiteral("Net Loss"), SummaryCard::TrendType::Negative);
    }

    m_incomeCard->setValue(formatInrCents(s.totalIncomeCents));

    m_holdingsCountCard->setValue(QString::number(s.activeHoldingsCount));
    m_holdingsCountCard->setSubtitle(
        QStringLiteral("%1 of %2 holdings valued").arg(s.quotedHoldingsCount).arg(s.activeHoldingsCount)
    );
}

void AnalyticsPage::populateCategories(const Services::PortfolioAnalyticsDTO &dto)
{
    if (!m_categoryTable) return;

    m_categoryTable->setRowCount(static_cast<int>(dto.categories.size()));
    for (int i = 0; i < static_cast<int>(dto.categories.size()); ++i) {
        const auto &cat = dto.categories[i];

        auto *itemCat  = new QTableWidgetItem(QString::fromStdString(cat.categoryName));
        auto *itemCnt  = new QTableWidgetItem(QString::number(cat.holdingsCount));
        auto *itemCost = new QTableWidgetItem(formatInrCents(cat.costBasisCents));
        auto *itemVal  = new QTableWidgetItem(cat.hasValuation ? formatInrCents(cat.estimatedValueCents) : QStringLiteral("—"));
        auto *itemPnl  = new QTableWidgetItem(cat.hasValuation ? formatInrCents(cat.unrealizedPnlCents) : QStringLiteral("—"));
        auto *itemPct  = new QTableWidgetItem(cat.hasValuation ? QStringLiteral("%1%").arg(cat.unrealizedPnlPercent, 0, 'f', 1) : QStringLiteral("—"));
        auto *itemReal = new QTableWidgetItem(formatInrCents(cat.realizedPnlCents));
        auto *itemAlloc = new QTableWidgetItem(cat.hasValuation ? QStringLiteral("%1%").arg(cat.allocationPercent, 0, 'f', 1) : QStringLiteral("0.0%"));

        itemCat->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemCnt->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        itemCost->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemVal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemPnl->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemPct->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemReal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemAlloc->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (cat.hasValuation) {
            if (cat.unrealizedPnlCents >= 0) {
                itemPnl->setForeground(QColor(QStringLiteral("#16A34A")));
                itemPct->setForeground(QColor(QStringLiteral("#16A34A")));
            } else {
                itemPnl->setForeground(QColor(QStringLiteral("#DC2626")));
                itemPct->setForeground(QColor(QStringLiteral("#DC2626")));
            }
        }

        m_categoryTable->setItem(i, 0, itemCat);
        m_categoryTable->setItem(i, 1, itemCnt);
        m_categoryTable->setItem(i, 2, itemCost);
        m_categoryTable->setItem(i, 3, itemVal);
        m_categoryTable->setItem(i, 4, itemPnl);
        m_categoryTable->setItem(i, 5, itemPct);
        m_categoryTable->setItem(i, 6, itemReal);
        m_categoryTable->setItem(i, 7, itemAlloc);
    }

    if (m_coverageLabel) {
        double cov = dto.summary.valuationCoveragePercent;
        m_coverageLabel->setText(QStringLiteral("Valuation Coverage: %1% (%2/%3 holdings)")
            .arg(cov, 0, 'f', 1)
            .arg(dto.summary.quotedHoldingsCount)
            .arg(dto.summary.activeHoldingsCount));
    }
}

void AnalyticsPage::populatePerformers(const Services::PortfolioAnalyticsDTO &dto)
{
    // Populate Top Performers
    if (m_topPerformersTable) {
        m_topPerformersTable->setRowCount(static_cast<int>(dto.topPerformers.size()));
        for (int i = 0; i < static_cast<int>(dto.topPerformers.size()); ++i) {
            const auto &p = dto.topPerformers[i];
            auto *sym  = new QTableWidgetItem(QString::fromStdString(p.symbol));
            auto *cost = new QTableWidgetItem(formatInrCents(p.costBasisCents));
            auto *val  = new QTableWidgetItem(formatInrCents(p.marketValueCents));
            auto *ret  = new QTableWidgetItem(QStringLiteral("+%1%").arg(p.unrealizedPnlPercent, 0, 'f', 1));

            ret->setForeground(QColor(QStringLiteral("#16A34A")));
            cost->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            val->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ret->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            m_topPerformersTable->setItem(i, 0, sym);
            m_topPerformersTable->setItem(i, 1, cost);
            m_topPerformersTable->setItem(i, 2, val);
            m_topPerformersTable->setItem(i, 3, ret);
        }
    }

    // Populate Worst Performers
    if (m_worstPerformersTable) {
        m_worstPerformersTable->setRowCount(static_cast<int>(dto.worstPerformers.size()));
        for (int i = 0; i < static_cast<int>(dto.worstPerformers.size()); ++i) {
            const auto &p = dto.worstPerformers[i];
            auto *sym  = new QTableWidgetItem(QString::fromStdString(p.symbol));
            auto *cost = new QTableWidgetItem(formatInrCents(p.costBasisCents));
            auto *val  = new QTableWidgetItem(formatInrCents(p.marketValueCents));
            auto *ret  = new QTableWidgetItem(QStringLiteral("%1%").arg(p.unrealizedPnlPercent, 0, 'f', 1));

            if (p.unrealizedPnlPercent < 0.0) {
                ret->setForeground(QColor(QStringLiteral("#DC2626")));
            }
            cost->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            val->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ret->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            m_worstPerformersTable->setItem(i, 0, sym);
            m_worstPerformersTable->setItem(i, 1, cost);
            m_worstPerformersTable->setItem(i, 2, val);
            m_worstPerformersTable->setItem(i, 3, ret);
        }
    }
}

void AnalyticsPage::populateHistory(const Services::PortfolioAnalyticsDTO &dto)
{
    if (!m_historyTable || !m_historyEmptyCard) return;

    if (!dto.hasPerformanceHistory) {
        m_historyEmptyCard->setVisible(true);
        m_historyTable->setVisible(false);
        if (m_historyEmptyMsg) {
            m_historyEmptyMsg->setText(QString::fromStdString(dto.performanceHistoryNote));
        }
        return;
    }

    m_historyEmptyCard->setVisible(false);
    m_historyTable->setVisible(true);

    m_historyTable->setRowCount(static_cast<int>(dto.performanceHistory.size()));
    for (int i = 0; i < static_cast<int>(dto.performanceHistory.size()); ++i) {
        const auto &snap = dto.performanceHistory[i];

        auto *itemDate  = new QTableWidgetItem(QString::fromStdString(snap.snapshotTimestamp));
        auto *itemInv   = new QTableWidgetItem(formatInrCents(snap.totalInvestedCents));
        auto *itemVal   = new QTableWidgetItem(formatInrCents(snap.estimatedValueCents));
        auto *itemUnreal = new QTableWidgetItem(formatInrCents(snap.unrealizedPnlCents));
        auto *itemReal  = new QTableWidgetItem(formatInrCents(snap.realizedPnlCents));
        auto *itemNotes = new QTableWidgetItem(QString::fromStdString(snap.notes));

        itemDate->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemInv->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemVal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemUnreal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemReal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (snap.unrealizedPnlCents >= 0) {
            itemUnreal->setForeground(QColor(QStringLiteral("#16A34A")));
        } else {
            itemUnreal->setForeground(QColor(QStringLiteral("#DC2626")));
        }

        m_historyTable->setItem(i, 0, itemDate);
        m_historyTable->setItem(i, 1, itemInv);
        m_historyTable->setItem(i, 2, itemVal);
        m_historyTable->setItem(i, 3, itemUnreal);
        m_historyTable->setItem(i, 4, itemReal);
        m_historyTable->setItem(i, 5, itemNotes);
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

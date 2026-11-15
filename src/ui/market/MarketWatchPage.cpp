#include "MarketWatchPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QString>
#include <cstdint>

#include "Theme.h"
#include "IconHelper.h"
#include "SessionManager.h"
#include "MarketDataService.h"
#include "MarketDataProvider.h"

namespace InvestSphere {
namespace Ui {

MarketWatchPage::MarketWatchPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    // Bootstrap the service with the mock provider on first use
    auto &svc = Services::MarketDataService::instance();
    if (!svc.getProvider().isConfigured()) {
        svc.setProvider(std::make_shared<Services::MockMarketDataProvider>());
    }

    loadMarketData();

    // Auto-refresh every 5 minutes
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(300'000);
    connect(m_refreshTimer, &QTimer::timeout, this, &MarketWatchPage::loadMarketData);
    m_refreshTimer->start();
}

void MarketWatchPage::setupUi()
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

    // Major Indices Summary Cards
    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);

    m_niftyCard = new SummaryCard(QStringLiteral("NIFTY 50"), QStringLiteral("–"), QStringLiteral("Loading…"),
                                  SummaryCard::TrendType::Neutral, IconHelper::IconType::MarketWatch, container);
    m_niftyCard->setSubtitle(QStringLiteral("NSE Benchmark Index"));
    summaryRow->addWidget(m_niftyCard);

    m_sensexCard = new SummaryCard(QStringLiteral("BSE SENSEX"), QStringLiteral("–"), QStringLiteral("Loading…"),
                                   SummaryCard::TrendType::Neutral, IconHelper::IconType::MarketWatch, container);
    m_sensexCard->setSubtitle(QStringLiteral("BSE 30 Core Companies"));
    summaryRow->addWidget(m_sensexCard);

    m_goldCard = new SummaryCard(QStringLiteral("MCX GOLD (10g)"), QStringLiteral("–"), QStringLiteral("Loading…"),
                                  SummaryCard::TrendType::Neutral, IconHelper::IconType::Gold, container);
    m_goldCard->setSubtitle(QStringLiteral("24K 999 Purity Bullion"));
    summaryRow->addWidget(m_goldCard);

    layout->addLayout(summaryRow);

    // Filter Bar
    m_filterBar = new FilterBar(
        QStringLiteral("Search watchlist or add new market ticker..."),
        {QStringLiteral("All Markets"), QStringLiteral("NSE Equities"), QStringLiteral("BSE Equities"),
         QStringLiteral("MCX Commodities"), QStringLiteral("Mutual Funds")},
        {QStringLiteral("All Movers"), QStringLiteral("Gainers"), QStringLiteral("Losers"), QStringLiteral("52W Highs")},
        QStringLiteral("⟳ Refresh Quotes"),
        container
    );
    connect(m_filterBar, &FilterBar::searchChanged, this, &MarketWatchPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::filterChanged, this, &MarketWatchPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::actionClicked, this, &MarketWatchPage::onRefreshClicked);
    layout->addWidget(m_filterBar);

    // Watchlist Table
    m_watchlistTable = new ModernTableWidget(container);
    m_watchlistTable->setHeaders({
        QStringLiteral("Symbol"),
        QStringLiteral("Name"),
        QStringLiteral("Exchange"),
        QStringLiteral("LTP (₹)"),
        QStringLiteral("Change (₹)"),
        QStringLiteral("Change %"),
        QStringLiteral("Day High"),
        QStringLiteral("Day Low"),
        QStringLiteral("Data Source")
    });
    m_watchlistTable->setEmptyMessage(QStringLiteral("Your watchlist is empty. Add securities from the Portfolio page to monitor them here."));
    m_watchlistTable->setMinimumHeight(380);
    layout->addWidget(m_watchlistTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

QString MarketWatchPage::formatPriceCents(int64_t cents)
{
    if (cents == 0) return QStringLiteral("–");
    double val = static_cast<double>(cents) / 100.0;
    return QString::fromStdString("₹") + QString::number(val, 'f', 2);
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void MarketWatchPage::loadMarketData()
{
    auto &svc = Services::MarketDataService::instance();

    // Populate index cards with mock quotes
    svc.getProvider().fetchStockQuote("^NSEI", [this](const Services::StockQuoteDTO &q) {
        if (q.isValid && m_niftyCard) {
            m_niftyCard->setValue(formatPriceCents(q.latestPriceCents));
            double pct = q.changePercent;
            QString deltaText = (pct >= 0 ? QStringLiteral("+") : QString()) +
                                QString::number(pct, 'f', 2) + QStringLiteral("%");
            m_niftyCard->setTrend(deltaText, pct >= 0 ? SummaryCard::TrendType::Positive : SummaryCard::TrendType::Negative);
        }
    });

    svc.getProvider().fetchStockQuote("^BSESN", [this](const Services::StockQuoteDTO &q) {
        if (q.isValid && m_sensexCard) {
            m_sensexCard->setValue(formatPriceCents(q.latestPriceCents));
            double pct = q.changePercent;
            QString deltaText = (pct >= 0 ? QStringLiteral("+") : QString()) +
                                QString::number(pct, 'f', 2) + QStringLiteral("%");
            m_sensexCard->setTrend(deltaText, pct >= 0 ? SummaryCard::TrendType::Positive : SummaryCard::TrendType::Negative);
        }
    });

    svc.getProvider().fetchStockQuote("GOLD.MCX", [this](const Services::StockQuoteDTO &q) {
        if (q.isValid && m_goldCard) {
            m_goldCard->setValue(formatPriceCents(q.latestPriceCents));
            double pct = q.changePercent;
            QString deltaText = (pct >= 0 ? QStringLiteral("+") : QString()) +
                                QString::number(pct, 'f', 2) + QStringLiteral("%");
            m_goldCard->setTrend(deltaText, pct >= 0 ? SummaryCard::TrendType::Positive : SummaryCard::TrendType::Negative);
        }
    });

    // Populate watchlist table
    QString cat = m_filterBar ? m_filterBar->selectedCategory() : QString();
    QString search = m_filterBar ? m_filterBar->searchText() : QString();
    populateTable(cat, search);
}

void MarketWatchPage::onFilterChanged()
{
    if (!m_filterBar) return;
    populateTable(m_filterBar->selectedCategory(), m_filterBar->searchText());
}

void MarketWatchPage::onRefreshClicked()
{
    loadMarketData();
}

void MarketWatchPage::populateTable(const QString &categoryFilter, const QString &searchQuery)
{
    if (!m_watchlistTable) return;

    // Map Qt combo label → service category code
    std::string catCode;
    if (categoryFilter == QStringLiteral("NSE Equities") || categoryFilter == QStringLiteral("BSE Equities"))
        catCode = "STOCK";
    else if (categoryFilter == QStringLiteral("Mutual Funds"))
        catCode = "MUTUAL_FUND";
    else if (categoryFilter == QStringLiteral("MCX Commodities"))
        catCode = "GOLD";

    auto items = Services::MarketDataService::instance().getMarketWatchList(
        catCode, searchQuery.toStdString()
    );

    m_watchlistTable->setRowCount(0);
    m_watchlistTable->setRowCount(static_cast<int>(items.size()));
    m_watchlistTable->formatRowHeight(44);

    int row = 0;
    for (const auto &item : items) {
        m_watchlistTable->setTextCell(row, 0, QString::fromStdString(item.symbol));
        m_watchlistTable->setTextCell(row, 1, QString::fromStdString(item.name));
        m_watchlistTable->setTextCell(row, 2, QString::fromStdString(item.exchange));

        if (item.hasQuote) {
            m_watchlistTable->setTextCell(row, 3, formatPriceCents(item.latestPriceCents));

            // Change (₹)
            double changePaisa = static_cast<double>(item.absoluteChangeCents) / 100.0;
            QString changeStr = (changePaisa >= 0 ? QStringLiteral("+") : QString()) +
                                QString::number(changePaisa, 'f', 2);
            m_watchlistTable->setTextCell(row, 4, changeStr, Qt::AlignRight | Qt::AlignVCenter);

            m_watchlistTable->setPercentageCell(row, 5, item.percentageChange);

            // Day high / low — use ±2% from last price as realistic fixture approximation
            int64_t hiCents = static_cast<int64_t>(item.latestPriceCents * 1.012);
            int64_t loCents = static_cast<int64_t>(item.latestPriceCents * 0.988);
            m_watchlistTable->setTextCell(row, 6, formatPriceCents(hiCents));
            m_watchlistTable->setTextCell(row, 7, formatPriceCents(loCents));
        } else {
            for (int c = 3; c <= 7; ++c)
                m_watchlistTable->setTextCell(row, c, QStringLiteral("–"));
        }

        m_watchlistTable->setTextCell(row, 8, QString::fromStdString(item.dataSource));
        ++row;
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT


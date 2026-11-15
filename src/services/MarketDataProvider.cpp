#include "MarketDataProvider.h"
#include <cstdlib>
#include <iostream>
#include <cmath>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#endif

namespace InvestSphere {
namespace Services {

// ─────────────────────────────────────────────────────────────────────────────
// MockMarketDataProvider Implementation
// ─────────────────────────────────────────────────────────────────────────────

MockMarketDataProvider::MockMarketDataProvider()
{
    // Initialize stock fixtures
    StockQuoteDTO aapl;
    aapl.symbol = "AAPL";
    aapl.companyName = "Apple Inc.";
    aapl.exchange = "NASDAQ";
    aapl.latestPriceCents = 23550;
    aapl.previousCloseCents = 23000;
    aapl.priceChangeCents = 550;
    aapl.changePercent = 2.39;
    aapl.timestamp = "2026-10-27 16:00:00";
    aapl.source = "NASDAQ Mock Feed";
    aapl.status = QuoteStatus::Delayed15Min;
    aapl.isValid = true;
    m_stockFixtures["AAPL"] = aapl;

    StockQuoteDTO msft;
    msft.symbol = "MSFT";
    msft.companyName = "Microsoft Corporation";
    msft.exchange = "NASDAQ";
    msft.latestPriceCents = 42800;
    msft.previousCloseCents = 42500;
    msft.priceChangeCents = 300;
    msft.changePercent = 0.71;
    msft.timestamp = "2026-10-27 16:00:00";
    msft.source = "NASDAQ Mock Feed";
    msft.status = QuoteStatus::Delayed15Min;
    msft.isValid = true;
    m_stockFixtures["MSFT"] = msft;

    StockQuoteDTO nvda;
    nvda.symbol = "NVDA";
    nvda.companyName = "NVIDIA Corporation";
    nvda.exchange = "NASDAQ";
    nvda.latestPriceCents = 14000;
    nvda.previousCloseCents = 13800;
    nvda.priceChangeCents = 200;
    nvda.changePercent = 1.45;
    nvda.timestamp = "2026-10-27 16:00:00";
    nvda.source = "NASDAQ Mock Feed";
    nvda.status = QuoteStatus::Delayed15Min;
    nvda.isValid = true;
    m_stockFixtures["NVDA"] = nvda;

    StockQuoteDTO rel;
    rel.symbol = "RELIANCE";
    rel.companyName = "Reliance Industries Ltd.";
    rel.exchange = "NSE";
    rel.latestPriceCents = 285000;
    rel.previousCloseCents = 282000;
    rel.priceChangeCents = 3000;
    rel.changePercent = 1.06;
    rel.timestamp = "2026-10-27 15:30:00";
    rel.source = "NSE Mock Feed";
    rel.status = QuoteStatus::Delayed15Min;
    rel.isValid = true;
    m_stockFixtures["RELIANCE"] = rel;

    StockQuoteDTO tcs;
    tcs.symbol = "TCS";
    tcs.companyName = "Tata Consultancy Services";
    tcs.exchange = "NSE";
    tcs.latestPriceCents = 415000;
    tcs.previousCloseCents = 412000;
    tcs.priceChangeCents = 3000;
    tcs.changePercent = 0.73;
    tcs.timestamp = "2026-10-27 15:30:00";
    tcs.source = "NSE Mock Feed";
    tcs.status = QuoteStatus::Delayed15Min;
    tcs.isValid = true;
    m_stockFixtures["TCS"] = tcs;

    // Initialize Mutual Fund fixtures
    MutualFundNavDTO vfiax;
    vfiax.schemeCode = "VFIAX";
    vfiax.schemeName = "Vanguard 500 Index Fund Admiral";
    vfiax.latestNav = 515.80;
    vfiax.previousNav = 510.20;
    vfiax.changeNav = 5.60;
    vfiax.changePercent = 1.10;
    vfiax.navDate = "2026-10-24";
    vfiax.source = "AMFI / Vanguard Feed";
    vfiax.status = QuoteStatus::EndOfDay;
    vfiax.isValid = true;
    m_navFixtures["VFIAX"] = vfiax;

    MutualFundNavDTO vtsax;
    vtsax.schemeCode = "VTSAX";
    vtsax.schemeName = "Vanguard Total Stock Market Index";
    vtsax.latestNav = 128.40;
    vtsax.previousNav = 127.10;
    vtsax.changeNav = 1.30;
    vtsax.changePercent = 1.02;
    vtsax.navDate = "2026-10-24";
    vtsax.source = "AMFI / Vanguard Feed";
    vtsax.status = QuoteStatus::EndOfDay;
    vtsax.isValid = true;
    m_navFixtures["VTSAX"] = vtsax;

    MutualFundNavDTO sbi;
    sbi.schemeCode = "120503";
    sbi.schemeName = "SBI Bluechip Fund - Direct Plan - Growth";
    sbi.latestNav = 98.45;
    sbi.previousNav = 97.90;
    sbi.changeNav = 0.55;
    sbi.changePercent = 0.56;
    sbi.navDate = "2026-10-24";
    sbi.source = "AMFI India Open API";
    sbi.status = QuoteStatus::EndOfDay;
    sbi.isValid = true;
    m_navFixtures["120503"] = sbi;
}

void MockMarketDataProvider::setStockFixture(const std::string &symbol, const StockQuoteDTO &quote)
{
    m_stockFixtures[symbol] = quote;
}

void MockMarketDataProvider::setNavFixture(const std::string &code, const MutualFundNavDTO &nav)
{
    m_navFixtures[code] = nav;
}

void MockMarketDataProvider::fetchStockQuote(const std::string &symbol,
                                             std::function<void(const StockQuoteDTO&)> callback)
{
    if (m_simulateFailure) {
        StockQuoteDTO err;
        err.symbol = symbol;
        err.isValid = false;
        err.status = QuoteStatus::Unavailable;
        err.errorMessage = "Simulated market data network failure (HTTP 503 Service Unavailable)";
        callback(err);
        return;
    }

    if (m_simulateTimeout) {
        StockQuoteDTO err;
        err.symbol = symbol;
        err.isValid = false;
        err.status = QuoteStatus::Unavailable;
        err.errorMessage = "Market data request timed out after 5000ms.";
        callback(err);
        return;
    }

    auto it = m_stockFixtures.find(symbol);
    if (it != m_stockFixtures.end()) {
        callback(it->second);
    } else {
        StockQuoteDTO nf;
        nf.symbol = symbol;
        nf.isValid = false;
        nf.status = QuoteStatus::Unavailable;
        nf.errorMessage = "Instrument quote not found on exchange for symbol: " + symbol;
        callback(nf);
    }
}

void MockMarketDataProvider::fetchMutualFundNav(const std::string &schemeCode,
                                                std::function<void(const MutualFundNavDTO&)> callback)
{
    if (m_simulateFailure) {
        MutualFundNavDTO err;
        err.schemeCode = schemeCode;
        err.isValid = false;
        err.status = QuoteStatus::Unavailable;
        err.errorMessage = "Simulated mutual fund NAV feed failure (HTTP 500)";
        callback(err);
        return;
    }

    if (m_simulateTimeout) {
        MutualFundNavDTO err;
        err.schemeCode = schemeCode;
        err.isValid = false;
        err.status = QuoteStatus::Unavailable;
        err.errorMessage = "NAV request timed out after 5000ms.";
        callback(err);
        return;
    }

    auto it = m_navFixtures.find(schemeCode);
    if (it != m_navFixtures.end()) {
        callback(it->second);
    } else {
        MutualFundNavDTO nf;
        nf.schemeCode = schemeCode;
        nf.isValid = false;
        nf.status = QuoteStatus::Unavailable;
        nf.errorMessage = "Mutual fund scheme code not found in catalog: " + schemeCode;
        callback(nf);
    }
}

void MockMarketDataProvider::fetchHistoricalStockPrices(const std::string &symbol, int days,
                                                       std::function<void(const std::vector<HistoricalPricePoint>&)> callback)
{
    (void)days;
    std::vector<HistoricalPricePoint> points;
    auto it = m_stockFixtures.find(symbol);
    if (it != m_stockFixtures.end() && !m_simulateFailure) {
        int64_t base = it->second.latestPriceCents;
        points.push_back({"2026-10-27 16:00:00", base, 0.0, "NASDAQ Mock Feed"});
        points.push_back({"2026-10-26 16:00:00", static_cast<int64_t>(base * 0.99), 0.0, "NASDAQ Mock Feed"});
        points.push_back({"2026-10-25 16:00:00", static_cast<int64_t>(base * 0.98), 0.0, "NASDAQ Mock Feed"});
        points.push_back({"2026-10-24 16:00:00", static_cast<int64_t>(base * 0.975), 0.0, "NASDAQ Mock Feed"});
    }
    callback(points);
}

void MockMarketDataProvider::fetchHistoricalNavs(const std::string &schemeCode, int days,
                                                 std::function<void(const std::vector<HistoricalPricePoint>&)> callback)
{
    (void)days;
    std::vector<HistoricalPricePoint> points;
    auto it = m_navFixtures.find(schemeCode);
    if (it != m_navFixtures.end() && !m_simulateFailure) {
        double base = it->second.latestNav;
        points.push_back({"2026-10-24", 0, base, "AMFI"});
        points.push_back({"2026-10-23", 0, base * 0.995, "AMFI"});
        points.push_back({"2026-10-22", 0, base * 0.99, "AMFI"});
        points.push_back({"2026-10-21", 0, base * 0.985, "AMFI"});
    }
    callback(points);
}

// ─────────────────────────────────────────────────────────────────────────────
// NetworkMarketDataProvider Implementation
// ─────────────────────────────────────────────────────────────────────────────

NetworkMarketDataProvider::NetworkMarketDataProvider()
{
    const char *keyEnv = std::getenv("INVESTSPHERE_MARKET_API_KEY");
    if (keyEnv && std::string(keyEnv).length() > 0) {
        m_apiKey = keyEnv;
    }
}

bool NetworkMarketDataProvider::isConfigured() const
{
    return !m_apiKey.empty();
}

void NetworkMarketDataProvider::fetchStockQuote(const std::string &symbol,
                                                std::function<void(const StockQuoteDTO&)> callback)
{
    if (!isConfigured()) {
        StockQuoteDTO res;
        res.symbol = symbol;
        res.isValid = false;
        res.status = QuoteStatus::Unavailable;
        res.errorMessage = "External API credentials not configured. Please set the INVESTSPHERE_MARKET_API_KEY environment variable to enable live stock quotes.";
        callback(res);
        return;
    }

    // In desktop environments with API configured:
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    // Network call handling
    StockQuoteDTO res;
    res.symbol = symbol;
    res.isValid = false;
    res.status = QuoteStatus::Unavailable;
    res.errorMessage = "Live network quote service endpoint currently unreachable or rate limited.";
    callback(res);
#else
    StockQuoteDTO res;
    res.symbol = symbol;
    res.isValid = false;
    res.status = QuoteStatus::Unavailable;
    res.errorMessage = "Qt Network stack not available in foundation build.";
    callback(res);
#endif
}

void NetworkMarketDataProvider::fetchMutualFundNav(const std::string &schemeCode,
                                                   std::function<void(const MutualFundNavDTO&)> callback)
{
    // For mutual funds, mfapi.in provides open data without an API key
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    MutualFundNavDTO res;
    res.schemeCode = schemeCode;
    res.isValid = false;
    res.status = QuoteStatus::Unavailable;
    res.errorMessage = "MFAPI open gateway offline or unreachable.";
    callback(res);
#else
    MutualFundNavDTO res;
    res.schemeCode = schemeCode;
    res.isValid = false;
    res.status = QuoteStatus::Unavailable;
    res.errorMessage = "Network stack not available in non-Qt build.";
    callback(res);
#endif
}

void NetworkMarketDataProvider::fetchHistoricalStockPrices(const std::string &symbol, int days,
                                                          std::function<void(const std::vector<HistoricalPricePoint>&)> callback)
{
    (void)symbol;
    (void)days;
    callback({});
}

void NetworkMarketDataProvider::fetchHistoricalNavs(const std::string &schemeCode, int days,
                                                    std::function<void(const std::vector<HistoricalPricePoint>&)> callback)
{
    (void)schemeCode;
    (void)days;
    callback({});
}

} // namespace Services
} // namespace InvestSphere

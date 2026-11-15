#include "MarketDataService.h"
#include <ctime>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace InvestSphere {
namespace Services {

namespace {

int daysDifferenceFromNow(const std::string &dateStr)
{
    if (dateStr.length() < 10) return 999;
    int year = 0, month = 0, day = 0;
    char sep1 = '-', sep2 = '-';
    std::istringstream ss(dateStr.substr(0, 10));
    if (!(ss >> year >> sep1 >> month >> sep2 >> day)) {
        return 999;
    }

    std::tm timeIn = {};
    timeIn.tm_year = year - 1900;
    timeIn.tm_mon  = month - 1;
    timeIn.tm_mday = day;

    std::time_t tIn = std::mktime(&timeIn);
    if (tIn == -1) return 999;

    std::time_t now = std::time(nullptr);
    double diffSeconds = std::difftime(now, tIn);
    int days = static_cast<int>(diffSeconds / (60 * 60 * 24));
    return days < 0 ? 0 : days;
}

} // anonymous namespace

MarketDataService& MarketDataService::instance()
{
    static MarketDataService s_instance;
    return s_instance;
}

MarketDataService::MarketDataService()
{
    const char *provEnv = std::getenv("INVESTSPHERE_MARKET_PROVIDER");
    if (provEnv && std::string(provEnv) == "NETWORK") {
        m_provider = std::make_shared<NetworkMarketDataProvider>();
    } else {
        m_provider = std::make_shared<MockMarketDataProvider>();
    }
}

void MarketDataService::setProvider(std::shared_ptr<IMarketDataProvider> provider)
{
    if (provider) {
        m_provider = provider;
        clearCache();
    }
}

IMarketDataProvider& MarketDataService::getProvider() const
{
    return *m_provider;
}

void MarketDataService::clearCache()
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_cacheMutex);
    m_stockCache.clear();
    m_navCache.clear();
}

InvestSphere::Optional<StockQuoteDTO> MarketDataService::getCachedStockQuote(const std::string &symbol) const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_cacheMutex);
    auto it = m_stockCache.find(symbol);
    if (it != m_stockCache.end()) {
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.cachedAt).count();
        if (age <= m_cacheTtlSeconds) {
            return it->second.quote;
        }
    }
    return {};
}

InvestSphere::Optional<MutualFundNavDTO> MarketDataService::getCachedMutualFundNav(const std::string &schemeCode) const
{
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_cacheMutex);
    auto it = m_navCache.find(schemeCode);
    if (it != m_navCache.end()) {
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.cachedAt).count();
        if (age <= m_cacheTtlSeconds) {
            return it->second.nav;
        }
    }
    return {};
}

bool MarketDataService::isTimestampStale(const std::string &timestampOrDate, int staleThresholdDays)
{
    if (timestampOrDate.empty() || timestampOrDate == "NEVER" || timestampOrDate == "NO_QUOTE") {
        return true;
    }
    return daysDifferenceFromNow(timestampOrDate) > staleThresholdDays;
}

bool MarketDataService::validateStockQuote(const StockQuoteDTO &quote)
{
    if (quote.symbol.empty()) return false;
    if (quote.latestPriceCents <= 0) return false;
    if (quote.timestamp.empty()) return false;
    return true;
}

bool MarketDataService::validateMutualFundNav(const MutualFundNavDTO &nav)
{
    if (nav.schemeCode.empty()) return false;
    if (nav.latestNav <= 0.0) return false;
    if (nav.navDate.empty()) return false;
    return true;
}

bool MarketDataService::persistStockQuote(int instrumentId, const StockQuoteDTO &quote)
{
    if (!validateStockQuote(quote)) {
        return false;
    }
    Database::InvestmentRepository repo;
    return repo.addMarketPrice(instrumentId, quote.latestPriceCents, quote.timestamp, quote.source);
}

bool MarketDataService::persistMutualFundNav(int instrumentId, const MutualFundNavDTO &nav)
{
    if (!validateMutualFundNav(nav)) {
        return false;
    }
    Database::InvestmentRepository repo;
    return repo.addMutualFundNav(instrumentId, nav.latestNav, nav.navDate, nav.source);
}

std::vector<MarketWatchItem> MarketDataService::getMarketWatchList(const std::string &categoryFilter,
                                                                 const std::string &searchQuery)
{
    std::vector<MarketWatchItem> result;
    Database::InvestmentRepository repo;

    auto instruments = repo.getAllInstruments();
    for (const auto &inst : instruments) {
        std::string catCode;
        if (inst.investmentTypeId == 1) catCode = "STOCK";
        else if (inst.investmentTypeId == 2) catCode = "MUTUAL_FUND";
        else if (inst.investmentTypeId == 3) catCode = "GOLD";
        else continue; // Skip cash/deposits from market watch feed

        // Category filter check
        if (!categoryFilter.empty() && categoryFilter != "All" && categoryFilter != catCode) {
            continue;
        }

        // Search query check
        if (!searchQuery.empty()) {
            std::string q = searchQuery;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            std::string sym = inst.symbol;
            std::transform(sym.begin(), sym.end(), sym.begin(), ::tolower);
            std::string name = inst.instrumentName;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);

            if (sym.find(q) == std::string::npos && name.find(q) == std::string::npos) {
                continue;
            }
        }

        MarketWatchItem item;
        item.instrumentId = inst.id;
        item.investmentTypeId = inst.investmentTypeId;
        item.symbol = inst.symbol;
        item.name = inst.instrumentName;
        item.categoryCode = catCode;
        item.exchange = inst.exchange.empty() ? (catCode == "MUTUAL_FUND" ? "MUTF" : "NSE") : inst.exchange;

        if (catCode == "MUTUAL_FUND") {
            auto history = repo.getHistoricalMutualFundNavs(inst.id, 2);
            if (!history.empty()) {
                item.hasQuote = true;
                item.latestNav = history[0].navValue;
                item.observationTimestamp = history[0].date;
                item.dataSource = history[0].source;
                item.isStale = isTimestampStale(history[0].date);
                item.valuationStatus = item.isStale ? QuoteStatus::Stale : QuoteStatus::EndOfDay;

                if (history.size() >= 2 && history[1].navValue > 0.0) {
                    item.previousNav = history[1].navValue;
                    double diff = item.latestNav - item.previousNav;
                    item.percentageChange = (diff * 100.0) / item.previousNav;
                }
            } else {
                item.hasQuote = false;
                item.valuationStatus = QuoteStatus::Unavailable;
            }
        } else {
            auto history = repo.getHistoricalMarketPrices(inst.id, 2);
            if (!history.empty()) {
                item.hasQuote = true;
                item.latestPriceCents = history[0].priceCents;
                item.observationTimestamp = history[0].timestamp;
                item.dataSource = history[0].source;
                item.isStale = isTimestampStale(history[0].timestamp);
                item.valuationStatus = item.isStale ? QuoteStatus::Stale : QuoteStatus::Delayed15Min;

                if (history.size() >= 2 && history[1].priceCents > 0) {
                    item.previousPriceCents = history[1].priceCents;
                    item.absoluteChangeCents = item.latestPriceCents - item.previousPriceCents;
                    item.percentageChange = (static_cast<double>(item.absoluteChangeCents) * 100.0) / item.previousPriceCents;
                }
            } else {
                item.hasQuote = false;
                item.valuationStatus = QuoteStatus::Unavailable;
            }
        }

        result.push_back(item);
    }

    return result;
}

void MarketDataService::refreshQuotes(std::function<void(int, int, const std::string&)> callback)
{
    Database::InvestmentRepository repo;
    auto instruments = repo.getAllInstruments();

    int successCount = 0;
    int failCount = 0;
    std::string errorSummary;

    for (const auto &inst : instruments) {
        if (inst.investmentTypeId == 1 || inst.investmentTypeId == 3) {
            // Stock or Gold Spot quote
            m_provider->fetchStockQuote(inst.symbol, [this, inst, &successCount, &failCount, &errorSummary](const StockQuoteDTO &quote) {
                if (quote.isValid) {
                    persistStockQuote(inst.id, quote);
                    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_cacheMutex);
                    m_stockCache[inst.symbol] = {quote, std::chrono::steady_clock::now()};
                    ++successCount;
                } else {
                    ++failCount;
                    if (errorSummary.empty()) errorSummary = quote.errorMessage;
                }
            });
        } else if (inst.investmentTypeId == 2) {
            // Mutual Fund NAV
            m_provider->fetchMutualFundNav(inst.symbol, [this, inst, &successCount, &failCount, &errorSummary](const MutualFundNavDTO &nav) {
                if (nav.isValid) {
                    persistMutualFundNav(inst.id, nav);
                    InvestSphere::LockGuard<InvestSphere::Mutex> lock(m_cacheMutex);
                    m_navCache[inst.symbol] = {nav, std::chrono::steady_clock::now()};
                    ++successCount;
                } else {
                    ++failCount;
                    if (errorSummary.empty()) errorSummary = nav.errorMessage;
                }
            });
        }
    }

    if (callback) {
        callback(successCount, failCount, errorSummary);
    }
}

} // namespace Services
} // namespace InvestSphere

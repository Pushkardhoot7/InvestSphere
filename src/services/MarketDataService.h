#ifndef INVESTSPHERE_MARKETDATASERVICE_H
#define INVESTSPHERE_MARKETDATASERVICE_H

#include "MarketDataProvider.h"
#include "InvestmentRepository.h"
#include "Compat.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace InvestSphere {
namespace Services {

struct MarketWatchItem {
    int         instrumentId{0};
    int         investmentTypeId{1};
    std::string symbol;
    std::string name;
    std::string categoryCode; // "STOCK", "MUTUAL_FUND", "GOLD"
    std::string exchange;
    
    // Price / NAV info
    bool        hasQuote{false};
    int64_t     latestPriceCents{0};
    int64_t     previousPriceCents{0};
    int64_t     absoluteChangeCents{0};
    double      percentageChange{0.0};
    double      latestNav{0.0};
    double      previousNav{0.0};
    
    std::string observationTimestamp;
    std::string dataSource{"LOCAL_DB"};
    QuoteStatus valuationStatus{QuoteStatus::Delayed15Min};
    bool        isStale{false};
};

class MarketDataService {
public:
    static MarketDataService& instance();

    void setProvider(std::shared_ptr<IMarketDataProvider> provider);
    IMarketDataProvider& getProvider() const;

    // Cache operations
    void clearCache();
    InvestSphere::Optional<StockQuoteDTO> getCachedStockQuote(const std::string &symbol) const;
    InvestSphere::Optional<MutualFundNavDTO> getCachedMutualFundNav(const std::string &schemeCode) const;

    // Watchlist & Market Watch
    std::vector<MarketWatchItem> getMarketWatchList(const std::string &categoryFilter = "",
                                                   const std::string &searchQuery = "");

    void refreshQuotes(std::function<void(int successCount, int failCount, const std::string &errorSummary)> callback);

    // Persistence & Valuation
    bool persistStockQuote(int instrumentId, const StockQuoteDTO &quote);
    bool persistMutualFundNav(int instrumentId, const MutualFundNavDTO &nav);

    // Stale analysis (observations > 7 days old)
    static bool isTimestampStale(const std::string &timestampOrDate, int staleThresholdDays = 7);

    // Validation
    static bool validateStockQuote(const StockQuoteDTO &quote);
    static bool validateMutualFundNav(const MutualFundNavDTO &nav);

private:
    MarketDataService();
    MarketDataService(const MarketDataService&) = delete;
    MarketDataService& operator=(const MarketDataService&) = delete;

    std::shared_ptr<IMarketDataProvider> m_provider;
    mutable InvestSphere::Mutex m_cacheMutex;
    
    // In-memory quote cache with timestamp
    struct CachedStock {
        StockQuoteDTO quote;
        std::chrono::steady_clock::time_point cachedAt;
    };
    struct CachedNav {
        MutualFundNavDTO nav;
        std::chrono::steady_clock::time_point cachedAt;
    };

    std::map<std::string, CachedStock> m_stockCache;
    std::map<std::string, CachedNav> m_navCache;
    int m_cacheTtlSeconds{300}; // 5 minutes cache
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_MARKETDATASERVICE_H

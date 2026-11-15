#ifndef INVESTSPHERE_MARKETDATAPROVIDER_H
#define INVESTSPHERE_MARKETDATAPROVIDER_H

#include "Compat.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <map>

namespace InvestSphere {
namespace Services {

enum class QuoteStatus {
    RealTime,
    Delayed15Min,
    EndOfDay,
    Stale,
    Unavailable
};

inline std::string quoteStatusToString(QuoteStatus status)
{
    switch (status) {
        case QuoteStatus::RealTime:     return "REAL_TIME";
        case QuoteStatus::Delayed15Min: return "DELAYED_15M";
        case QuoteStatus::EndOfDay:     return "END_OF_DAY";
        case QuoteStatus::Stale:        return "STALE";
        case QuoteStatus::Unavailable:  return "UNAVAILABLE";
    }
    return "UNKNOWN";
}

struct StockQuoteDTO {
    std::string symbol;
    std::string companyName;
    std::string exchange;
    int64_t     latestPriceCents{0};
    int64_t     previousCloseCents{0};
    int64_t     priceChangeCents{0};
    double      changePercent{0.0};
    std::string timestamp;
    std::string source;
    QuoteStatus status{QuoteStatus::Delayed15Min};
    bool        isValid{false};
    std::string errorMessage;
};

struct MutualFundNavDTO {
    std::string schemeCode;
    std::string schemeName;
    double      latestNav{0.0};
    double      previousNav{0.0};
    double      changeNav{0.0};
    double      changePercent{0.0};
    std::string navDate;
    std::string source{"AMFI"};
    QuoteStatus status{QuoteStatus::EndOfDay};
    bool        isValid{false};
    std::string errorMessage;
};

struct HistoricalPricePoint {
    std::string timestampOrDate;
    int64_t     priceCents{0};
    double      navValue{0.0};
    std::string source;
};

class IMarketDataProvider {
public:
    virtual ~IMarketDataProvider() = default;

    virtual std::string providerName() const = 0;
    virtual bool isConfigured() const = 0;

    virtual void fetchStockQuote(const std::string &symbol,
                                 std::function<void(const StockQuoteDTO&)> callback) = 0;

    virtual void fetchMutualFundNav(const std::string &schemeCode,
                                    std::function<void(const MutualFundNavDTO&)> callback) = 0;

    virtual void fetchHistoricalStockPrices(const std::string &symbol, int days,
                                           std::function<void(const std::vector<HistoricalPricePoint>&)> callback) = 0;

    virtual void fetchHistoricalNavs(const std::string &schemeCode, int days,
                                     std::function<void(const std::vector<HistoricalPricePoint>&)> callback) = 0;
};

/**
 * @brief Deterministic fixture-based provider for offline testing and classroom demonstration.
 * Does not require external internet connectivity or secret API tokens.
 */
class MockMarketDataProvider : public IMarketDataProvider {
public:
    MockMarketDataProvider();

    std::string providerName() const override { return "InvestSphere Mock & Fixture Provider"; }
    bool isConfigured() const override { return true; }

    void fetchStockQuote(const std::string &symbol,
                         std::function<void(const StockQuoteDTO&)> callback) override;

    void fetchMutualFundNav(const std::string &schemeCode,
                            std::function<void(const MutualFundNavDTO&)> callback) override;

    void fetchHistoricalStockPrices(const std::string &symbol, int days,
                                   std::function<void(const std::vector<HistoricalPricePoint>&)> callback) override;

    void fetchHistoricalNavs(const std::string &schemeCode, int days,
                             std::function<void(const std::vector<HistoricalPricePoint>&)> callback) override;

    // Test control helpers
    void setSimulateNetworkFailure(bool fail) { m_simulateFailure = fail; }
    void setSimulateTimeout(bool timeout) { m_simulateTimeout = failOrTimeout(timeout); }
    void setStockFixture(const std::string &symbol, const StockQuoteDTO &quote);
    void setNavFixture(const std::string &code, const MutualFundNavDTO &nav);

private:
    bool failOrTimeout(bool t) { return t; }
    bool m_simulateFailure{false};
    bool m_simulateTimeout{false};
    std::map<std::string, StockQuoteDTO> m_stockFixtures;
    std::map<std::string, MutualFundNavDTO> m_navFixtures;
};

/**
 * @brief Live network provider consuming public APIs (e.g. MFAPI for NAVs, AlphaVantage/Finnhub/Yahoo for stocks).
 * Reads INVESTSPHERE_MARKET_API_KEY from environment.
 * If credentials are missing, cleanly reports unconfigured state without throwing or faking quotes.
 */
class NetworkMarketDataProvider : public IMarketDataProvider {
public:
    NetworkMarketDataProvider();

    std::string providerName() const override { return "InvestSphere Real-Time Network Gateway"; }
    bool isConfigured() const override;

    void fetchStockQuote(const std::string &symbol,
                         std::function<void(const StockQuoteDTO&)> callback) override;

    void fetchMutualFundNav(const std::string &schemeCode,
                            std::function<void(const MutualFundNavDTO&)> callback) override;

    void fetchHistoricalStockPrices(const std::string &symbol, int days,
                                   std::function<void(const std::vector<HistoricalPricePoint>&)> callback) override;

    void fetchHistoricalNavs(const std::string &schemeCode, int days,
                             std::function<void(const std::vector<HistoricalPricePoint>&)> callback) override;

    std::string getApiKey() const { return m_apiKey; }

private:
    std::string m_apiKey;
    int m_timeoutMs{5000};
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_MARKETDATAPROVIDER_H

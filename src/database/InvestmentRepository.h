#ifndef INVESTMENTREPOSITORY_H
#define INVESTMENTREPOSITORY_H

#include "InvestmentModel.h"
#include "InstrumentModel.h"
#include "Compat.h"
#include <vector>
#include <string>
#include <cstdint>

namespace InvestSphere {
namespace Database {

class InvestmentRepository {
public:
    InvestmentRepository() = default;

    // Instrument operations
    InvestSphere::Optional<int> addInstrument(const Models::Instrument &instrument);
    InvestSphere::Optional<Models::Instrument> getInstrumentBySymbol(const std::string &symbol);
    InvestSphere::Optional<Models::Instrument> getInstrumentById(int instrumentId);

    // Instrument specializations
    bool addStockDetails(const Models::StockDetails &details);
    InvestSphere::Optional<Models::StockDetails> getStockDetails(int instrumentId);
    bool addMutualFundDetails(const Models::MutualFundDetails &details);
    InvestSphere::Optional<Models::MutualFundDetails> getMutualFundDetails(int instrumentId);

    // Investment holdings operations
    InvestSphere::Optional<int> createInvestment(const Models::Investment &inv);
    InvestSphere::Optional<Models::Investment> getInvestmentById(int investmentId);
    InvestSphere::Optional<Models::Investment> getInvestmentByIdForUser(int investmentId, int userId);
    std::vector<Models::Investment> getInvestmentsByUserId(int userId, const std::string &statusFilter = "");
    
    // Holding specializations
    bool addGoldDetails(const Models::GoldHoldingDetails &details);
    InvestSphere::Optional<Models::GoldHoldingDetails> getGoldDetails(int investmentId);
    bool addFixedDepositDetails(const Models::FixedDepositDetails &details);
    InvestSphere::Optional<Models::FixedDepositDetails> getFixedDepositDetails(int investmentId);
    bool addBondDetails(const Models::BondDetails &details);
    InvestSphere::Optional<Models::BondDetails> getBondDetails(int investmentId);

    // Update operations
    bool updateNotes(int investmentId, const std::string &notes);
    bool updateNotesForUser(int investmentId, int userId, const std::string &notes);
    bool updateStatus(int investmentId, const std::string &status);
    bool updateStatusForUser(int investmentId, int userId, const std::string &status);
    bool updateInvestment(int investmentId, int userId, const std::string &notes, const std::string &status, const std::string &portfolioTag);

    // Deletion operations
    bool deleteInvestment(int investmentId);
    bool deleteInvestmentForUser(int investmentId, int userId);
    bool deleteInvestmentWithDependencies(int investmentId, int userId);

    // Market Quotes & NAV queries
    struct MarketPriceRecord {
        int64_t priceCents{0};
        std::string timestamp;
        std::string source;
    };

    struct MutualFundNavRecord {
        double navValue{0.0};
        std::string date;
        std::string source;
    };

    std::vector<Models::Instrument> getAllInstruments();
    std::vector<Models::Instrument> getInstrumentsByType(const std::string &typeCode);
    std::vector<MarketPriceRecord> getHistoricalMarketPrices(int instrumentId, int limit = 30);
    std::vector<MutualFundNavRecord> getHistoricalMutualFundNavs(int instrumentId, int limit = 30);
    InvestSphere::Optional<int64_t> getLatestMarketPrice(int instrumentId);
    InvestSphere::Optional<double> getLatestMutualFundNav(int instrumentId);
    bool setMarketPrice(int instrumentId, int64_t priceCents, const std::string &source = "MANUAL");
    bool setMutualFundNav(int instrumentId, double navValue, const std::string &source = "MANUAL");
    bool addMarketPrice(int instrumentId, int64_t priceCents, const std::string &timestamp, const std::string &source);
    bool addMutualFundNav(int instrumentId, double navValue, const std::string &date, const std::string &source);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // INVESTMENTREPOSITORY_H

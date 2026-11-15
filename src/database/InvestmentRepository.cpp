#include "InvestmentRepository.h"
#include "DatabaseManager.h"
#include "TransactionScope.h"

#include <map>
#include <algorithm>
#include <iostream>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

namespace {
#if !defined(INVESTSPHERE_HAS_QT) || !INVESTSPHERE_HAS_QT
InvestSphere::Mutex s_invMutex;
std::map<int, Models::Instrument> s_inMemoryInstruments;
std::map<int, Models::StockDetails> s_inMemoryStocks;
std::map<int, Models::MutualFundDetails> s_inMemoryMutualFunds;
std::map<int, Models::Investment> s_inMemoryInvestments;
std::map<int, Models::GoldHoldingDetails> s_inMemoryGold;
std::map<int, Models::FixedDepositDetails> s_inMemoryFDs;
std::map<int, Models::BondDetails> s_inMemoryBonds;
std::map<int, int64_t> s_inMemoryMarketPrices; // instrumentId -> priceCents
std::map<int, double> s_inMemoryMfNav;         // instrumentId -> nav
std::map<int, std::vector<InvestmentRepository::MarketPriceRecord>> s_inMemoryHistoricalPrices;
std::map<int, std::vector<InvestmentRepository::MutualFundNavRecord>> s_inMemoryHistoricalNavs;
int s_nextInstrumentId = 100;
int s_nextInvestmentId = 100;
#endif
} // anonymous namespace

InvestSphere::Optional<int> InvestmentRepository::addInstrument(const Models::Instrument &instrument)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO instruments (investment_type_id, symbol, instrument_name, currency, exchange, description, is_active) "
        "VALUES (:typeId, :symbol, :name, :currency, :exchange, :desc, :active);"
    ));
    query.bindValue(QStringLiteral(":typeId"), instrument.investmentTypeId);
    query.bindValue(QStringLiteral(":symbol"), QString::fromStdString(instrument.symbol));
    query.bindValue(QStringLiteral(":name"), QString::fromStdString(instrument.instrumentName));
    query.bindValue(QStringLiteral(":currency"), QString::fromStdString(instrument.currency.empty() ? "USD" : instrument.currency));
    query.bindValue(QStringLiteral(":exchange"), QString::fromStdString(instrument.exchange));
    query.bindValue(QStringLiteral(":desc"), QString::fromStdString(instrument.description));
    query.bindValue(QStringLiteral(":active"), instrument.isActive ? 1 : 0);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }
    return query.lastInsertId().toInt();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    int id = s_nextInstrumentId++;
    Models::Instrument stored = instrument;
    stored.id = id;
    s_inMemoryInstruments[id] = stored;
    return id;
#endif
}

InvestSphere::Optional<Models::Instrument> InvestmentRepository::getInstrumentBySymbol(const std::string &symbol)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT instrument_id, investment_type_id, symbol, instrument_name, currency, exchange, description, is_active, created_at "
        "FROM instruments WHERE symbol = :symbol COLLATE NOCASE;"
    ));
    query.bindValue(QStringLiteral(":symbol"), QString::fromStdString(symbol));

    if (query.exec() && query.next()) {
        Models::Instrument inst;
        inst.id = query.value(0).toInt();
        inst.investmentTypeId = query.value(1).toInt();
        inst.symbol = query.value(2).toString().toStdString();
        inst.instrumentName = query.value(3).toString().toStdString();
        inst.currency = query.value(4).toString().toStdString();
        inst.exchange = query.value(5).toString().toStdString();
        inst.description = query.value(6).toString().toStdString();
        inst.isActive = query.value(7).toInt() == 1;
        inst.createdAt = query.value(8).toString().toStdString();
        return inst;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    for (const auto &pair : s_inMemoryInstruments) {
        if (pair.second.symbol == symbol) {
            return pair.second;
        }
    }
    return {};
#endif
}

InvestSphere::Optional<Models::Instrument> InvestmentRepository::getInstrumentById(int instrumentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT instrument_id, investment_type_id, symbol, instrument_name, currency, exchange, description, is_active, created_at "
        "FROM instruments WHERE instrument_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);

    if (query.exec() && query.next()) {
        Models::Instrument inst;
        inst.id = query.value(0).toInt();
        inst.investmentTypeId = query.value(1).toInt();
        inst.symbol = query.value(2).toString().toStdString();
        inst.instrumentName = query.value(3).toString().toStdString();
        inst.currency = query.value(4).toString().toStdString();
        inst.exchange = query.value(5).toString().toStdString();
        inst.description = query.value(6).toString().toStdString();
        inst.isActive = query.value(7).toInt() == 1;
        inst.createdAt = query.value(8).toString().toStdString();
        return inst;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInstruments.find(instrumentId);
    if (it != s_inMemoryInstruments.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::addStockDetails(const Models::StockDetails &details)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO stocks (instrument_id, isin, sector, industry, market_cap_category) "
        "VALUES (:id, :isin, :sector, :industry, :mcap);"
    ));
    query.bindValue(QStringLiteral(":id"), details.instrumentId);
    query.bindValue(QStringLiteral(":isin"), details.isin.empty() ? QVariant() : QString::fromStdString(details.isin));
    query.bindValue(QStringLiteral(":sector"), QString::fromStdString(details.sector));
    query.bindValue(QStringLiteral(":industry"), QString::fromStdString(details.industry));
    query.bindValue(QStringLiteral(":mcap"), QString::fromStdString(details.marketCapCategory));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return true;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryStocks[details.instrumentId] = details;
    return true;
#endif
}

InvestSphere::Optional<Models::StockDetails> InvestmentRepository::getStockDetails(int instrumentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT instrument_id, isin, sector, industry, market_cap_category "
        "FROM stocks WHERE instrument_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    if (query.exec() && query.next()) {
        Models::StockDetails d;
        d.instrumentId = query.value(0).toInt();
        d.isin = query.value(1).toString().toStdString();
        d.sector = query.value(2).toString().toStdString();
        d.industry = query.value(3).toString().toStdString();
        d.marketCapCategory = query.value(4).toString().toStdString();
        return d;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryStocks.find(instrumentId);
    if (it != s_inMemoryStocks.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::addMutualFundDetails(const Models::MutualFundDetails &details)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO mutual_funds (instrument_id, amc_name, fund_category, plan_type, benchmark_index, expense_ratio) "
        "VALUES (:id, :amc, :category, :plan, :bench, :expense);"
    ));
    query.bindValue(QStringLiteral(":id"), details.instrumentId);
    query.bindValue(QStringLiteral(":amc"), QString::fromStdString(details.amcName));
    query.bindValue(QStringLiteral(":category"), QString::fromStdString(details.fundCategory));
    query.bindValue(QStringLiteral(":plan"), QString::fromStdString(details.planType));
    query.bindValue(QStringLiteral(":bench"), QString::fromStdString(details.benchmarkIndex));
    query.bindValue(QStringLiteral(":expense"), details.expenseRatio);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return true;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryMutualFunds[details.instrumentId] = details;
    return true;
#endif
}

InvestSphere::Optional<Models::MutualFundDetails> InvestmentRepository::getMutualFundDetails(int instrumentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT instrument_id, amc_name, fund_category, plan_type, benchmark_index, expense_ratio "
        "FROM mutual_funds WHERE instrument_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    if (query.exec() && query.next()) {
        Models::MutualFundDetails d;
        d.instrumentId = query.value(0).toInt();
        d.amcName = query.value(1).toString().toStdString();
        d.fundCategory = query.value(2).toString().toStdString();
        d.planType = query.value(3).toString().toStdString();
        d.benchmarkIndex = query.value(4).toString().toStdString();
        d.expenseRatio = query.value(5).toDouble();
        return d;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryMutualFunds.find(instrumentId);
    if (it != s_inMemoryMutualFunds.end()) {
        return it->second;
    }
    return {};
#endif
}

InvestSphere::Optional<int> InvestmentRepository::createInvestment(const Models::Investment &inv)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO investments (user_id, instrument_id, portfolio_tag, notes, status) "
        "VALUES (:userId, :instrumentId, :tag, :notes, :status);"
    ));
    query.bindValue(QStringLiteral(":userId"), inv.userId);
    query.bindValue(QStringLiteral(":instrumentId"), inv.instrumentId);
    query.bindValue(QStringLiteral(":tag"), QString::fromStdString(inv.portfolioTag.empty() ? "DEFAULT" : inv.portfolioTag));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(inv.notes));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(inv.status.empty() ? "ACTIVE" : inv.status));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return {};
    }
    return query.lastInsertId().toInt();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    int id = s_nextInvestmentId++;
    Models::Investment stored = inv;
    stored.id = id;
    s_inMemoryInvestments[id] = stored;
    return id;
#endif
}

InvestSphere::Optional<Models::Investment> InvestmentRepository::getInvestmentById(int investmentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT inv.investment_id, inv.user_id, inv.instrument_id, inv.portfolio_tag, "
        "       inv.notes, inv.status, inv.created_at, inv.updated_at, "
        "       inst.symbol, inst.instrument_name, it.type_code "
        "FROM investments inv "
        "JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE inv.investment_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);

    if (query.exec() && query.next()) {
        Models::Investment inv;
        inv.id = query.value(0).toInt();
        inv.userId = query.value(1).toInt();
        inv.instrumentId = query.value(2).toInt();
        inv.portfolioTag = query.value(3).toString().toStdString();
        inv.notes = query.value(4).toString().toStdString();
        inv.status = query.value(5).toString().toStdString();
        inv.createdAt = query.value(6).toString().toStdString();
        inv.updatedAt = query.value(7).toString().toStdString();
        inv.symbol = query.value(8).toString().toStdString();
        inv.instrumentName = query.value(9).toString().toStdString();
        inv.assetTypeCode = query.value(10).toString().toStdString();
        return inv;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end()) {
        Models::Investment inv = it->second;
        auto instIt = s_inMemoryInstruments.find(inv.instrumentId);
        if (instIt != s_inMemoryInstruments.end()) {
            inv.symbol = instIt->second.symbol;
            inv.instrumentName = instIt->second.instrumentName;
            if (instIt->second.investmentTypeId == 1) inv.assetTypeCode = "STOCK";
            else if (instIt->second.investmentTypeId == 2) inv.assetTypeCode = "MUTUAL_FUND";
            else if (instIt->second.investmentTypeId == 3) inv.assetTypeCode = "GOLD";
            else if (instIt->second.investmentTypeId == 4) inv.assetTypeCode = "FIXED_DEPOSIT";
            else if (instIt->second.investmentTypeId == 5) inv.assetTypeCode = "BOND";
        }
        return inv;
    }
    return {};
#endif
}

InvestSphere::Optional<Models::Investment> InvestmentRepository::getInvestmentByIdForUser(int investmentId, int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT inv.investment_id, inv.user_id, inv.instrument_id, inv.portfolio_tag, "
        "       inv.notes, inv.status, inv.created_at, inv.updated_at, "
        "       inst.symbol, inst.instrument_name, it.type_code "
        "FROM investments inv "
        "JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE inv.investment_id = :id AND inv.user_id = :userId;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);
    query.bindValue(QStringLiteral(":userId"), userId);

    if (query.exec() && query.next()) {
        Models::Investment inv;
        inv.id = query.value(0).toInt();
        inv.userId = query.value(1).toInt();
        inv.instrumentId = query.value(2).toInt();
        inv.portfolioTag = query.value(3).toString().toStdString();
        inv.notes = query.value(4).toString().toStdString();
        inv.status = query.value(5).toString().toStdString();
        inv.createdAt = query.value(6).toString().toStdString();
        inv.updatedAt = query.value(7).toString().toStdString();
        inv.symbol = query.value(8).toString().toStdString();
        inv.instrumentName = query.value(9).toString().toStdString();
        inv.assetTypeCode = query.value(10).toString().toStdString();
        return inv;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        Models::Investment inv = it->second;
        auto instIt = s_inMemoryInstruments.find(inv.instrumentId);
        if (instIt != s_inMemoryInstruments.end()) {
            inv.symbol = instIt->second.symbol;
            inv.instrumentName = instIt->second.instrumentName;
            if (instIt->second.investmentTypeId == 1) inv.assetTypeCode = "STOCK";
            else if (instIt->second.investmentTypeId == 2) inv.assetTypeCode = "MUTUAL_FUND";
            else if (instIt->second.investmentTypeId == 3) inv.assetTypeCode = "GOLD";
            else if (instIt->second.investmentTypeId == 4) inv.assetTypeCode = "FIXED_DEPOSIT";
            else if (instIt->second.investmentTypeId == 5) inv.assetTypeCode = "BOND";
        }
        return inv;
    }
    return {};
#endif
}

std::vector<Models::Investment> InvestmentRepository::getInvestmentsByUserId(int userId, const std::string &statusFilter)
{
    std::vector<Models::Investment> results;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);

    QString sql = QStringLiteral(
        "SELECT inv.investment_id, inv.user_id, inv.instrument_id, inv.portfolio_tag, "
        "       inv.notes, inv.status, inv.created_at, inv.updated_at, "
        "       inst.symbol, inst.instrument_name, it.type_code, "
        "       COALESCE(ah.remaining_quantity, 0.0), COALESCE(ah.net_invested_amount, 0) "
        "FROM investments inv "
        "JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_active_holdings ah ON inv.investment_id = ah.investment_id "
        "WHERE inv.user_id = :userId "
    );

    if (!statusFilter.empty()) {
        sql += QStringLiteral(" AND inv.status = :status ");
    }
    sql += QStringLiteral(" ORDER BY inv.created_at DESC;");

    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), userId);
    if (!statusFilter.empty()) {
        query.bindValue(QStringLiteral(":status"), QString::fromStdString(statusFilter));
    }

    if (query.exec()) {
        while (query.next()) {
            Models::Investment inv;
            inv.id = query.value(0).toInt();
            inv.userId = query.value(1).toInt();
            inv.instrumentId = query.value(2).toInt();
            inv.portfolioTag = query.value(3).toString().toStdString();
            inv.notes = query.value(4).toString().toStdString();
            inv.status = query.value(5).toString().toStdString();
            inv.createdAt = query.value(6).toString().toStdString();
            inv.updatedAt = query.value(7).toString().toStdString();
            inv.symbol = query.value(8).toString().toStdString();
            inv.instrumentName = query.value(9).toString().toStdString();
            inv.assetTypeCode = query.value(10).toString().toStdString();
            inv.remainingQuantity = query.value(11).toDouble();
            inv.netInvestedAmountCents = query.value(12).toLongLong();
            results.push_back(inv);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    for (const auto &pair : s_inMemoryInvestments) {
        if (pair.second.userId == userId) {
            if (statusFilter.empty() || pair.second.status == statusFilter) {
                Models::Investment inv = pair.second;
                auto instIt = s_inMemoryInstruments.find(inv.instrumentId);
                if (instIt != s_inMemoryInstruments.end()) {
                    inv.symbol = instIt->second.symbol;
                    inv.instrumentName = instIt->second.instrumentName;
                    if (instIt->second.investmentTypeId == 1) inv.assetTypeCode = "STOCK";
                    else if (instIt->second.investmentTypeId == 2) inv.assetTypeCode = "MUTUAL_FUND";
                    else if (instIt->second.investmentTypeId == 3) inv.assetTypeCode = "GOLD";
                    else if (instIt->second.investmentTypeId == 4) inv.assetTypeCode = "FIXED_DEPOSIT";
                    else if (instIt->second.investmentTypeId == 5) inv.assetTypeCode = "BOND";
                }
                results.push_back(inv);
            }
        }
    }
#endif
    return results;
}

bool InvestmentRepository::addGoldDetails(const Models::GoldHoldingDetails &details)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO gold_holdings (investment_id, gold_form, purity_karat, storage_location) "
        "VALUES (:id, :form, :purity, :storage);"
    ));
    query.bindValue(QStringLiteral(":id"), details.investmentId);
    query.bindValue(QStringLiteral(":form"), QString::fromStdString(details.goldForm));
    query.bindValue(QStringLiteral(":purity"), details.purityKarat);
    query.bindValue(QStringLiteral(":storage"), QString::fromStdString(details.storageLocation));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return true;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryGold[details.investmentId] = details;
    return true;
#endif
}

InvestSphere::Optional<Models::GoldHoldingDetails> InvestmentRepository::getGoldDetails(int investmentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT investment_id, gold_form, purity_karat, storage_location "
        "FROM gold_holdings WHERE investment_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);
    if (query.exec() && query.next()) {
        Models::GoldHoldingDetails d;
        d.investmentId = query.value(0).toInt();
        d.goldForm = query.value(1).toString().toStdString();
        d.purityKarat = query.value(2).toDouble();
        d.storageLocation = query.value(3).toString().toStdString();
        return d;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryGold.find(investmentId);
    if (it != s_inMemoryGold.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::addFixedDepositDetails(const Models::FixedDepositDetails &details)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO fixed_deposits (investment_id, institution_name, account_number, principal_amount, "
        "annual_interest_rate, compounding_frequency, payout_type, start_date, maturity_date, "
        "estimated_maturity_amount, status) "
        "VALUES (:id, :inst, :acc, :principal, :rate, :comp, :payout, :start, :maturity, :estMaturity, :status);"
    ));
    query.bindValue(QStringLiteral(":id"), details.investmentId);
    query.bindValue(QStringLiteral(":inst"), QString::fromStdString(details.institutionName));
    query.bindValue(QStringLiteral(":acc"), QString::fromStdString(details.accountNumber));
    query.bindValue(QStringLiteral(":principal"), static_cast<qint64>(details.principalAmountCents));
    query.bindValue(QStringLiteral(":rate"), details.annualInterestRate);
    query.bindValue(QStringLiteral(":comp"), QString::fromStdString(details.compoundingFrequency));
    query.bindValue(QStringLiteral(":payout"), QString::fromStdString(details.payoutType));
    query.bindValue(QStringLiteral(":start"), QString::fromStdString(details.startDate));
    query.bindValue(QStringLiteral(":maturity"), QString::fromStdString(details.maturityDate));
    query.bindValue(QStringLiteral(":estMaturity"), static_cast<qint64>(details.estimatedMaturityAmountCents));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(details.status));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return true;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryFDs[details.investmentId] = details;
    return true;
#endif
}

InvestSphere::Optional<Models::FixedDepositDetails> InvestmentRepository::getFixedDepositDetails(int investmentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT investment_id, institution_name, account_number, principal_amount, "
        "       annual_interest_rate, compounding_frequency, payout_type, start_date, "
        "       maturity_date, estimated_maturity_amount, status "
        "FROM fixed_deposits WHERE investment_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);
    if (query.exec() && query.next()) {
        Models::FixedDepositDetails d;
        d.investmentId = query.value(0).toInt();
        d.institutionName = query.value(1).toString().toStdString();
        d.accountNumber = query.value(2).toString().toStdString();
        d.principalAmountCents = query.value(3).toLongLong();
        d.annualInterestRate = query.value(4).toDouble();
        d.compoundingFrequency = query.value(5).toString().toStdString();
        d.payoutType = query.value(6).toString().toStdString();
        d.startDate = query.value(7).toString().toStdString();
        d.maturityDate = query.value(8).toString().toStdString();
        d.estimatedMaturityAmountCents = query.value(9).toLongLong();
        d.status = query.value(10).toString().toStdString();
        return d;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryFDs.find(investmentId);
    if (it != s_inMemoryFDs.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::addBondDetails(const Models::BondDetails &details)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO bonds (investment_id, issuer_name, bond_type, face_value, "
        "coupon_rate, coupon_frequency, issue_date, maturity_date, credit_rating, status) "
        "VALUES (:id, :issuer, :type, :face, :rate, :freq, :issue, :maturity, :rating, :status);"
    ));
    query.bindValue(QStringLiteral(":id"), details.investmentId);
    query.bindValue(QStringLiteral(":issuer"), QString::fromStdString(details.issuerName));
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(details.bondType));
    query.bindValue(QStringLiteral(":face"), static_cast<qint64>(details.faceValueCents));
    query.bindValue(QStringLiteral(":rate"), details.couponRate);
    query.bindValue(QStringLiteral(":freq"), QString::fromStdString(details.couponFrequency));
    query.bindValue(QStringLiteral(":issue"), QString::fromStdString(details.issueDate));
    query.bindValue(QStringLiteral(":maturity"), QString::fromStdString(details.maturityDate));
    query.bindValue(QStringLiteral(":rating"), QString::fromStdString(details.creditRating));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(details.status));

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return true;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryBonds[details.investmentId] = details;
    return true;
#endif
}

InvestSphere::Optional<Models::BondDetails> InvestmentRepository::getBondDetails(int investmentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT investment_id, issuer_name, bond_type, face_value, "
        "       coupon_rate, coupon_frequency, issue_date, maturity_date, credit_rating, status "
        "FROM bonds WHERE investment_id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), investmentId);
    if (query.exec() && query.next()) {
        Models::BondDetails d;
        d.investmentId = query.value(0).toInt();
        d.issuerName = query.value(1).toString().toStdString();
        d.bondType = query.value(2).toString().toStdString();
        d.faceValueCents = query.value(3).toLongLong();
        d.couponRate = query.value(4).toDouble();
        d.couponFrequency = query.value(5).toString().toStdString();
        d.issueDate = query.value(6).toString().toStdString();
        d.maturityDate = query.value(7).toString().toStdString();
        d.creditRating = query.value(8).toString().toStdString();
        d.status = query.value(9).toString().toStdString();
        return d;
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryBonds.find(investmentId);
    if (it != s_inMemoryBonds.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::updateNotes(int investmentId, const std::string &notes)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE investments SET notes = :notes, updated_at = datetime('now') WHERE investment_id = :id;"));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(notes));
    query.bindValue(QStringLiteral(":id"), investmentId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end()) {
        it->second.notes = notes;
        return true;
    }
    return false;
#endif
}

bool InvestmentRepository::updateNotesForUser(int investmentId, int userId, const std::string &notes)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE investments SET notes = :notes, updated_at = datetime('now') "
        "WHERE investment_id = :id AND user_id = :userId;"
    ));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(notes));
    query.bindValue(QStringLiteral(":id"), investmentId);
    query.bindValue(QStringLiteral(":userId"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        it->second.notes = notes;
        return true;
    }
    return false;
#endif
}

bool InvestmentRepository::updateStatus(int investmentId, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE investments SET status = :status, updated_at = datetime('now') WHERE investment_id = :id;"));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status));
    query.bindValue(QStringLiteral(":id"), investmentId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end()) {
        it->second.status = status;
        return true;
    }
    return false;
#endif
}

bool InvestmentRepository::updateStatusForUser(int investmentId, int userId, const std::string &status)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE investments SET status = :status, updated_at = datetime('now') "
        "WHERE investment_id = :id AND user_id = :userId;"
    ));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status));
    query.bindValue(QStringLiteral(":id"), investmentId);
    query.bindValue(QStringLiteral(":userId"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        it->second.status = status;
        return true;
    }
    return false;
#endif
}

bool InvestmentRepository::updateInvestment(int investmentId, int userId, const std::string &notes, const std::string &status, const std::string &portfolioTag)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE investments SET notes = :notes, status = :status, portfolio_tag = :tag, updated_at = datetime('now') "
        "WHERE investment_id = :id AND user_id = :userId;"
    ));
    query.bindValue(QStringLiteral(":notes"), QString::fromStdString(notes));
    query.bindValue(QStringLiteral(":status"), QString::fromStdString(status.empty() ? "ACTIVE" : status));
    query.bindValue(QStringLiteral(":tag"), QString::fromStdString(portfolioTag.empty() ? "DEFAULT" : portfolioTag));
    query.bindValue(QStringLiteral(":id"), investmentId);
    query.bindValue(QStringLiteral(":userId"), userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text().toStdString();
        return false;
    }
    return query.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        it->second.notes = notes;
        it->second.status = status;
        it->second.portfolioTag = portfolioTag;
        return true;
    }
    return false;
#endif
}

bool InvestmentRepository::deleteInvestment(int investmentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();

    // Check if transactions exist (Policy: ON DELETE RESTRICT on transactions)
    QSqlQuery checkQuery(db);
    checkQuery.prepare(QStringLiteral("SELECT count(*) FROM transactions WHERE investment_id = :id;"));
    checkQuery.bindValue(QStringLiteral(":id"), investmentId);
    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() > 0) {
            m_lastError = "Cannot delete investment: active transaction ledger records exist (Referential Policy RESTRICT).";
            return false;
        }
    }

    QSqlQuery delQuery(db);
    delQuery.prepare(QStringLiteral("DELETE FROM investments WHERE investment_id = :id;"));
    delQuery.bindValue(QStringLiteral(":id"), investmentId);
    if (!delQuery.exec()) {
        m_lastError = delQuery.lastError().text().toStdString();
        return false;
    }
    return delQuery.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    return s_inMemoryInvestments.erase(investmentId) > 0;
#endif
}

bool InvestmentRepository::deleteInvestmentForUser(int investmentId, int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();

    // Enforce ownership check first
    QSqlQuery ownerQuery(db);
    ownerQuery.prepare(QStringLiteral("SELECT count(*) FROM investments WHERE investment_id = :id AND user_id = :userId;"));
    ownerQuery.bindValue(QStringLiteral(":id"), investmentId);
    ownerQuery.bindValue(QStringLiteral(":userId"), userId);
    if (!ownerQuery.exec() || !ownerQuery.next() || ownerQuery.value(0).toInt() == 0) {
        m_lastError = "Permission denied: investment not found or not owned by user.";
        return false;
    }

    // Check if transactions exist
    QSqlQuery checkQuery(db);
    checkQuery.prepare(QStringLiteral("SELECT count(*) FROM transactions WHERE investment_id = :id;"));
    checkQuery.bindValue(QStringLiteral(":id"), investmentId);
    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() > 0) {
            m_lastError = "Cannot delete investment: active transaction ledger records exist (Referential Policy RESTRICT).";
            return false;
        }
    }

    QSqlQuery delQuery(db);
    delQuery.prepare(QStringLiteral("DELETE FROM investments WHERE investment_id = :id AND user_id = :userId;"));
    delQuery.bindValue(QStringLiteral(":id"), investmentId);
    delQuery.bindValue(QStringLiteral(":userId"), userId);
    if (!delQuery.exec()) {
        m_lastError = delQuery.lastError().text().toStdString();
        return false;
    }
    return delQuery.numRowsAffected() > 0;
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        s_inMemoryInvestments.erase(it);
        return true;
    }
    m_lastError = "Permission denied: investment not owned by user.";
    return false;
#endif
}

bool InvestmentRepository::deleteInvestmentWithDependencies(int investmentId, int userId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();

    // Enforce ownership check first
    QSqlQuery ownerQuery(db);
    ownerQuery.prepare(QStringLiteral("SELECT count(*) FROM investments WHERE investment_id = :id AND user_id = :userId;"));
    ownerQuery.bindValue(QStringLiteral(":id"), investmentId);
    ownerQuery.bindValue(QStringLiteral(":userId"), userId);
    if (!ownerQuery.exec() || !ownerQuery.next() || ownerQuery.value(0).toInt() == 0) {
        m_lastError = "Permission denied: investment not found or not owned by user.";
        return false;
    }

    TransactionScope tx;

    // 1. Delete dependent financial events
    QSqlQuery delEvents(db);
    delEvents.prepare(QStringLiteral("DELETE FROM financial_events WHERE investment_id = :id AND user_id = :userId;"));
    delEvents.bindValue(QStringLiteral(":id"), investmentId);
    delEvents.bindValue(QStringLiteral(":userId"), userId);
    delEvents.exec();

    // 2. Delete specialization table records
    QSqlQuery delGold(db);
    delGold.prepare(QStringLiteral("DELETE FROM gold_holdings WHERE investment_id = :id;"));
    delGold.bindValue(QStringLiteral(":id"), investmentId);
    delGold.exec();

    QSqlQuery delFd(db);
    delFd.prepare(QStringLiteral("DELETE FROM fixed_deposits WHERE investment_id = :id;"));
    delFd.bindValue(QStringLiteral(":id"), investmentId);
    delFd.exec();

    QSqlQuery delBonds(db);
    delBonds.prepare(QStringLiteral("DELETE FROM bonds WHERE investment_id = :id;"));
    delBonds.bindValue(QStringLiteral(":id"), investmentId);
    delBonds.exec();

    // 3. Delete dependent transactions
    QSqlQuery delTx(db);
    delTx.prepare(QStringLiteral("DELETE FROM transactions WHERE investment_id = :id AND user_id = :userId;"));
    delTx.bindValue(QStringLiteral(":id"), investmentId);
    delTx.bindValue(QStringLiteral(":userId"), userId);
    if (!delTx.exec()) {
        m_lastError = delTx.lastError().text().toStdString();
        return false;
    }

    // 4. Delete the investment record itself
    QSqlQuery delInv(db);
    delInv.prepare(QStringLiteral("DELETE FROM investments WHERE investment_id = :id AND user_id = :userId;"));
    delInv.bindValue(QStringLiteral(":id"), investmentId);
    delInv.bindValue(QStringLiteral(":userId"), userId);
    if (!delInv.exec()) {
        m_lastError = delInv.lastError().text().toStdString();
        return false;
    }

    return tx.commit();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryInvestments.find(investmentId);
    if (it != s_inMemoryInvestments.end() && it->second.userId == userId) {
        s_inMemoryGold.erase(investmentId);
        s_inMemoryFDs.erase(investmentId);
        s_inMemoryBonds.erase(investmentId);
        s_inMemoryInvestments.erase(it);
        return true;
    }
    m_lastError = "Permission denied: investment not owned by user.";
    return false;
#endif
}

InvestSphere::Optional<int64_t> InvestmentRepository::getLatestMarketPrice(int instrumentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT latest_price FROM vw_latest_market_prices WHERE instrument_id = :id;"));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    if (query.exec() && query.next()) {
        return query.value(0).toLongLong();
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryMarketPrices.find(instrumentId);
    if (it != s_inMemoryMarketPrices.end()) {
        return it->second;
    }
    return {};
#endif
}

InvestSphere::Optional<double> InvestmentRepository::getLatestMutualFundNav(int instrumentId)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT latest_nav FROM vw_latest_mutual_fund_nav WHERE instrument_id = :id;"));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return {};
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryMfNav.find(instrumentId);
    if (it != s_inMemoryMfNav.end()) {
        return it->second;
    }
    return {};
#endif
}

bool InvestmentRepository::setMarketPrice(int instrumentId, int64_t priceCents, const std::string &source)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO market_prices (instrument_id, price, observation_timestamp, source) "
        "VALUES (:id, :price, datetime('now'), :source);"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":price"), static_cast<qint64>(priceCents));
    query.bindValue(QStringLiteral(":source"), QString::fromStdString(source));
    return query.exec();
#else
    (void)source;
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryMarketPrices[instrumentId] = priceCents;
    return true;
#endif
}

bool InvestmentRepository::setMutualFundNav(int instrumentId, double navValue, const std::string &source)
{
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO mutual_fund_nav (instrument_id, nav_value, observation_date, source) "
        "VALUES (:id, :nav, date('now'), :source);"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":nav"), navValue);
    query.bindValue(QStringLiteral(":source"), QString::fromStdString(source));
    return query.exec();
#else
    (void)source;
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryMfNav[instrumentId] = navValue;
    return true;
#endif
}

std::vector<Models::Instrument> InvestmentRepository::getAllInstruments()
{
    std::vector<Models::Instrument> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT instrument_id, investment_type_id, symbol, instrument_name, currency, exchange, description, is_active, created_at "
        "FROM instruments WHERE is_active = 1 ORDER BY symbol ASC;"
    ));
    if (query.exec()) {
        while (query.next()) {
            Models::Instrument inst;
            inst.id = query.value(0).toInt();
            inst.investmentTypeId = query.value(1).toInt();
            inst.symbol = query.value(2).toString().toStdString();
            inst.instrumentName = query.value(3).toString().toStdString();
            inst.currency = query.value(4).toString().toStdString();
            inst.exchange = query.value(5).toString().toStdString();
            inst.description = query.value(6).toString().toStdString();
            inst.isActive = query.value(7).toBool();
            inst.createdAt = query.value(8).toString().toStdString();
            list.push_back(inst);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    for (const auto &pair : s_inMemoryInstruments) {
        if (pair.second.isActive) list.push_back(pair.second);
    }
#endif
    return list;
}

std::vector<Models::Instrument> InvestmentRepository::getInstrumentsByType(const std::string &typeCode)
{
    std::vector<Models::Instrument> list;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT inst.instrument_id, inst.investment_type_id, inst.symbol, inst.instrument_name, "
        "       inst.currency, inst.exchange, inst.description, inst.is_active, inst.created_at "
        "FROM instruments inst "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "WHERE it.type_code = :type AND inst.is_active = 1 "
        "ORDER BY inst.symbol ASC;"
    ));
    query.bindValue(QStringLiteral(":type"), QString::fromStdString(typeCode));
    if (query.exec()) {
        while (query.next()) {
            Models::Instrument inst;
            inst.id = query.value(0).toInt();
            inst.investmentTypeId = query.value(1).toInt();
            inst.symbol = query.value(2).toString().toStdString();
            inst.instrumentName = query.value(3).toString().toStdString();
            inst.currency = query.value(4).toString().toStdString();
            inst.exchange = query.value(5).toString().toStdString();
            inst.description = query.value(6).toString().toStdString();
            inst.isActive = query.value(7).toBool();
            inst.createdAt = query.value(8).toString().toStdString();
            list.push_back(inst);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    for (const auto &pair : s_inMemoryInstruments) {
        if (pair.second.isActive) list.push_back(pair.second);
    }
#endif
    return list;
}

std::vector<InvestmentRepository::MarketPriceRecord> InvestmentRepository::getHistoricalMarketPrices(int instrumentId, int limit)
{
    std::vector<MarketPriceRecord> history;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT price, observation_timestamp, source "
        "FROM market_prices "
        "WHERE instrument_id = :id "
        "ORDER BY observation_timestamp DESC "
        "LIMIT :limit;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":limit"), limit);
    if (query.exec()) {
        while (query.next()) {
            MarketPriceRecord rec;
            rec.priceCents = query.value(0).toLongLong();
            rec.timestamp = query.value(1).toString().toStdString();
            rec.source = query.value(2).toString().toStdString();
            history.push_back(rec);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryHistoricalPrices.find(instrumentId);
    if (it != s_inMemoryHistoricalPrices.end()) {
        history = it->second;
    } else if (s_inMemoryMarketPrices.find(instrumentId) != s_inMemoryMarketPrices.end()) {
        MarketPriceRecord rec;
        rec.priceCents = s_inMemoryMarketPrices[instrumentId];
        rec.timestamp = "2026-10-27 12:00:00";
        rec.source = "MANUAL";
        history.push_back(rec);
    }
#endif
    return history;
}

std::vector<InvestmentRepository::MutualFundNavRecord> InvestmentRepository::getHistoricalMutualFundNavs(int instrumentId, int limit)
{
    std::vector<MutualFundNavRecord> history;
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT nav_value, observation_date, source "
        "FROM mutual_fund_nav "
        "WHERE instrument_id = :id "
        "ORDER BY observation_date DESC "
        "LIMIT :limit;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":limit"), limit);
    if (query.exec()) {
        while (query.next()) {
            MutualFundNavRecord rec;
            rec.navValue = query.value(0).toDouble();
            rec.date = query.value(1).toString().toStdString();
            rec.source = query.value(2).toString().toStdString();
            history.push_back(rec);
        }
    }
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    auto it = s_inMemoryHistoricalNavs.find(instrumentId);
    if (it != s_inMemoryHistoricalNavs.end()) {
        history = it->second;
    } else if (s_inMemoryMfNav.find(instrumentId) != s_inMemoryMfNav.end()) {
        MutualFundNavRecord rec;
        rec.navValue = s_inMemoryMfNav[instrumentId];
        rec.date = "2026-10-27";
        rec.source = "AMFI";
        history.push_back(rec);
    }
#endif
    return history;
}

bool InvestmentRepository::addMarketPrice(int instrumentId, int64_t priceCents, const std::string &timestamp, const std::string &source)
{
    if (priceCents <= 0) {
        m_lastError = "Market price must be positive.";
        return false;
    }
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO market_prices (instrument_id, price, observation_timestamp, source) "
        "VALUES (:id, :price, :ts, :source) "
        "ON CONFLICT(instrument_id, observation_timestamp) DO UPDATE SET price = excluded.price, source = excluded.source;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":price"), static_cast<qint64>(priceCents));
    query.bindValue(QStringLiteral(":ts"), QString::fromStdString(timestamp));
    query.bindValue(QStringLiteral(":source"), QString::fromStdString(source));
    return query.exec();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryMarketPrices[instrumentId] = priceCents;
    MarketPriceRecord rec{priceCents, timestamp, source};
    s_inMemoryHistoricalPrices[instrumentId].insert(s_inMemoryHistoricalPrices[instrumentId].begin(), rec);
    return true;
#endif
}

bool InvestmentRepository::addMutualFundNav(int instrumentId, double navValue, const std::string &date, const std::string &source)
{
    if (navValue <= 0.0) {
        m_lastError = "Mutual fund NAV must be positive.";
        return false;
    }
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO mutual_fund_nav (instrument_id, nav_value, observation_date, source) "
        "VALUES (:id, :nav, :date, :source) "
        "ON CONFLICT(instrument_id, observation_date) DO UPDATE SET nav_value = excluded.nav_value, source = excluded.source;"
    ));
    query.bindValue(QStringLiteral(":id"), instrumentId);
    query.bindValue(QStringLiteral(":nav"), navValue);
    query.bindValue(QStringLiteral(":date"), QString::fromStdString(date));
    query.bindValue(QStringLiteral(":source"), QString::fromStdString(source));
    return query.exec();
#else
    InvestSphere::LockGuard<InvestSphere::Mutex> lock(s_invMutex);
    s_inMemoryMfNav[instrumentId] = navValue;
    MutualFundNavRecord rec{navValue, date, source};
    s_inMemoryHistoricalNavs[instrumentId].insert(s_inMemoryHistoricalNavs[instrumentId].begin(), rec);
    return true;
#endif
}

} // namespace Database
} // namespace InvestSphere

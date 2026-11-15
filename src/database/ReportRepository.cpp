#include "ReportRepository.h"
#include "DatabaseManager.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#endif

namespace InvestSphere {
namespace Database {

namespace {

std::string formatCurrency(double val)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << val;
    return ss.str();
}

} // anonymous namespace

ReportResult ReportRepository::runReport(const std::string &reportId, const ReportFilter &filter)
{
    if (reportId == "PORTFOLIO_SUMMARY") return getPortfolioSummaryReport(filter);
    if (reportId == "CATEGORY_BREAKDOWN") return getCategoryBreakdownReport(filter);
    if (reportId == "TRANSACTION_HISTORY") return getTransactionHistoryReport(filter);
    if (reportId == "MONTHLY_SUMMARY") return getMonthlyTransactionSummaryReport(filter);
    if (reportId == "REALIZED_PNL") return getRealizedProfitLossReport(filter);
    if (reportId == "UNREALIZED_PNL") return getUnrealizedProfitLossReport(filter);
    if (reportId == "DIVIDEND_INTEREST") return getDividendAndInterestReport(filter);
    if (reportId == "ASSET_ALLOCATION") return getAssetAllocationReport(filter);
    if (reportId == "FD_MATURITY") return getFixedDepositMaturityReport(filter);
    if (reportId == "BOND_MATURITY") return getBondMaturityReport(filter);
    if (reportId == "PERFORMANCE") return getInvestmentPerformanceReport(filter);
    if (reportId == "USER_AUDIT") return getUserAuditReport(filter);

    ReportResult res;
    res.reportId = reportId;
    res.success = false;
    res.errorMessage = "Unknown report identifier: " + reportId;
    return res;
}

// 1. Portfolio Summary Report
ReportResult ReportRepository::getPortfolioSummaryReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "PORTFOLIO_SUMMARY";
    res.title = "Consolidated Portfolio Summary Report";
    res.description = "Overview of active holdings, acquisition costs, latest valuation, and unrealized return.";
    res.headers = {"Tag", "Symbol", "Instrument Name", "Category", "Quantity", "Invested (₹/$)", "Market Price", "Est. Value", "Unrealized P&L", "Return %"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    QString sql = QStringLiteral(
        "SELECT "
        "    inv.portfolio_tag, "
        "    inst.symbol, "
        "    inst.instrument_name, "
        "    it.type_name, "
        "    h.remaining_quantity, "
        "    ROUND(h.net_invested_amount / 100.0, 2), "
        "    ROUND(COALESCE(lmp.latest_price, CAST(ROUND(COALESCE(nav.latest_nav, 0) * 100) AS INTEGER), 0) / 100.0, 2), "
        "    ROUND(CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER) / 100.0, 2), "
        "    ROUND(CAST(ROUND((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) AS INTEGER) / 100.0, 2), "
        "    CASE WHEN h.net_invested_amount > 0 "
        "         THEN ROUND((((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) * 100.0) / h.net_invested_amount, 2) "
        "         ELSE 0.0 END "
        "FROM vw_active_holdings h "
        "JOIN investments inv ON h.investment_id = inv.investment_id "
        "JOIN instruments inst ON h.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id "
        "LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id "
        "WHERE h.user_id = :userId AND h.remaining_quantity > 0 "
    );

    if (!filter.categoryCode.empty()) {
        sql += QStringLiteral(" AND it.type_code = :category ");
    }
    sql += QStringLiteral(" ORDER BY h.net_invested_amount DESC;");

    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), filter.userId);
    if (!filter.categoryCode.empty()) {
        query.bindValue(QStringLiteral(":category"), QString::fromStdString(filter.categoryCode));
    }

    double totalInvested = 0.0;
    double totalEstValue = 0.0;
    double totalUnrealizedPnl = 0.0;

    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 10; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalInvested += query.value(5).toDouble();
            totalEstValue += query.value(7).toDouble();
            totalUnrealizedPnl += query.value(8).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Total Positions", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Capital Invested", formatCurrency(totalInvested)});
        res.summaryCards.push_back({"Current Valuation", formatCurrency(totalEstValue)});
        res.summaryCards.push_back({"Unrealized P&L", formatCurrency(totalUnrealizedPnl)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"DEFAULT", "AAPL", "Apple Inc.", "Equity Stocks", "55.0", "9400.00", "235.50", "12952.50", "3552.50", "37.79"});
    res.summaryCards.push_back({"Total Positions", "1"});
    res.summaryCards.push_back({"Capital Invested", "9400.00"});
    res.summaryCards.push_back({"Current Valuation", "12952.50"});
    res.summaryCards.push_back({"Unrealized P&L", "3552.50"});
#endif
    return res;
}

// 2. Investment Category Report
ReportResult ReportRepository::getCategoryBreakdownReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "CATEGORY_BREAKDOWN";
    res.title = "Investment Holdings Grouped by Category";
    res.description = "Portfolio capital and valuation breakdown categorized by underlying asset classes.";
    res.headers = {"Asset Category", "Holdings Count", "Capital Invested (₹/$)", "Estimated Value (₹/$)", "Total Unrealized P&L"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    it.type_name, "
        "    COUNT(DISTINCT inv.investment_id), "
        "    ROUND(SUM(h.net_invested_amount) / 100.0, 2), "
        "    ROUND(SUM(CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER)) / 100.0, 2), "
        "    ROUND(SUM(CAST(ROUND((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) AS INTEGER)) / 100.0, 2) "
        "FROM vw_active_holdings h "
        "JOIN investments inv ON h.investment_id = inv.investment_id "
        "JOIN instruments inst ON h.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id "
        "LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id "
        "WHERE h.user_id = :userId AND h.remaining_quantity > 0 "
        "GROUP BY it.investment_type_id, it.type_name "
        "ORDER BY SUM(h.net_invested_amount) DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalInvested = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 5; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalInvested += query.value(2).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Categories", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Total Capital", formatCurrency(totalInvested)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"Equity Stocks", "2", "19915.00", "25792.50", "5877.50"});
    res.summaryCards.push_back({"Categories", "1"});
#endif
    return res;
}

// 3. Transaction History Report
ReportResult ReportRepository::getTransactionHistoryReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "TRANSACTION_HISTORY";
    res.title = "Transaction History Audit Report";
    res.description = "Filtered chronological ledger of all trade orders, deposits, and income cash flows.";
    res.headers = {"Tx ID", "Date & Time", "Type", "Symbol", "Security Name", "Quantity", "Price", "Gross", "Fees & Tax", "Net Amount", "Notes"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    QString sql = QStringLiteral(
        "SELECT "
        "    t.transaction_id, "
        "    t.transaction_date, "
        "    t.transaction_type, "
        "    COALESCE(inst.symbol, 'N/A'), "
        "    COALESCE(inst.instrument_name, 'Cash Balance'), "
        "    COALESCE(t.quantity, 0.0), "
        "    ROUND(COALESCE(t.unit_price, 0) / 100.0, 2), "
        "    ROUND(t.gross_amount / 100.0, 2), "
        "    ROUND((t.fees + t.taxes) / 100.0, 2), "
        "    ROUND(t.net_amount / 100.0, 2), "
        "    COALESCE(t.notes, '') "
        "FROM transactions t "
        "LEFT JOIN investments inv ON t.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE t.user_id = :userId "
    );

    if (!filter.startDate.empty()) {
        sql += QStringLiteral(" AND date(t.transaction_date) >= :startDate ");
    }
    if (!filter.endDate.empty()) {
        sql += QStringLiteral(" AND date(t.transaction_date) <= :endDate ");
    }
    sql += QStringLiteral(" ORDER BY t.transaction_date DESC LIMIT :limit;");

    query.prepare(sql);
    query.bindValue(QStringLiteral(":userId"), filter.userId);
    if (!filter.startDate.empty()) query.bindValue(QStringLiteral(":startDate"), QString::fromStdString(filter.startDate));
    if (!filter.endDate.empty()) query.bindValue(QStringLiteral(":endDate"), QString::fromStdString(filter.endDate));
    query.bindValue(QStringLiteral(":limit"), filter.limit > 0 ? filter.limit : 200);

    double totalNet = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 11; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalNet += query.value(9).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Total Records", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Net Flow Sum", formatCurrency(totalNet)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"1", "2026-02-10 10:15:00", "BUY", "AAPL", "Apple Inc.", "50.0", "180.00", "9000.00", "10.00", "9010.00", "Initial purchase"});
    res.summaryCards.push_back({"Total Records", "1"});
#endif
    return res;
}

// 4. Monthly Transaction Summary Report
ReportResult ReportRepository::getMonthlyTransactionSummaryReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "MONTHLY_SUMMARY";
    res.title = "Monthly Transaction Cadence Summary";
    res.description = "Aggregated monthly cash outflow and inflow across buys, sells, and dividends.";
    res.headers = {"Month", "Total Trades", "Purchases (₹/$)", "Sales (₹/$)", "Income Received", "Fees Incurred"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    strftime('%Y-%m', t.transaction_date) AS month, "
        "    COUNT(t.transaction_id), "
        "    ROUND(SUM(CASE WHEN t.transaction_type = 'BUY' THEN t.net_amount ELSE 0 END) / 100.0, 2), "
        "    ROUND(SUM(CASE WHEN t.transaction_type IN ('SELL', 'REDEMPTION') THEN t.net_amount ELSE 0 END) / 100.0, 2), "
        "    ROUND(SUM(CASE WHEN t.transaction_type IN ('DIVIDEND', 'INTEREST') THEN t.net_amount ELSE 0 END) / 100.0, 2), "
        "    ROUND(SUM(t.fees + t.taxes) / 100.0, 2) "
        "FROM transactions t "
        "WHERE t.user_id = :userId "
        "GROUP BY strftime('%Y-%m', t.transaction_date) "
        "ORDER BY month DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 6; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Months Tracked", std::to_string(res.rows.size())});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"2026-02", "2", "12785.00", "0.00", "0.00", "35.00"});
    res.summaryCards.push_back({"Months Tracked", "1"});
#endif
    return res;
}

// 5. Realized Profit/Loss Report
ReportResult ReportRepository::getRealizedProfitLossReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "REALIZED_PNL";
    res.title = "Realized Capital Gains & Exit Trades";
    res.description = "Itemized analysis of liquidated and redeemed positions with trade friction costs.";
    res.headers = {"Tx ID", "Exit Date", "Symbol", "Security Name", "Qty Sold", "Sale Price", "Gross Proceeds", "Fees & Taxes", "Net Proceeds", "Notes"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    t.transaction_id, "
        "    t.transaction_date, "
        "    inst.symbol, "
        "    inst.instrument_name, "
        "    t.quantity, "
        "    ROUND(t.unit_price / 100.0, 2), "
        "    ROUND(t.gross_amount / 100.0, 2), "
        "    ROUND((t.fees + t.taxes) / 100.0, 2), "
        "    ROUND(t.net_amount / 100.0, 2), "
        "    COALESCE(t.notes, '') "
        "FROM transactions t "
        "JOIN investments inv ON t.investment_id = inv.investment_id "
        "JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE t.user_id = :userId AND t.transaction_type IN ('SELL', 'REDEMPTION') "
        "ORDER BY t.transaction_date DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalGross = 0.0;
    double totalNet = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 10; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalGross += query.value(6).toDouble();
            totalNet += query.value(8).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Exits Count", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Gross Realized", formatCurrency(totalGross)});
        res.summaryCards.push_back({"Net Proceeds", formatCurrency(totalNet)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"3", "2026-08-20 11:45:00", "AAPL", "Apple Inc.", "20.0", "220.00", "4400.00", "30.00", "4370.00", "Profit booking"});
    res.summaryCards.push_back({"Exits Count", "1"});
    res.summaryCards.push_back({"Gross Realized", "4400.00"});
#endif
    return res;
}

// 6. Unrealized Profit/Loss Report
ReportResult ReportRepository::getUnrealizedProfitLossReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "UNREALIZED_PNL";
    res.title = "Unrealized Capital Gains & Valuation Report";
    res.description = "Valuation comparison of remaining inventory against documented weighted average cost basis.";
    res.headers = {"Symbol", "Security Name", "Category", "Quantity", "Avg Cost (₹/$)", "Market Quote", "Total Cost Basis", "Est. Market Value", "Unrealized P&L", "Gain %"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    inst.symbol, "
        "    inst.instrument_name, "
        "    it.type_name, "
        "    h.remaining_quantity, "
        "    ROUND(CASE WHEN h.remaining_quantity > 0 THEN (h.net_invested_amount / h.remaining_quantity) / 100.0 ELSE 0 END, 2), "
        "    ROUND(COALESCE(lmp.latest_price, CAST(ROUND(COALESCE(nav.latest_nav, 0) * 100) AS INTEGER), 0) / 100.0, 2), "
        "    ROUND(h.net_invested_amount / 100.0, 2), "
        "    ROUND(CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER) / 100.0, 2), "
        "    ROUND(CAST(ROUND((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) AS INTEGER) / 100.0, 2), "
        "    CASE WHEN h.net_invested_amount > 0 "
        "         THEN ROUND((((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) * 100.0) / h.net_invested_amount, 2) "
        "         ELSE 0.0 END "
        "FROM vw_active_holdings h "
        "JOIN instruments inst ON h.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id "
        "LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id "
        "WHERE h.user_id = :userId AND h.remaining_quantity > 0 "
        "ORDER BY (h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalCost = 0.0;
    double totalVal = 0.0;
    double totalPnl = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 10; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalCost += query.value(6).toDouble();
            totalVal += query.value(7).toDouble();
            totalPnl += query.value(8).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Open Holdings", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Total Cost Basis", formatCurrency(totalCost)});
        res.summaryCards.push_back({"Portfolio Value", formatCurrency(totalVal)});
        res.summaryCards.push_back({"Net Unrealized P&L", formatCurrency(totalPnl)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"AAPL", "Apple Inc.", "Equity Stocks", "55.0", "170.91", "235.50", "9400.00", "12952.50", "3552.50", "37.79"});
    res.summaryCards.push_back({"Open Holdings", "1"});
    res.summaryCards.push_back({"Total Cost Basis", "9400.00"});
#endif
    return res;
}

// 7. Dividend and Interest Income Report
ReportResult ReportRepository::getDividendAndInterestReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "DIVIDEND_INTEREST";
    res.title = "Investment Income & Cash Dividends";
    res.description = "Chronological log of all credited dividends, bank FD interest, and bond coupon distributions.";
    res.headers = {"Tx ID", "Date", "Income Type", "Symbol", "Paying Instrument", "Amount Credited (₹/$)", "TDS / Tax Withheld", "Remarks"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    t.transaction_id, "
        "    t.transaction_date, "
        "    t.transaction_type, "
        "    COALESCE(inst.symbol, 'N/A'), "
        "    COALESCE(inst.instrument_name, 'Direct Deposit'), "
        "    ROUND(t.net_amount / 100.0, 2), "
        "    ROUND(t.taxes / 100.0, 2), "
        "    COALESCE(t.notes, '') "
        "FROM transactions t "
        "LEFT JOIN investments inv ON t.investment_id = inv.investment_id "
        "LEFT JOIN instruments inst ON inv.instrument_id = inst.instrument_id "
        "WHERE t.user_id = :userId AND t.transaction_type IN ('DIVIDEND', 'INTEREST') "
        "ORDER BY t.transaction_date DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalIncome = 0.0;
    double totalTds = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 8; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalIncome += query.value(5).toDouble();
            totalTds += query.value(6).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Distributions", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Total Income Credited", formatCurrency(totalIncome)});
        res.summaryCards.push_back({"Total Tax Deducted", formatCurrency(totalTds)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"9", "2026-05-15 12:00:00", "DIVIDEND", "AAPL", "Apple Inc.", "125.00", "0.00", "Quarterly dividend"});
    res.summaryCards.push_back({"Distributions", "1"});
    res.summaryCards.push_back({"Total Income Credited", "125.00"});
#endif
    return res;
}

// 8. Asset Allocation Percentage Report
ReportResult ReportRepository::getAssetAllocationReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "ASSET_ALLOCATION";
    res.title = "Portfolio Asset Class Allocation Breakdown";
    res.description = "Proportional capital weight and current market value share across asset classes.";
    res.headers = {"Asset Class", "Holdings Count", "Capital Invested (₹/$)", "Market Value (₹/$)", "Capital Share %"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    it.type_name, "
        "    COUNT(DISTINCT inv.investment_id), "
        "    ROUND(SUM(h.net_invested_amount) / 100.0, 2), "
        "    ROUND(SUM(CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER)) / 100.0, 2), "
        "    ROUND((SUM(h.net_invested_amount) * 100.0) / NULLIF((SELECT SUM(net_invested_amount) FROM vw_active_holdings WHERE user_id = :userId AND remaining_quantity > 0), 0), 2) "
        "FROM vw_active_holdings h "
        "JOIN investments inv ON h.investment_id = inv.investment_id "
        "JOIN instruments inst ON h.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id "
        "LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id "
        "WHERE h.user_id = :userId AND h.remaining_quantity > 0 "
        "GROUP BY it.investment_type_id, it.type_name "
        "ORDER BY SUM(h.net_invested_amount) DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 5; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Asset Classes", std::to_string(res.rows.size())});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"Equity Stocks", "2", "19915.00", "25792.50", "38.50"});
    res.summaryCards.push_back({"Asset Classes", "1"});
#endif
    return res;
}

// 9. Fixed Deposit Maturity Report
ReportResult ReportRepository::getFixedDepositMaturityReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "FD_MATURITY";
    res.title = "Fixed Deposit Redemption & Maturity Schedule";
    res.description = "Audit report of active and matured term deposits with compounding interest and tenure.";
    res.headers = {"Bank / Institution", "Account Number", "Principal (₹/$)", "Interest Rate", "Compounding", "Payout Type", "Start Date", "Maturity Date", "Days Left", "Est. Maturity Proceeds", "Status"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    fd.institution_name, "
        "    COALESCE(fd.account_number, 'N/A'), "
        "    ROUND(fd.principal_amount / 100.0, 2), "
        "    fd.annual_interest_rate || '%', "
        "    fd.compounding_frequency, "
        "    fd.payout_type, "
        "    fd.start_date, "
        "    fd.maturity_date, "
        "    CAST((julianday(fd.maturity_date) - julianday(date('now'))) AS INTEGER), "
        "    ROUND(fd.estimated_maturity_amount / 100.0, 2), "
        "    fd.status "
        "FROM fixed_deposits fd "
        "JOIN investments inv ON fd.investment_id = inv.investment_id "
        "WHERE inv.user_id = :userId "
        "ORDER BY fd.maturity_date ASC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalPrincipal = 0.0;
    double totalProceeds = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 11; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalPrincipal += query.value(2).toDouble();
            totalProceeds += query.value(9).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Total FDs", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Principal Deployed", formatCurrency(totalPrincipal)});
        res.summaryCards.push_back({"Maturity Proceeds", formatCurrency(totalProceeds)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"HDFC Bank Ltd.", "FD-984210456", "10000.00", "7.10%", "QUARTERLY", "CUMULATIVE", "2026-01-15", "2027-01-15", "80", "10729.00", "ACTIVE"});
    res.summaryCards.push_back({"Total FDs", "1"});
    res.summaryCards.push_back({"Principal Deployed", "10000.00"});
#endif
    return res;
}

// 10. Bond Maturity Report
ReportResult ReportRepository::getBondMaturityReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "BOND_MATURITY";
    res.title = "Bonds & Government Securities Maturity Report";
    res.description = "Comprehensive schedule of debt instruments, face values, and semi-annual coupon yields.";
    res.headers = {"Issuer Name", "Bond Category", "Unit Face Value", "Quantity", "Total Par Value", "Coupon Rate", "Coupon Frequency", "Issue Date", "Maturity Date", "Days Left", "Credit Rating", "Status"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    b.issuer_name, "
        "    b.bond_type, "
        "    ROUND(b.face_value / 100.0, 2), "
        "    COALESCE(h.remaining_quantity, 1.0), "
        "    ROUND((b.face_value * COALESCE(h.remaining_quantity, 1.0)) / 100.0, 2), "
        "    b.coupon_rate || '%', "
        "    b.coupon_frequency, "
        "    b.issue_date, "
        "    b.maturity_date, "
        "    CAST((julianday(b.maturity_date) - julianday(date('now'))) AS INTEGER), "
        "    COALESCE(b.credit_rating, 'NR'), "
        "    b.status "
        "FROM bonds b "
        "JOIN investments inv ON b.investment_id = inv.investment_id "
        "LEFT JOIN vw_active_holdings h ON inv.investment_id = h.investment_id "
        "WHERE inv.user_id = :userId "
        "ORDER BY b.maturity_date ASC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    double totalPar = 0.0;
    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 12; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            totalPar += query.value(4).toDouble();
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Bonds Held", std::to_string(res.rows.size())});
        res.summaryCards.push_back({"Total Face Value", formatCurrency(totalPar)});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"US Treasury", "GOVERNMENT", "1000.00", "10.0", "10000.00", "4.25%", "SEMI_ANNUALLY", "2024-05-15", "2034-05-15", "2750", "AAA", "ACTIVE"});
    res.summaryCards.push_back({"Bonds Held", "1"});
    res.summaryCards.push_back({"Total Face Value", "10000.00"});
#endif
    return res;
}

// 11. Investment Performance Report
ReportResult ReportRepository::getInvestmentPerformanceReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "PERFORMANCE";
    res.title = "Total Return Performance & Yield Comparison";
    res.description = "Calculates total investment return combining unrealized capital appreciation and income received.";
    res.headers = {"Symbol", "Security Name", "Category", "Quantity", "Invested (₹/$)", "Market Value", "Income Paid", "Capital Gains", "Total Return %", "Quote Date"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    inst.symbol, "
        "    inst.instrument_name, "
        "    it.type_name, "
        "    h.remaining_quantity, "
        "    ROUND(h.net_invested_amount / 100.0, 2), "
        "    ROUND(CAST(ROUND(h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) AS INTEGER) / 100.0, 2), "
        "    ROUND(COALESCE((SELECT SUM(net_amount) FROM transactions WHERE investment_id = h.investment_id AND transaction_type IN ('DIVIDEND', 'INTEREST')), 0) / 100.0, 2), "
        "    ROUND(CAST(ROUND((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount) AS INTEGER) / 100.0, 2), "
        "    CASE WHEN h.net_invested_amount > 0 "
        "         THEN ROUND((((h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) - h.net_invested_amount + COALESCE((SELECT SUM(net_amount) FROM transactions WHERE investment_id = h.investment_id AND transaction_type IN ('DIVIDEND', 'INTEREST')), 0)) * 100.0) / h.net_invested_amount, 2) "
        "         ELSE 0.0 END, "
        "    COALESCE(lmp.latest_timestamp, nav.latest_nav_date, 'NO_QUOTE') "
        "FROM vw_active_holdings h "
        "JOIN instruments inst ON h.instrument_id = inst.instrument_id "
        "JOIN investment_types it ON inst.investment_type_id = it.investment_type_id "
        "LEFT JOIN vw_latest_market_prices lmp ON h.instrument_id = lmp.instrument_id "
        "LEFT JOIN vw_latest_mutual_fund_nav nav ON h.instrument_id = nav.instrument_id "
        "WHERE h.user_id = :userId AND h.remaining_quantity > 0 "
        "ORDER BY (h.remaining_quantity * COALESCE(lmp.latest_price, ROUND(COALESCE(nav.latest_nav, 0) * 100), 0)) DESC;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);

    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 10; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Positions Analyzed", std::to_string(res.rows.size())});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"AAPL", "Apple Inc.", "Equity Stocks", "55.0", "9400.00", "12952.50", "125.00", "3552.50", "39.12", "2026-10-24 16:00:00"});
    res.summaryCards.push_back({"Positions Analyzed", "1"});
#endif
    return res;
}

// 12. User Activity / Audit Log Report
ReportResult ReportRepository::getUserAuditReport(const ReportFilter &filter)
{
    ReportResult res;
    res.reportId = "USER_AUDIT";
    res.title = "User Audit Trail & Security Ledger";
    res.description = "Immutable log of financial modifications, logins, updates, and database actions.";
    res.headers = {"Audit ID", "Timestamp", "Action Type", "Entity", "Entity ID", "Description"};

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = DatabaseManager::instance().getQtDatabase();
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT "
        "    al.audit_id, "
        "    al.action_timestamp, "
        "    al.action_type, "
        "    al.entity_type, "
        "    COALESCE(al.entity_id, 0), "
        "    al.description "
        "FROM audit_logs al "
        "WHERE al.user_id = :userId "
        "ORDER BY al.action_timestamp DESC "
        "LIMIT :limit;"
    ));
    query.bindValue(QStringLiteral(":userId"), filter.userId);
    query.bindValue(QStringLiteral(":limit"), filter.limit > 0 ? filter.limit : 100);

    if (query.exec()) {
        while (query.next()) {
            std::vector<std::string> row;
            for (int i = 0; i < 6; ++i) {
                row.push_back(query.value(i).toString().toStdString());
            }
            res.rows.push_back(row);
        }
        res.summaryCards.push_back({"Audit Logs Recorded", std::to_string(res.rows.size())});
    } else {
        res.success = false;
        res.errorMessage = query.lastError().text().toStdString();
    }
#else
    res.rows.push_back({"101", "2026-10-27 10:00:00", "LOGIN", "USER", "1", "User session authenticated"});
    res.summaryCards.push_back({"Audit Logs Recorded", "1"});
#endif
    return res;
}

} // namespace Database
} // namespace InvestSphere

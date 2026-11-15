#include "ReportService.h"

namespace InvestSphere {
namespace Services {

ReportService& ReportService::instance()
{
    static ReportService s_instance;
    return s_instance;
}

std::vector<ReportDescriptor> ReportService::getAvailableReports() const
{
    return {
        {
            "PORTFOLIO_SUMMARY",
            "Portfolio Summary Report",
            "Portfolio",
            "Consolidated overview of invested capital, market value, and unrealized profit/loss."
        },
        {
            "CATEGORY_BREAKDOWN",
            "Investment Category Report",
            "Holdings",
            "Active holdings grouped by investment category with cost basis and market value."
        },
        {
            "TRANSACTION_HISTORY",
            "Transaction History Report",
            "Activity",
            "Complete activity log of orders, purchases, sales, and capital deposits."
        },
        {
            "MONTHLY_SUMMARY",
            "Monthly Transaction Summary",
            "Activity",
            "Monthly investment cadence and capital outflow grouped by calendar month."
        },
        {
            "REALIZED_PNL",
            "Realized Profit/Loss Report",
            "Tax & P&L",
            "Realized capital gains and losses on closed trades with trade friction costs."
        },
        {
            "UNREALIZED_PNL",
            "Unrealized Profit/Loss Report",
            "Tax & P&L",
            "Estimated unrealized gains and losses on open inventory using latest market quotes."
        },
        {
            "DIVIDEND_INTEREST",
            "Dividend and Interest Income",
            "Income",
            "Itemized schedule of credited cash dividends, bank FD interest, and bond coupons."
        },
        {
            "ASSET_ALLOCATION",
            "Asset Allocation Report",
            "Portfolio",
            "Diversification weights and proportional share of portfolio by asset class."
        },
        {
            "FD_MATURITY",
            "Fixed Deposit Maturity Report",
            "Fixed Income",
            "Upcoming and matured bank fixed deposits with compounding interest and proceeds."
        },
        {
            "BOND_MATURITY",
            "Bond Maturity Report",
            "Fixed Income",
            "Government securities and corporate bonds with face values and coupon yields."
        },
        {
            "PERFORMANCE",
            "Investment Performance Report",
            "Analytics",
            "Total return comparison across instruments combining capital gain and income."
        },
        {
            "USER_AUDIT",
            "User Activity & Audit Trail",
            "Security",
            "Immutable audit ledger recording user logins, exports, updates, and modifications."
        }
    };
}

Database::ReportResult ReportService::generateReport(
    int userId,
    const std::string &reportId,
    const std::string &startDate,
    const std::string &endDate,
    const std::string &categoryCode,
    const std::string &searchQuery,
    int limit)
{
    Database::ReportFilter filter;
    filter.userId = userId;
    filter.startDate = startDate;
    filter.endDate = endDate;
    filter.categoryCode = categoryCode;
    filter.searchTerm = searchQuery;
    filter.limit = limit;

    return m_repo.runReport(reportId, filter);
}

bool ReportService::exportReportToCsv(
    const std::string &filePath,
    const Database::ReportResult &report,
    std::string &errorMessage)
{
    return CsvExportService::exportToFile(filePath, report, errorMessage);
}

} // namespace Services
} // namespace InvestSphere

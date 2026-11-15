#ifndef INVESTSPHERE_REPORTREPOSITORY_H
#define INVESTSPHERE_REPORTREPOSITORY_H

#include "Compat.h"
#include <string>
#include <vector>
#include <utility>

namespace InvestSphere {
namespace Database {

struct ReportFilter {
    int userId{0};
    std::string categoryCode;
    std::string startDate;
    std::string endDate;
    std::string searchTerm;
    int limit{200};
};

struct ReportResult {
    std::string reportId;
    std::string title;
    std::string description;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::pair<std::string, std::string>> summaryCards;
    bool success{true};
    std::string errorMessage;
};

class ReportRepository {
public:
    ReportRepository() = default;

    ReportResult getPortfolioSummaryReport(const ReportFilter &filter);
    ReportResult getCategoryBreakdownReport(const ReportFilter &filter);
    ReportResult getTransactionHistoryReport(const ReportFilter &filter);
    ReportResult getMonthlyTransactionSummaryReport(const ReportFilter &filter);
    ReportResult getRealizedProfitLossReport(const ReportFilter &filter);
    ReportResult getUnrealizedProfitLossReport(const ReportFilter &filter);
    ReportResult getDividendAndInterestReport(const ReportFilter &filter);
    ReportResult getAssetAllocationReport(const ReportFilter &filter);
    ReportResult getFixedDepositMaturityReport(const ReportFilter &filter);
    ReportResult getBondMaturityReport(const ReportFilter &filter);
    ReportResult getInvestmentPerformanceReport(const ReportFilter &filter);
    ReportResult getUserAuditReport(const ReportFilter &filter);

    ReportResult runReport(const std::string &reportId, const ReportFilter &filter);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // INVESTSPHERE_REPORTREPOSITORY_H

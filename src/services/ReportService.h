#ifndef INVESTSPHERE_REPORTSERVICE_H
#define INVESTSPHERE_REPORTSERVICE_H

#include "ReportRepository.h"
#include "CsvExportService.h"
#include <string>
#include <vector>
#include <utility>

namespace InvestSphere {
namespace Services {

struct ReportDescriptor {
    std::string id;
    std::string title;
    std::string category;
    std::string description;
};

class ReportService {
public:
    static ReportService& instance();

    std::vector<ReportDescriptor> getAvailableReports() const;

    Database::ReportResult generateReport(
        int userId,
        const std::string &reportId,
        const std::string &startDate = "",
        const std::string &endDate = "",
        const std::string &categoryCode = "",
        const std::string &searchQuery = "",
        int limit = 200
    );

    bool exportReportToCsv(
        const std::string &filePath,
        const Database::ReportResult &report,
        std::string &errorMessage
    );

private:
    ReportService() = default;
    ReportService(const ReportService&) = delete;
    ReportService& operator=(const ReportService&) = delete;

    Database::ReportRepository m_repo;
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_REPORTSERVICE_H

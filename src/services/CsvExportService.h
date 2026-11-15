#ifndef INVESTSPHERE_CSVEXPORTSERVICE_H
#define INVESTSPHERE_CSVEXPORTSERVICE_H

#include "ReportRepository.h"
#include <string>
#include <vector>

namespace InvestSphere {
namespace Services {

class CsvExportService {
public:
    static std::string escapeCsvField(const std::string &field);
    static std::string generateCsv(const Database::ReportResult &report);
    static bool exportToFile(const std::string &filePath, const Database::ReportResult &report, std::string &errorMessage);
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_CSVEXPORTSERVICE_H

#include "CsvExportService.h"
#include <fstream>
#include <sstream>

namespace InvestSphere {
namespace Services {

std::string CsvExportService::escapeCsvField(const std::string &field)
{
    bool needsQuotes = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuotes = true;
            break;
        }
    }

    if (!needsQuotes) {
        return field;
    }

    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\""; // Double quotes escaping per RFC 4180
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

std::string CsvExportService::generateCsv(const Database::ReportResult &report)
{
    std::ostringstream ss;

    // Report Header Row
    for (size_t i = 0; i < report.headers.size(); ++i) {
        ss << escapeCsvField(report.headers[i]);
        if (i + 1 < report.headers.size()) {
            ss << ",";
        }
    }
    ss << "\r\n";

    // Data Rows
    if (report.rows.empty()) {
        ss << "# No records found matching the active filters.\r\n";
    } else {
        for (const auto &row : report.rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                ss << escapeCsvField(row[i]);
                if (i + 1 < row.size()) {
                    ss << ",";
                }
            }
            ss << "\r\n";
        }
    }

    return ss.str();
}

bool CsvExportService::exportToFile(const std::string &filePath,
                                    const Database::ReportResult &report,
                                    std::string &errorMessage)
{
    if (filePath.empty()) {
        errorMessage = "Destination file path is empty.";
        return false;
    }

    std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        errorMessage = "Failed to create or open file at destination: " + filePath;
        return false;
    }

    std::string csvContent = generateCsv(report);
    outFile << csvContent;

    if (outFile.fail()) {
        errorMessage = "I/O error encountered while writing CSV content to: " + filePath;
        outFile.close();
        return false;
    }

    outFile.close();
    return true;
}

} // namespace Services
} // namespace InvestSphere

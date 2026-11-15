#ifndef INVESTSPHERE_REPORTSPAGE_H
#define INVESTSPHERE_REPORTSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QVector>
#include <QPair>
#include <QString>
#include "SummaryCard.h"
#include "ModernTableWidget.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Financial Reports page wired to ReportService.
 * Shows STCG/LTCG/income summary cards, 12 report template cards,
 * CSV/PDF export via QFileDialog, and a generated-reports archive table.
 */
class ReportsPage : public QWidget {
    Q_OBJECT

public:
    explicit ReportsPage(QWidget *parent = nullptr);

public slots:
    void loadSummaryData();

private:
    void setupUi();
    QWidget* createReportTemplatesGrid();
    void generateAndExportReport(const std::string &reportId, const QString &reportTitle);
    void addArchiveRow(const QString &title, const QString &type,
                       const QString &period, const QString &format, const QString &status);

    SummaryCard       *m_stcgCard{nullptr};
    SummaryCard       *m_ltcgCard{nullptr};
    SummaryCard       *m_incomeCard{nullptr};
    ModernTableWidget *m_generatedReportsTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_REPORTSPAGE_H

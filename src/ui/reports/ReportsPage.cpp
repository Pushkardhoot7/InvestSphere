#include "ReportsPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <string>

#include "Theme.h"
#include "IconHelper.h"
#include "SectionHeader.h"
#include "SessionManager.h"
#include "ReportService.h"
#include "CsvExportService.h"

namespace InvestSphere {
namespace Ui {

ReportsPage::ReportsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadSummaryData();
}

void ReportsPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(24);

    // ── Summary Cards ────────────────────────────────────────────────────────
    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);

    m_stcgCard = new SummaryCard(QStringLiteral("Short-Term Capital Gains (STCG)"),
                                  QStringLiteral("₹0.00"), QStringLiteral("FY 2025-26"),
                                  SummaryCard::TrendType::Neutral, IconHelper::IconType::Reports, container);
    m_stcgCard->setSubtitle(QStringLiteral("Taxable gains on holdings < 1 yr"));
    summaryRow->addWidget(m_stcgCard);

    m_ltcgCard = new SummaryCard(QStringLiteral("Long-Term Capital Gains (LTCG)"),
                                  QStringLiteral("₹0.00"), QStringLiteral("FY 2025-26"),
                                  SummaryCard::TrendType::Neutral, IconHelper::IconType::Reports, container);
    m_ltcgCard->setSubtitle(QStringLiteral("Subject to ₹1.25L exemption threshold"));
    summaryRow->addWidget(m_ltcgCard);

    m_incomeCard = new SummaryCard(QStringLiteral("Annual Dividend & Interest"),
                                    QStringLiteral("₹0.00"), QStringLiteral("Income Head"),
                                    SummaryCard::TrendType::Neutral, IconHelper::IconType::TrendUp, container);
    m_incomeCard->setSubtitle(QStringLiteral("Taxable under other sources"));
    summaryRow->addWidget(m_incomeCard);

    layout->addLayout(summaryRow);

    // ── Section 1: Report Templates ──────────────────────────────────────────
    auto *templatesHeader = new SectionHeader(
        QStringLiteral("Standard Financial & Tax Statements"),
        QStringLiteral("Generate and export instant CSV statements for tax filings and audit."),
        QString(), container);
    layout->addWidget(templatesHeader);
    layout->addWidget(createReportTemplatesGrid());

    // ── Section 2: Generated Reports Archive ─────────────────────────────────
    auto *historyHeader = new SectionHeader(
        QStringLiteral("Generated Statements & Exports Archive"),
        QStringLiteral("Archived exports and download history for this session."),
        QString(), container);
    layout->addWidget(historyHeader);

    m_generatedReportsTable = new ModernTableWidget(container);
    m_generatedReportsTable->setHeaders({
        QStringLiteral("Report Title"),
        QStringLiteral("Category"),
        QStringLiteral("Financial Period"),
        QStringLiteral("Format"),
        QStringLiteral("Generated At"),
        QStringLiteral("Status")
    });
    m_generatedReportsTable->setEmptyMessage(
        QStringLiteral("No reports generated yet. Click 'Generate Statement' above to create an export."));
    m_generatedReportsTable->setMinimumHeight(240);
    layout->addWidget(m_generatedReportsTable);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

// ─── Template Grid ──────────────────────────────────────────────────────────

QWidget* ReportsPage::createReportTemplatesGrid()
{
    auto *gridWidget = new QWidget(this);
    auto *grid = new QGridLayout(gridWidget);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(16);

    // Each tuple: {reportId, title, desc, formatBadge, row, col}
    struct TemplateSpec {
        const char *reportId;
        QString     title;
        QString     desc;
        QString     badge;
        int row, col;
    };

    const QVector<TemplateSpec> specs = {
        {"REALIZED_PNL",
         QStringLiteral("Capital Gains Tax Statement"),
         QStringLiteral("Comprehensive breakdown of short-term and long-term gains with indexed acquisition costs for ITR filing."),
         QStringLiteral("PDF + CSV"), 0, 0},

        {"PORTFOLIO_SUMMARY",
         QStringLiteral("Consolidated Holdings Statement"),
         QStringLiteral("Full portfolio snapshot across stocks, mutual funds, gold, fixed deposits, and bonds with current market value."),
         QStringLiteral("CSV"), 0, 1},

        {"DIVIDEND_INTEREST",
         QStringLiteral("Dividend & Interest Income Report"),
         QStringLiteral("Itemized record of all cash dividend credits, quarterly FD interest payouts, and semi-annual bond coupons."),
         QStringLiteral("CSV"), 1, 0},

        {"TRANSACTION_HISTORY",
         QStringLiteral("Full Financial Ledger Audit Trail"),
         QStringLiteral("Immutable double-entry transaction history with timestamps, reference IDs, brokerage, and net amounts."),
         QStringLiteral("CSV"), 1, 1},

        {"UNREALIZED_PNL",
         QStringLiteral("Unrealized P&L Report"),
         QStringLiteral("Estimated unrealized gains and losses on open inventory positions using latest available market quotes."),
         QStringLiteral("CSV"), 2, 0},

        {"ASSET_ALLOCATION",
         QStringLiteral("Asset Allocation Report"),
         QStringLiteral("Diversification weights and proportional share of portfolio by asset class."),
         QStringLiteral("CSV"), 2, 1},

        {"FD_MATURITY",
         QStringLiteral("Fixed Deposit Maturity Report"),
         QStringLiteral("Upcoming and matured bank fixed deposits with compounding interest schedules and estimated maturity proceeds."),
         QStringLiteral("CSV"), 3, 0},

        {"BOND_MATURITY",
         QStringLiteral("Bond Maturity & Coupon Report"),
         QStringLiteral("Government securities and corporate bonds with face values, coupon yields, and upcoming payment dates."),
         QStringLiteral("CSV"), 3, 1},

        {"MONTHLY_SUMMARY",
         QStringLiteral("Monthly Transaction Summary"),
         QStringLiteral("Monthly investment cadence and capital outflow grouped by calendar month."),
         QStringLiteral("CSV"), 4, 0},

        {"PERFORMANCE",
         QStringLiteral("Investment Performance Report"),
         QStringLiteral("Total return comparison across instruments combining capital gain and dividend/interest income."),
         QStringLiteral("CSV"), 4, 1},

        {"CATEGORY_BREAKDOWN",
         QStringLiteral("Investment Category Breakdown"),
         QStringLiteral("Active holdings grouped by investment category with cost basis and current market value."),
         QStringLiteral("CSV"), 5, 0},

        {"AUDIT",
         QStringLiteral("User Activity Audit Log"),
         QStringLiteral("Complete immutable audit trail of user actions, login events, data modifications, and system alerts."),
         QStringLiteral("CSV"), 5, 1},
    };

    for (const auto &spec : specs) {
        std::string rId = spec.reportId;
        QString rTitle  = spec.title;

        auto *card = new QFrame(gridWidget);
        card->setStyleSheet(QStringLiteral(
            "QFrame {"
            "   background-color: #FFFFFF;"
            "   border: 1px solid #E5E7EB;"
            "   border-radius: 12px;"
            "   padding: 16px;"
            "}"
            "QFrame:hover {"
            "   border: 1px solid #CBD5E1;"
            "   background-color: #FAFBFC;"
            "}"
        ));
        auto *cLayout = new QVBoxLayout(card);
        cLayout->setSpacing(8);

        auto *topRow = new QHBoxLayout();
        auto *titleLbl = new QLabel(spec.title, card);
        titleLbl->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 700; color: #0F172A;"));
        titleLbl->setWordWrap(true);

        auto *fmtLabel = new QLabel(spec.badge, card);
        fmtLabel->setStyleSheet(QStringLiteral(
            "background-color: #EFF6FF; color: #2563EB; font-size: 10px; font-weight: 700; "
            "padding: 2px 6px; border-radius: 4px;"
        ));

        topRow->addWidget(titleLbl, 1);
        topRow->addWidget(fmtLabel);
        cLayout->addLayout(topRow);

        auto *descLbl = new QLabel(spec.desc, card);
        descLbl->setWordWrap(true);
        descLbl->setStyleSheet(QStringLiteral("font-size: 12px; color: #64748B; line-height: 1.4;"));
        cLayout->addWidget(descLbl);
        cLayout->addStretch(1);

        auto *genBtn = new QPushButton(QStringLiteral("Generate Statement"), card);
        genBtn->setCursor(Qt::PointingHandCursor);
        genBtn->setIcon(IconHelper::getIcon(IconHelper::IconType::Reports, QColor(37, 99, 235), 14));
        genBtn->setStyleSheet(Theme::secondaryButtonStyle());

        connect(genBtn, &QPushButton::clicked, this, [this, rId, rTitle]() {
            generateAndExportReport(rId, rTitle);
        });
        cLayout->addWidget(genBtn);

        grid->addWidget(card, spec.row, spec.col);
    }

    return gridWidget;
}

// ─── Service Wiring ─────────────────────────────────────────────────────────

void ReportsPage::loadSummaryData()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    // Run realized P&L report to extract STCG/LTCG numbers
    auto result = Services::ReportService::instance().generateReport(
        userId, "REALIZED_PNL", "", "", "", "", 1000
    );

    // Extract STCG / LTCG from summary cards if the report has them
    for (const auto &card : result.summaryCards) {
        if (card.first == "STCG" && m_stcgCard)
            m_stcgCard->setValue(QString::fromStdString(card.second));
        else if (card.first == "LTCG" && m_ltcgCard)
            m_ltcgCard->setValue(QString::fromStdString(card.second));
    }

    // Run dividend/interest report for income card
    auto incomeResult = Services::ReportService::instance().generateReport(
        userId, "DIVIDEND_INTEREST", "", "", "", "", 1000
    );
    for (const auto &card : incomeResult.summaryCards) {
        if ((card.first == "TOTAL_INCOME" || card.first == "Income") && m_incomeCard)
            m_incomeCard->setValue(QString::fromStdString(card.second));
    }
}

void ReportsPage::generateAndExportReport(const std::string &reportId, const QString &reportTitle)
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) {
        QMessageBox::warning(this, QStringLiteral("Not Logged In"),
                             QStringLiteral("Please log in to generate reports."));
        return;
    }

    QString defaultName = reportTitle.toLower().replace(QStringLiteral(" "), QStringLiteral("_"))
                          + QStringLiteral("_export.csv");
    QString path = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export Report: ") + reportTitle,
        defaultName,
        QStringLiteral("CSV Files (*.csv);;All Files (*.*)")
    );
    if (path.isEmpty()) return;

    auto result = Services::ReportService::instance().generateReport(
        userId, reportId, "", "", "", "", 10000
    );

    if (!result.success) {
        QMessageBox::critical(this, QStringLiteral("Report Error"),
                              QString::fromStdString(result.errorMessage));
        return;
    }

    std::string errMsg;
    if (!Services::CsvExportService::exportToFile(path.toStdString(), result, errMsg)) {
        QMessageBox::warning(this, QStringLiteral("Export Failed"),
                             QStringLiteral("Could not write CSV:\n") + QString::fromStdString(errMsg));
        addArchiveRow(reportTitle, QString::fromStdString(result.reportId),
                      QStringLiteral("FY 2025-26"), QStringLiteral("CSV"), QStringLiteral("Failed"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("Export Successful"),
                             reportTitle + QStringLiteral(" exported to:\n") + path);

    addArchiveRow(reportTitle, QString::fromStdString(result.reportId),
                  QStringLiteral("FY 2025-26"), QStringLiteral("CSV"), QStringLiteral("Exported"));
}

void ReportsPage::addArchiveRow(const QString &title, const QString &type,
                                 const QString &period, const QString &format, const QString &status)
{
    if (!m_generatedReportsTable) return;

    int newRow = m_generatedReportsTable->rowCount();
    m_generatedReportsTable->insertRow(newRow);
    m_generatedReportsTable->formatRowHeight(44);

    m_generatedReportsTable->setTextCell(newRow, 0, title);
    m_generatedReportsTable->setTextCell(newRow, 1, type);
    m_generatedReportsTable->setTextCell(newRow, 2, period);
    m_generatedReportsTable->setTextCell(newRow, 3, format);
    m_generatedReportsTable->setTextCell(newRow, 4,
        QDateTime::currentDateTime().toString(QStringLiteral("dd MMM yyyy  hh:mm")));

    StatusBadge::BadgeVariant variant =
        status == QStringLiteral("Exported") ? StatusBadge::BadgeVariant::Success
                                              : StatusBadge::BadgeVariant::Danger;
    m_generatedReportsTable->setBadgeCell(newRow, 5, status, variant);
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT



#include "MaturityTrackerPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <cstdint>
#include <string>

#include "Theme.h"
#include "IconHelper.h"
#include "SectionHeader.h"
#include "SessionManager.h"
#include "MaturityService.h"
#include "CsvExportService.h"
#include "ReportRepository.h"

namespace InvestSphere {
namespace Ui {

MaturityTrackerPage::MaturityTrackerPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadData();
}

void MaturityTrackerPage::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(20);

    // ── Summary Cards Row ────────────────────────────────────────────────────
    auto *summaryRow = new QHBoxLayout();
    summaryRow->setSpacing(16);

    m_in30DaysCard = new SummaryCard(QStringLiteral("Maturities (Next 30 Days)"),
                                     QStringLiteral("0 Instruments"),
                                     QStringLiteral("Immediate"),
                                     SummaryCard::TrendType::Neutral,
                                     IconHelper::IconType::Calendar, container);
    m_in30DaysCard->setSubtitle(QStringLiteral("₹0.00 expected liquidity"));
    summaryRow->addWidget(m_in30DaysCard);

    m_in90DaysCard = new SummaryCard(QStringLiteral("Maturities (Next 90 Days)"),
                                     QStringLiteral("0 Instruments"),
                                     QStringLiteral("Near-term"),
                                     SummaryCard::TrendType::Neutral,
                                     IconHelper::IconType::Calendar, container);
    m_in90DaysCard->setSubtitle(QStringLiteral("₹0.00 anticipated capital return"));
    summaryRow->addWidget(m_in90DaysCard);

    m_totalInflowCard = new SummaryCard(QStringLiteral("12-Month Maturity Pipeline"),
                                         QStringLiteral("₹0.00"),
                                         QStringLiteral("Scheduled"),
                                         SummaryCard::TrendType::Neutral,
                                         IconHelper::IconType::Analytics, container);
    m_totalInflowCard->setSubtitle(QStringLiteral("Cumulative fixed income redemptions"));
    summaryRow->addWidget(m_totalInflowCard);

    layout->addLayout(summaryRow);

    // ── Filter Bar ───────────────────────────────────────────────────────────
    m_filterBar = new FilterBar(
        QStringLiteral("Search by bank, account, or issuer..."),
        {QStringLiteral("All Instruments"), QStringLiteral("Fixed Deposits (FD)"),
         QStringLiteral("Bonds & G-Secs"), QStringLiteral("Sovereign Gold Bonds (SGB)")},
        {QStringLiteral("All Windows"), QStringLiteral("Next 30 Days"),
         QStringLiteral("Next 90 Days"), QStringLiteral("Next 1 Year"), QStringLiteral("Matured")},
        QStringLiteral("Export Schedule"),
        container
    );
    connect(m_filterBar, &FilterBar::searchChanged, this, &MaturityTrackerPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::filterChanged, this, &MaturityTrackerPage::onFilterChanged);
    connect(m_filterBar, &FilterBar::actionClicked, this, &MaturityTrackerPage::onExportClicked);
    layout->addWidget(m_filterBar);

    // ── Maturity Schedule Table ──────────────────────────────────────────────
    m_maturityTable = new ModernTableWidget(container);
    m_maturityTable->setHeaders({
        QStringLiteral("Instrument"),
        QStringLiteral("Institution / Issuer"),
        QStringLiteral("Asset Class"),
        QStringLiteral("Maturity Date"),
        QStringLiteral("Days Remaining"),
        QStringLiteral("Principal Deployed"),
        QStringLiteral("Maturity Inflow"),
        QStringLiteral("Status")
    });
    m_maturityTable->setEmptyMessage(QStringLiteral("No upcoming maturities found in the active portfolio timeline."));
    m_maturityTable->setMinimumHeight(380);
    layout->addWidget(m_maturityTable, 1);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

// ─── Helpers ────────────────────────────────────────────────────────────────

QString MaturityTrackerPage::formatCents(int64_t cents)
{
    if (cents <= 0) return QStringLiteral("₹0.00");
    double val = static_cast<double>(cents) / 100.0;
    return QStringLiteral("₹") + QString("%L1").arg(val, 0, 'f', 2);
}

QString MaturityTrackerPage::badgeText(const std::string &window)
{
    if (window == "7_DAYS")  return QStringLiteral("≤7 Days");
    if (window == "30_DAYS") return QStringLiteral("≤30 Days");
    if (window == "90_DAYS") return QStringLiteral("≤90 Days");
    if (window == "MATURED") return QStringLiteral("Matured");
    return QStringLiteral("Upcoming");
}

// ─── Public Slot ────────────────────────────────────────────────────────────

void MaturityTrackerPage::loadData()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    auto &svc = Services::MaturityService::instance();
    auto summary = svc.getMaturitySummary(userId);

    if (m_in30DaysCard) {
        m_in30DaysCard->setValue(QString::number(summary.in30DaysCount) + QStringLiteral(" Instrument") +
                                  (summary.in30DaysCount != 1 ? QStringLiteral("s") : QString()));
        m_in30DaysCard->setSubtitle(formatCents(summary.in30DaysAmountCents) + QStringLiteral(" expected liquidity"));
        m_in30DaysCard->setTrend(
            summary.in30DaysCount > 0 ? QStringLiteral("Action Required") : QStringLiteral("None Imminent"),
            summary.in30DaysCount > 0 ? SummaryCard::TrendType::Negative : SummaryCard::TrendType::Neutral
        );
    }
    if (m_in90DaysCard) {
        m_in90DaysCard->setValue(QString::number(summary.in90DaysCount) + QStringLiteral(" Instrument") +
                                  (summary.in90DaysCount != 1 ? QStringLiteral("s") : QString()));
        m_in90DaysCard->setSubtitle(formatCents(summary.in90DaysAmountCents) + QStringLiteral(" anticipated"));
        m_in90DaysCard->setTrend(
            summary.in90DaysCount > 0 ? QStringLiteral("Near-term") : QStringLiteral("None Near-term"),
            summary.in90DaysCount > 0 ? SummaryCard::TrendType::Warning : SummaryCard::TrendType::Neutral
        );
    }
    if (m_totalInflowCard) {
        m_totalInflowCard->setValue(formatCents(summary.totalFutureAmountCents));
        m_totalInflowCard->setSubtitle(
            QString::number(summary.totalFutureCount) + QStringLiteral(" future redemptions scheduled")
        );
    }

    populateTable();
}

// ─── Private Slots ──────────────────────────────────────────────────────────

void MaturityTrackerPage::onFilterChanged()
{
    populateTable();
}

void MaturityTrackerPage::onExportClicked()
{
    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) return;

    QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export Maturity Schedule"),
        QStringLiteral("maturity_schedule.csv"),
        QStringLiteral("CSV Files (*.csv)")
    );
    if (path.isEmpty()) return;

    Database::ReportResult result;
    result.reportId = "MATURITY_SCHEDULE";
    result.title    = "Maturity Tracker Schedule";
    result.headers  = {"Instrument", "Institution", "Asset Class", "Maturity Date",
                       "Days Remaining", "Principal (INR)", "Proceeds (INR)", "Status"};

    auto items = Services::MaturityService::instance().getMaturitySchedule(userId, "ALL", "ALL");
    for (const auto &item : items) {
        result.rows.push_back({
            item.title,
            item.institutionOrIssuer,
            item.assetType,
            item.maturityDate,
            std::to_string(item.daysRemaining),
            std::to_string(static_cast<double>(item.totalPrincipalCents) / 100.0),
            std::to_string(static_cast<double>(item.estimatedProceedsCents) / 100.0),
            item.status
        });
    }

    std::string errMsg;
    if (!Services::CsvExportService::exportToFile(path.toStdString(), result, errMsg)) {
        QMessageBox::warning(this, QStringLiteral("Export Failed"),
                             QStringLiteral("Could not write CSV: ") + QString::fromStdString(errMsg));
    } else {
        QMessageBox::information(this, QStringLiteral("Export Successful"),
                                 QStringLiteral("Maturity schedule exported to:\n") + path);
    }
}

void MaturityTrackerPage::populateTable()
{
    if (!m_maturityTable || !m_filterBar) return;

    int userId = App::SessionManager::instance().currentUserId();
    if (userId <= 0) { m_maturityTable->setRowCount(0); return; }

    std::string windowFilter = "ALL_FUTURE";
    QString status = m_filterBar->selectedStatus();
    if (status == QStringLiteral("Next 30 Days"))  windowFilter = "30_DAYS";
    else if (status == QStringLiteral("Next 90 Days"))  windowFilter = "90_DAYS";
    else if (status == QStringLiteral("Matured"))        windowFilter = "MATURED";
    else if (status == QStringLiteral("All Windows"))    windowFilter = "ALL";

    std::string assetFilter = "ALL";
    QString cat = m_filterBar->selectedCategory();
    if (cat == QStringLiteral("Fixed Deposits (FD)"))    assetFilter = "FIXED_DEPOSIT";
    else if (cat.startsWith(QStringLiteral("Bonds")) ||
             cat.startsWith(QStringLiteral("Sovereign"))) assetFilter = "BOND";

    auto items = Services::MaturityService::instance().getMaturitySchedule(
        userId, windowFilter, assetFilter, m_filterBar->searchText().toStdString()
    );

    m_maturityTable->setRowCount(0);
    m_maturityTable->setRowCount(static_cast<int>(items.size()));
    m_maturityTable->formatRowHeight(44);

    int row = 0;
    for (const auto &item : items) {
        m_maturityTable->setTextCell(row, 0, QString::fromStdString(item.title));
        m_maturityTable->setTextCell(row, 1, QString::fromStdString(item.institutionOrIssuer));

        StatusBadge::BadgeVariant assetVariant =
            item.assetType == "FIXED_DEPOSIT" ? StatusBadge::BadgeVariant::Info : StatusBadge::BadgeVariant::Warning;
        m_maturityTable->setBadgeCell(row, 2,
            item.assetType == "FIXED_DEPOSIT" ? QStringLiteral("Fixed Deposit") : QStringLiteral("Bond"),
            assetVariant);

        m_maturityTable->setTextCell(row, 3, QString::fromStdString(item.maturityDate));

        int days = item.daysRemaining;
        QString daysStr = days >= 0 ? QString::number(days) + QStringLiteral(" days") : QStringLiteral("Matured");
        StatusBadge::BadgeVariant daysVariant =
            days < 0   ? StatusBadge::BadgeVariant::Neutral :
            days <= 30 ? StatusBadge::BadgeVariant::Danger  :
            days <= 90 ? StatusBadge::BadgeVariant::Warning :
                         StatusBadge::BadgeVariant::Success;
        m_maturityTable->setBadgeCell(row, 4, daysStr, daysVariant);

        m_maturityTable->setCurrencyCell(row, 5,
            static_cast<double>(item.totalPrincipalCents) / 100.0);
        m_maturityTable->setCurrencyCell(row, 6,
            static_cast<double>(item.estimatedProceedsCents) / 100.0);

        StatusBadge::BadgeVariant statusVariant =
            item.status == "ACTIVE"   ? StatusBadge::BadgeVariant::Success :
            item.status == "MATURED"  ? StatusBadge::BadgeVariant::Neutral :
                                        StatusBadge::BadgeVariant::Danger;
        m_maturityTable->setBadgeCell(row, 7, QString::fromStdString(item.status), statusVariant);
        ++row;
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

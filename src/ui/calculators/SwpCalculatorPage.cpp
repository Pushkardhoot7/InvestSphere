#include "SwpCalculatorPage.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QFrame>
#include <QLabel>
#include <cmath>

#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

SwpCalculatorPage::SwpCalculatorPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    recalculate();
}

QString SwpCalculatorPage::formatInr(double val) const
{
    return QStringLiteral("₹%L1").arg(val, 0, 'f', 0);
}

QString SwpCalculatorPage::formatInrCents(int64_t cents) const
{
    double val = static_cast<double>(cents) / 100.0;
    return QStringLiteral("₹%L1").arg(val, 0, 'f', 0);
}

void SwpCalculatorPage::setupUi()
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

    // ── Header Card ─────────────────────────────────────────────────────────
    auto *headerCard = new QFrame(container);
    headerCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 18px 24px;"
        "}"
    ));
    auto *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setSpacing(6);

    auto *hTitle = new QLabel(QStringLiteral("Systematic Withdrawal Plan (SWP) Calculator"), headerCard);
    hTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 20px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));

    auto *hDesc = new QLabel(
        QStringLiteral("Plan sustainable cash flow withdrawals from your mutual fund or investment corpus. "
                       "Convention: Each month, return is accrued first (nominal annual rate / 12), followed by the withdrawal. "
                       "If withdrawals exceed total value, depletion is accurately highlighted."),
        headerCard
    );
    hDesc->setWordWrap(true);
    hDesc->setStyleSheet(QStringLiteral("font-family: %1; font-size: 13px; color: #64748B; line-height: 1.4;").arg(Theme::fontFamily()));

    headerLayout->addWidget(hTitle);
    headerLayout->addWidget(hDesc);
    layout->addWidget(headerCard);

    // ── Depletion Warning Banner (default hidden) ───────────────────────────
    m_depletionBanner = new QFrame(container);
    m_depletionBanner->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FEF2F2;"
        "   border: 1px solid #FCA5A5;"
        "   border-radius: 8px;"
        "   padding: 12px 16px;"
        "}"
    ));
    auto *depLayout = new QHBoxLayout(m_depletionBanner);
    depLayout->setContentsMargins(8, 6, 8, 6);
    depLayout->setSpacing(12);

    auto *warnIcon = new QLabel(QStringLiteral("⚠️"), m_depletionBanner);
    warnIcon->setStyleSheet(QStringLiteral("font-size: 20px;"));
    depLayout->addWidget(warnIcon);

    m_depletionLabel = new QLabel(m_depletionBanner);
    m_depletionLabel->setWordWrap(true);
    m_depletionLabel->setStyleSheet(QStringLiteral("color: #991B1B; font-weight: 600; font-size: 13px;"));
    depLayout->addWidget(m_depletionLabel, 1);

    m_depletionBanner->setVisible(false);
    layout->addWidget(m_depletionBanner);

    // ── Two Columns: Inputs Left, Results Right ──────────────────────────────
    auto *mainRow = new QHBoxLayout();
    mainRow->setSpacing(24);

    auto *inputWrapper = new QWidget(container);
    setupInputPanel(inputWrapper);
    mainRow->addWidget(inputWrapper, 4);

    auto *resultWrapper = new QWidget(container);
    setupResultPanel(resultWrapper);
    mainRow->addWidget(resultWrapper, 6);

    layout->addLayout(mainRow);

    // ── Year-wise Projection Table Section ───────────────────────────────────
    auto *tableCard = new QFrame(container);
    tableCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 20px 24px;"
        "}"
    ));
    auto *tableCardLayout = new QVBoxLayout(tableCard);
    tableCardLayout->setSpacing(14);

    auto *tableTitle = new QLabel(QStringLiteral("Year-Wise Withdrawal & Balance Timeline"), tableCard);
    tableTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    tableCardLayout->addWidget(tableTitle);

    m_yearTable = new QTableWidget(tableCard);
    m_yearTable->setColumnCount(5);
    m_yearTable->setHorizontalHeaderLabels({
        QStringLiteral("Year"),
        QStringLiteral("Monthly Withdrawal (₹)"),
        QStringLiteral("Annual Withdrawn (₹)"),
        QStringLiteral("Remaining Balance (₹)"),
        QStringLiteral("Status")
    });
    m_yearTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_yearTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_yearTable->setAlternatingRowColors(true);
    m_yearTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_yearTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_yearTable->verticalHeader()->setVisible(false);
    m_yearTable->setMinimumHeight(240);
    m_yearTable->setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 8px;"
        "   gridline-color: #F1F5F9;"
        "   background-color: #FFFFFF;"
        "   alternate-background-color: #F8FAFC;"
        "   font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #F8FAFC;"
        "   color: #475569;"
        "   font-weight: 600;"
        "   font-size: 12px;"
        "   padding: 10px;"
        "   border: none;"
        "   border-bottom: 2px solid #E2E8F0;"
        "}"
        "QTableWidget::item {"
        "   padding: 10px;"
        "   color: #1E293B;"
        "}"
    ));
    tableCardLayout->addWidget(m_yearTable);

    // Disclaimer
    auto *disclaimerLbl = new QLabel(
        QStringLiteral("<b>Disclaimer:</b> Projections assume consistent compounding rates without market volatility. "
                       "Actual market returns fluctuate and can lead to faster corpus depletion during downturns. "
                       "No projected values or duration guarantees are implied."),
        tableCard
    );
    disclaimerLbl->setWordWrap(true);
    disclaimerLbl->setStyleSheet(QStringLiteral("font-size: 11px; color: #94A3B8; font-style: italic;"));
    tableCardLayout->addWidget(disclaimerLbl);

    layout->addWidget(tableCard);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void SwpCalculatorPage::setupInputPanel(QWidget *parent)
{
    auto *layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(parent);
    card->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 24px;"
        "}"
    ));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(20);

    auto *panelTitle = new QLabel(QStringLiteral("Withdrawal Plan Parameters"), card);
    panelTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    cardLayout->addWidget(panelTitle);

    // 1. Initial Investment Corpus
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Initial Portfolio Corpus (₹)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_initialCorpusSpin = new QDoubleSpinBox(card);
        m_initialCorpusSpin->setRange(1000.0, 100000000.0);
        m_initialCorpusSpin->setSingleStep(50000.0);
        m_initialCorpusSpin->setValue(2500000.0);
        m_initialCorpusSpin->setDecimals(0);
        m_initialCorpusSpin->setPrefix(QStringLiteral("₹ "));
        m_initialCorpusSpin->setFixedWidth(150);
        m_initialCorpusSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_initialCorpusSpin);
        block->addLayout(row);

        m_initialCorpusSlider = new QSlider(Qt::Horizontal, card);
        m_initialCorpusSlider->setRange(100, 10000); // 100k to 10M (* 1000)
        m_initialCorpusSlider->setValue(2500);
        m_initialCorpusSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_initialCorpusSlider);

        connect(m_initialCorpusSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
            int sliderVal = static_cast<int>(val / 1000.0);
            if (sliderVal <= 10000) {
                m_initialCorpusSlider->blockSignals(true);
                m_initialCorpusSlider->setValue(sliderVal);
                m_initialCorpusSlider->blockSignals(false);
            }
            recalculate();
        });
        connect(m_initialCorpusSlider, &QSlider::valueChanged, this, [this](int val) {
            m_initialCorpusSpin->blockSignals(true);
            m_initialCorpusSpin->setValue(val * 1000.0);
            m_initialCorpusSpin->blockSignals(false);
            recalculate();
        });

        cardLayout->addLayout(block);
    }

    // 2. Monthly Withdrawal Amount
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Monthly Withdrawal (₹)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_monthlyWithdrawalSpin = new QDoubleSpinBox(card);
        m_monthlyWithdrawalSpin->setRange(0.0, 5000000.0);
        m_monthlyWithdrawalSpin->setSingleStep(1000.0);
        m_monthlyWithdrawalSpin->setValue(20000.0);
        m_monthlyWithdrawalSpin->setDecimals(0);
        m_monthlyWithdrawalSpin->setPrefix(QStringLiteral("₹ "));
        m_monthlyWithdrawalSpin->setFixedWidth(150);
        m_monthlyWithdrawalSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_monthlyWithdrawalSpin);
        block->addLayout(row);

        m_monthlyWithdrawalSlider = new QSlider(Qt::Horizontal, card);
        m_monthlyWithdrawalSlider->setRange(0, 200); // 0 to 200k in 1k steps
        m_monthlyWithdrawalSlider->setValue(20);
        m_monthlyWithdrawalSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_monthlyWithdrawalSlider);

        connect(m_monthlyWithdrawalSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
            int sliderVal = static_cast<int>(val / 1000.0);
            if (sliderVal <= 200) {
                m_monthlyWithdrawalSlider->blockSignals(true);
                m_monthlyWithdrawalSlider->setValue(sliderVal);
                m_monthlyWithdrawalSlider->blockSignals(false);
            }
            recalculate();
        });
        connect(m_monthlyWithdrawalSlider, &QSlider::valueChanged, this, [this](int val) {
            m_monthlyWithdrawalSpin->blockSignals(true);
            m_monthlyWithdrawalSpin->setValue(val * 1000.0);
            m_monthlyWithdrawalSpin->blockSignals(false);
            recalculate();
        });

        cardLayout->addLayout(block);
    }

    // 3. Expected Annual Return Rate (%)
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Expected Return Rate (p.a.)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_returnRateSpin = new QDoubleSpinBox(card);
        m_returnRateSpin->setRange(0.0, 30.0);
        m_returnRateSpin->setSingleStep(0.5);
        m_returnRateSpin->setValue(8.0);
        m_returnRateSpin->setDecimals(1);
        m_returnRateSpin->setSuffix(QStringLiteral(" %"));
        m_returnRateSpin->setFixedWidth(150);
        m_returnRateSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_returnRateSpin);
        block->addLayout(row);

        m_returnRateSlider = new QSlider(Qt::Horizontal, card);
        m_returnRateSlider->setRange(0, 300); // 0.0% to 30.0% (*10)
        m_returnRateSlider->setValue(80);
        m_returnRateSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_returnRateSlider);

        connect(m_returnRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
            m_returnRateSlider->blockSignals(true);
            m_returnRateSlider->setValue(static_cast<int>(std::round(val * 10.0)));
            m_returnRateSlider->blockSignals(false);
            recalculate();
        });
        connect(m_returnRateSlider, &QSlider::valueChanged, this, [this](int val) {
            m_returnRateSpin->blockSignals(true);
            m_returnRateSpin->setValue(val / 10.0);
            m_returnRateSpin->blockSignals(false);
            recalculate();
        });

        cardLayout->addLayout(block);
    }

    // 4. Withdrawal Duration (Years)
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Withdrawal Duration"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_periodYearsSpin = new QSpinBox(card);
        m_periodYearsSpin->setRange(1, 40);
        m_periodYearsSpin->setValue(15);
        m_periodYearsSpin->setSuffix(QStringLiteral(" Years"));
        m_periodYearsSpin->setFixedWidth(150);
        m_periodYearsSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_periodYearsSpin);
        block->addLayout(row);

        m_periodYearsSlider = new QSlider(Qt::Horizontal, card);
        m_periodYearsSlider->setRange(1, 40);
        m_periodYearsSlider->setValue(15);
        m_periodYearsSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_periodYearsSlider);

        connect(m_periodYearsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
            m_periodYearsSlider->blockSignals(true);
            m_periodYearsSlider->setValue(val);
            m_periodYearsSlider->blockSignals(false);
            recalculate();
        });
        connect(m_periodYearsSlider, &QSlider::valueChanged, this, [this](int val) {
            m_periodYearsSpin->blockSignals(true);
            m_periodYearsSpin->setValue(val);
            m_periodYearsSpin->blockSignals(false);
            recalculate();
        });

        cardLayout->addLayout(block);
    }

    // Validation label
    m_validationLabel = new QLabel(card);
    m_validationLabel->setStyleSheet(QStringLiteral("color: #DC2626; font-size: 12px; font-weight: 600;"));
    m_validationLabel->setVisible(false);
    cardLayout->addWidget(m_validationLabel);

    // Reset button
    m_resetBtn = new QPushButton(QStringLiteral("Reset Defaults"), card);
    m_resetBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 8px;"
        "   padding: 10px 18px;"
        "   font-weight: 600;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E2E8F0;"
        "   color: #1E293B;"
        "}"
    ));
    connect(m_resetBtn, &QPushButton::clicked, this, &SwpCalculatorPage::resetInputs);
    cardLayout->addWidget(m_resetBtn);

    layout->addWidget(card);
}

void SwpCalculatorPage::setupResultPanel(QWidget *parent)
{
    auto *layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // Grid for 4 Summary Cards
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    m_initialCorpusCard = new SummaryCard(
        QStringLiteral("Initial Investment"),
        QStringLiteral("₹0"),
        QStringLiteral("Starting Principal"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        parent
    );
    grid->addWidget(m_initialCorpusCard, 0, 0);

    m_totalWithdrawnCard = new SummaryCard(
        QStringLiteral("Total Amount Withdrawn"),
        QStringLiteral("₹0"),
        QStringLiteral("Cash flow received"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::Transactions,
        parent
    );
    grid->addWidget(m_totalWithdrawnCard, 0, 1);

    m_remainingBalanceCard = new SummaryCard(
        QStringLiteral("Estimated Remaining Balance"),
        QStringLiteral("₹0"),
        QStringLiteral("Corpus at tenure end"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::Analytics,
        parent
    );
    grid->addWidget(m_remainingBalanceCard, 1, 0);

    m_totalEarningsCard = new SummaryCard(
        QStringLiteral("Investment Earnings"),
        QStringLiteral("₹0"),
        QStringLiteral("Gross returns accrued"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::TrendUp,
        parent
    );
    grid->addWidget(m_totalEarningsCard, 1, 1);

    layout->addLayout(grid);

    // 5th Card: Withdrawals completed
    m_withdrawalsCompletedCard = new SummaryCard(
        QStringLiteral("Withdrawals Completed"),
        QStringLiteral("0"),
        QStringLiteral("Scheduled cycles"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Calendar,
        parent
    );
    layout->addWidget(m_withdrawalsCompletedCard);
}

void SwpCalculatorPage::resetInputs()
{
    if (m_initialCorpusSpin)      m_initialCorpusSpin->setValue(2500000.0);
    if (m_monthlyWithdrawalSpin)  m_monthlyWithdrawalSpin->setValue(20000.0);
    if (m_returnRateSpin)         m_returnRateSpin->setValue(8.0);
    if (m_periodYearsSpin)        m_periodYearsSpin->setValue(15);
    recalculate();
}

void SwpCalculatorPage::recalculate()
{
    if (!m_initialCorpusSpin || !m_monthlyWithdrawalSpin || !m_returnRateSpin || !m_periodYearsSpin) return;

    double corpus     = m_initialCorpusSpin->value();
    double withdrawal = m_monthlyWithdrawalSpin->value();
    double rate       = m_returnRateSpin->value();
    int    years      = m_periodYearsSpin->value();

    auto res = Finance::AnnuityEngine::calculateSwp(corpus, withdrawal, rate, years);

    if (!res.valid) {
        if (m_validationLabel) {
            m_validationLabel->setText(QString::fromStdString(res.errorMessage));
            m_validationLabel->setVisible(true);
        }
        return;
    }

    if (m_validationLabel) {
        m_validationLabel->setVisible(false);
    }

    // 1. Update Cards
    m_initialCorpusCard->setValue(formatInrCents(res.initialCorpusCents));
    m_initialCorpusCard->setSubtitle(QStringLiteral("%1 years time horizon").arg(years));

    m_totalWithdrawnCard->setValue(formatInrCents(res.totalWithdrawnCents));
    m_totalWithdrawnCard->setSubtitle(QStringLiteral("%1 monthly withdrawals").arg(res.withdrawalsCompleted));

    m_remainingBalanceCard->setValue(formatInrCents(res.finalBalanceCents));
    if (res.finalBalanceCents > 0) {
        m_remainingBalanceCard->setTrend(QStringLiteral("Capital Intact"), SummaryCard::TrendType::Positive);
    } else {
        m_remainingBalanceCard->setTrend(QStringLiteral("Corpus Depleted"), SummaryCard::TrendType::Negative);
    }

    m_totalEarningsCard->setValue(formatInrCents(res.totalEarningsCents));
    m_totalEarningsCard->setSubtitle(QStringLiteral("At %1% annual return").arg(rate, 0, 'f', 1));

    int totalScheduled = years * 12;
    m_withdrawalsCompletedCard->setValue(
        QStringLiteral("%1 of %2").arg(res.withdrawalsCompleted).arg(totalScheduled)
    );
    if (res.depleted) {
        m_withdrawalsCompletedCard->setSubtitle(QStringLiteral("Exhausted at month %1").arg(res.depletedAtMonth));
    } else {
        m_withdrawalsCompletedCard->setSubtitle(QStringLiteral("100% of scheduled cycles completed"));
    }

    // 2. Depletion Warning Banner
    if (res.depleted) {
        int year = (res.depletedAtMonth + 11) / 12;
        int monthInYear = ((res.depletedAtMonth - 1) % 12) + 1;
        m_depletionLabel->setText(
            QStringLiteral("<b>Portfolio Depletion Warning:</b> Corpus was completely exhausted in <b>Year %1, Month %2</b> "
                           "(Month %3 overall). Withdrawals ceased after this point, and the final withdrawal was only partial. "
                           "Consider lowering your withdrawal amount or increasing starting capital.")
                .arg(year)
                .arg(monthInYear)
                .arg(res.depletedAtMonth)
        );
        m_depletionBanner->setVisible(true);
    } else {
        m_depletionBanner->setVisible(false);
    }

    // 3. Populate Table
    populateYearTable(res);
}

void SwpCalculatorPage::populateYearTable(const Finance::SwpResult &res)
{
    if (!m_yearTable) return;

    m_yearTable->setRowCount(static_cast<int>(res.yearlyBreakdown.size()));
    for (int i = 0; i < static_cast<int>(res.yearlyBreakdown.size()); ++i) {
        const auto &yr = res.yearlyBreakdown[i];

        auto *itemYear  = new QTableWidgetItem(QStringLiteral("Year %1").arg(yr.year));
        auto *itemMthWd = new QTableWidgetItem(formatInr(yr.withdrawalMonthly));
        auto *itemYrWd  = new QTableWidgetItem(formatInrCents(yr.totalWithdrawnThisYearCents));
        auto *itemBal   = new QTableWidgetItem(formatInrCents(yr.balanceAtYearEndCents));

        QString statusText;
        if (yr.depletedThisYear) {
            statusText = QStringLiteral("⚠️ Depleted (M%1)").arg(yr.depletedAtMonth);
        } else if (yr.balanceAtYearEndCents == 0) {
            statusText = QStringLiteral("Depleted");
        } else {
            statusText = QStringLiteral("Active");
        }
        auto *itemStatus = new QTableWidgetItem(statusText);

        itemYear->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemMthWd->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemYrWd->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemBal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemStatus->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        if (yr.depletedThisYear || yr.balanceAtYearEndCents == 0) {
            itemBal->setForeground(QColor(QStringLiteral("#DC2626")));
            itemStatus->setForeground(QColor(QStringLiteral("#DC2626")));
        } else {
            itemBal->setForeground(QColor(QStringLiteral("#16A34A")));
            itemStatus->setForeground(QColor(QStringLiteral("#2563EB")));
        }

        m_yearTable->setItem(i, 0, itemYear);
        m_yearTable->setItem(i, 1, itemMthWd);
        m_yearTable->setItem(i, 2, itemYrWd);
        m_yearTable->setItem(i, 3, itemBal);
        m_yearTable->setItem(i, 4, itemStatus);
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

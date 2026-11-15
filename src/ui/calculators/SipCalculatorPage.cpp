#include "SipCalculatorPage.h"

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

SipCalculatorPage::SipCalculatorPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    recalculate();
}

QString SipCalculatorPage::formatInr(double val) const
{
    return QStringLiteral("₹%L1").arg(val, 0, 'f', 0);
}

QString SipCalculatorPage::formatInrCents(int64_t cents) const
{
    double val = static_cast<double>(cents) / 100.0;
    return QStringLiteral("₹%L1").arg(val, 0, 'f', 0);
}

void SipCalculatorPage::setupUi()
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

    auto *hTitle = new QLabel(QStringLiteral("Systematic Investment Plan (SIP) Calculator"), headerCard);
    hTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 20px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));

    auto *hDesc = new QLabel(
        QStringLiteral("Plan your recurring monthly mutual fund and equity investments. "
                       "Calculations use monthly compounding with end-of-month contribution timing (ordinary annuity): "
                       "FV = P × [((1 + r)ⁿ − 1) / r]. Optional annual step-up compounds each year's higher contribution forward."),
        headerCard
    );
    hDesc->setWordWrap(true);
    hDesc->setStyleSheet(QStringLiteral("font-family: %1; font-size: 13px; color: #64748B; line-height: 1.4;").arg(Theme::fontFamily()));

    headerLayout->addWidget(hTitle);
    headerLayout->addWidget(hDesc);
    layout->addWidget(headerCard);

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

    auto *tableTitle = new QLabel(QStringLiteral("Year-Wise Growth Schedule"), tableCard);
    tableTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    tableCardLayout->addWidget(tableTitle);

    m_yearTable = new QTableWidget(tableCard);
    m_yearTable->setColumnCount(5);
    m_yearTable->setHorizontalHeaderLabels({
        QStringLiteral("Year"),
        QStringLiteral("Total Invested (₹)"),
        QStringLiteral("Portfolio Value (₹)"),
        QStringLiteral("Estimated Gain (₹)"),
        QStringLiteral("Gain %")
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
        QStringLiteral("<b>Disclaimer:</b> Calculations are based on assumed rates of return and are for illustrative purposes only. "
                       "Mutual fund investments and equity securities are subject to market risks. Read all scheme-related documents carefully."),
        tableCard
    );
    disclaimerLbl->setWordWrap(true);
    disclaimerLbl->setStyleSheet(QStringLiteral("font-size: 11px; color: #94A3B8; font-style: italic;"));
    tableCardLayout->addWidget(disclaimerLbl);

    layout->addWidget(tableCard);

    scrollArea->setWidget(container);
    rootLayout->addWidget(scrollArea);
}

void SipCalculatorPage::setupInputPanel(QWidget *parent)
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

    auto *panelTitle = new QLabel(QStringLiteral("Investment Parameters"), card);
    panelTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 16px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    cardLayout->addWidget(panelTitle);

    // 1. Monthly Investment Amount
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Monthly Investment (₹)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_monthlyAmountSpin = new QDoubleSpinBox(card);
        m_monthlyAmountSpin->setRange(100.0, 10000000.0);
        m_monthlyAmountSpin->setSingleStep(1000.0);
        m_monthlyAmountSpin->setValue(10000.0);
        m_monthlyAmountSpin->setDecimals(0);
        m_monthlyAmountSpin->setPrefix(QStringLiteral("₹ "));
        m_monthlyAmountSpin->setFixedWidth(140);
        m_monthlyAmountSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_monthlyAmountSpin);
        block->addLayout(row);

        m_monthlyAmountSlider = new QSlider(Qt::Horizontal, card);
        m_monthlyAmountSlider->setRange(500, 200000); // Up to 2 Lakh on slider
        m_monthlyAmountSlider->setSingleStep(500);
        m_monthlyAmountSlider->setValue(10000);
        m_monthlyAmountSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_monthlyAmountSlider);

        connect(m_monthlyAmountSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
            if (val <= 200000.0) {
                m_monthlyAmountSlider->blockSignals(true);
                m_monthlyAmountSlider->setValue(static_cast<int>(val));
                m_monthlyAmountSlider->blockSignals(false);
            }
            recalculate();
        });
        connect(m_monthlyAmountSlider, &QSlider::valueChanged, this, [this](int val) {
            m_monthlyAmountSpin->blockSignals(true);
            m_monthlyAmountSpin->setValue(val);
            m_monthlyAmountSpin->blockSignals(false);
            recalculate();
        });

        cardLayout->addLayout(block);
    }

    // 2. Expected Annual Return Rate (%)
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Expected Return Rate (p.a.)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_returnRateSpin = new QDoubleSpinBox(card);
        m_returnRateSpin->setRange(0.0, 50.0);
        m_returnRateSpin->setSingleStep(0.5);
        m_returnRateSpin->setValue(12.0);
        m_returnRateSpin->setDecimals(1);
        m_returnRateSpin->setSuffix(QStringLiteral(" %"));
        m_returnRateSpin->setFixedWidth(140);
        m_returnRateSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_returnRateSpin);
        block->addLayout(row);

        m_returnRateSlider = new QSlider(Qt::Horizontal, card);
        m_returnRateSlider->setRange(0, 300); // 0.0% to 30.0% (*10)
        m_returnRateSlider->setValue(120);
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

    // 3. Investment Duration (Years)
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Investment Duration"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_periodYearsSpin = new QSpinBox(card);
        m_periodYearsSpin->setRange(1, 40);
        m_periodYearsSpin->setValue(10);
        m_periodYearsSpin->setSuffix(QStringLiteral(" Years"));
        m_periodYearsSpin->setFixedWidth(140);
        m_periodYearsSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_periodYearsSpin);
        block->addLayout(row);

        m_periodYearsSlider = new QSlider(Qt::Horizontal, card);
        m_periodYearsSlider->setRange(1, 40);
        m_periodYearsSlider->setValue(10);
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

    // 4. Annual Step-up Percentage (%)
    {
        auto *block = new QVBoxLayout();
        block->setSpacing(6);
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(QStringLiteral("Annual Step-Up (% p.a.)"), card);
        lbl->setStyleSheet(QStringLiteral("font-weight: 600; font-size: 13px; color: #1E293B;"));

        m_stepUpSpin = new QDoubleSpinBox(card);
        m_stepUpSpin->setRange(0.0, 50.0);
        m_stepUpSpin->setSingleStep(1.0);
        m_stepUpSpin->setValue(0.0);
        m_stepUpSpin->setDecimals(1);
        m_stepUpSpin->setSuffix(QStringLiteral(" %"));
        m_stepUpSpin->setFixedWidth(140);
        m_stepUpSpin->setStyleSheet(Theme::inputFieldStyle());

        row->addWidget(lbl);
        row->addStretch(1);
        row->addWidget(m_stepUpSpin);
        block->addLayout(row);

        m_stepUpSlider = new QSlider(Qt::Horizontal, card);
        m_stepUpSlider->setRange(0, 30); // 0% to 30%
        m_stepUpSlider->setValue(0);
        m_stepUpSlider->setStyleSheet(QStringLiteral(
            "QSlider::groove:horizontal { height: 6px; background: #E2E8F0; border-radius: 3px; }"
            "QSlider::sub-page:horizontal { background: #2563EB; border-radius: 3px; }"
            "QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #2563EB; width: 18px; margin-top: -6px; margin-bottom: -6px; border-radius: 9px; }"
        ));
        block->addWidget(m_stepUpSlider);

        connect(m_stepUpSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
            m_stepUpSlider->blockSignals(true);
            m_stepUpSlider->setValue(static_cast<int>(val));
            m_stepUpSlider->blockSignals(false);
            recalculate();
        });
        connect(m_stepUpSlider, &QSlider::valueChanged, this, [this](int val) {
            m_stepUpSpin->blockSignals(true);
            m_stepUpSpin->setValue(val);
            m_stepUpSpin->blockSignals(false);
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
    connect(m_resetBtn, &QPushButton::clicked, this, &SipCalculatorPage::resetInputs);
    cardLayout->addWidget(m_resetBtn);

    layout->addWidget(card);
}

void SipCalculatorPage::setupResultPanel(QWidget *parent)
{
    auto *layout = new QVBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // 4 Summary Cards in a 2x2 grid
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    m_investedCard = new SummaryCard(
        QStringLiteral("Total Amount Invested"),
        QStringLiteral("₹0"),
        QStringLiteral("120 installments"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Portfolio,
        parent
    );
    grid->addWidget(m_investedCard, 0, 0);

    m_returnsCard = new SummaryCard(
        QStringLiteral("Estimated Wealth Gain"),
        QStringLiteral("₹0"),
        QStringLiteral("+0.0%"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::TrendUp,
        parent
    );
    grid->addWidget(m_returnsCard, 0, 1);

    m_totalMaturityCard = new SummaryCard(
        QStringLiteral("Projected Maturity Corpus"),
        QStringLiteral("₹0"),
        QStringLiteral("Future Value"),
        SummaryCard::TrendType::Positive,
        IconHelper::IconType::Analytics,
        parent
    );
    grid->addWidget(m_totalMaturityCard, 1, 0);

    m_installmentsCard = new SummaryCard(
        QStringLiteral("Total Installments"),
        QStringLiteral("120"),
        QStringLiteral("Monthly schedule"),
        SummaryCard::TrendType::Neutral,
        IconHelper::IconType::Calendar,
        parent
    );
    grid->addWidget(m_installmentsCard, 1, 1);

    layout->addLayout(grid);

    // Proportion Breakdown Bar Card
    auto *propCard = new QFrame(parent);
    propCard->setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E2E8F0;"
        "   border-radius: 12px;"
        "   padding: 18px 20px;"
        "}"
    ));
    auto *propLayout = new QVBoxLayout(propCard);
    propLayout->setSpacing(10);

    auto *propTitle = new QLabel(QStringLiteral("Capital Allocation: Principal vs Growth"), propCard);
    propTitle->setStyleSheet(QStringLiteral("font-family: %1; font-size: 14px; font-weight: 700; color: #0F172A;").arg(Theme::fontFamily()));
    propLayout->addWidget(propTitle);

    m_breakdownLegend = new QLabel(QStringLiteral("Principal (50%) | Growth (50%)"), propCard);
    m_breakdownLegend->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 600; color: #475569;"));
    propLayout->addWidget(m_breakdownLegend);

    m_breakdownBar = new QFrame(propCard);
    m_breakdownBar->setFixedHeight(14);
    m_breakdownBar->setStyleSheet(QStringLiteral("background-color: #E2E8F0; border-radius: 7px;"));
    propLayout->addWidget(m_breakdownBar);

    layout->addWidget(propCard);
}

void SipCalculatorPage::resetInputs()
{
    if (m_monthlyAmountSpin) m_monthlyAmountSpin->setValue(10000.0);
    if (m_returnRateSpin)    m_returnRateSpin->setValue(12.0);
    if (m_periodYearsSpin)   m_periodYearsSpin->setValue(10);
    if (m_stepUpSpin)        m_stepUpSpin->setValue(0.0);
    recalculate();
}

void SipCalculatorPage::recalculate()
{
    if (!m_monthlyAmountSpin || !m_returnRateSpin || !m_periodYearsSpin || !m_stepUpSpin) return;

    double monthly = m_monthlyAmountSpin->value();
    double rate    = m_returnRateSpin->value();
    int    years   = m_periodYearsSpin->value();
    double stepUp  = m_stepUpSpin->value();

    auto res = Finance::AnnuityEngine::calculateSip(monthly, rate, years, stepUp);

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
    m_investedCard->setValue(formatInrCents(res.totalInvestedCents));
    if (stepUp > 0.0) {
        m_investedCard->setSubtitle(QStringLiteral("%1 installments (with %2% annual step-up)").arg(res.totalInstallments).arg(stepUp, 0, 'f', 1));
    } else {
        m_investedCard->setSubtitle(QStringLiteral("%1 monthly installments of %2").arg(res.totalInstallments).arg(formatInr(monthly)));
    }

    m_returnsCard->setValue(formatInrCents(res.estimatedGainCents));
    m_returnsCard->setTrend(QStringLiteral("+%1%").arg(res.returnPercent, 0, 'f', 1), SummaryCard::TrendType::Positive);

    m_totalMaturityCard->setValue(formatInrCents(res.maturityValueCents));
    m_totalMaturityCard->setSubtitle(QStringLiteral("Over %1 years @ %2% p.a.").arg(years).arg(rate, 0, 'f', 1));

    m_installmentsCard->setValue(QString::number(res.totalInstallments));
    m_installmentsCard->setSubtitle(QStringLiteral("%1 years horizon").arg(years));

    // 2. Update Breakdown Bar
    double maturityTotal = static_cast<double>(res.maturityValueCents);
    if (maturityTotal > 0.0) {
        double investedRatio = (static_cast<double>(res.totalInvestedCents) / maturityTotal) * 100.0;
        double gainsRatio    = (static_cast<double>(res.estimatedGainCents) / maturityTotal) * 100.0;
        if (gainsRatio < 0.0) gainsRatio = 0.0;

        m_breakdownLegend->setText(
            QStringLiteral("Principal: %1% (Blue)  •  Estimated Gain: %2% (Green)")
                .arg(investedRatio, 0, 'f', 1)
                .arg(gainsRatio, 0, 'f', 1)
        );

        double split = investedRatio / 100.0;
        if (split > 1.0) split = 1.0;
        if (split < 0.0) split = 0.0;

        m_breakdownBar->setStyleSheet(QStringLiteral(
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #2563EB, stop:%1 #2563EB, stop:%1 #16A34A, stop:1 #16A34A);"
            "border-radius: 7px;"
        ).arg(split));
    }

    // 3. Populate Year Table
    populateYearTable(res);
}

void SipCalculatorPage::populateYearTable(const Finance::SipResult &res)
{
    if (!m_yearTable) return;

    m_yearTable->setRowCount(static_cast<int>(res.yearlyBreakdown.size()));
    for (int i = 0; i < static_cast<int>(res.yearlyBreakdown.size()); ++i) {
        const auto &pt = res.yearlyBreakdown[i];

        auto *itemYear = new QTableWidgetItem(QStringLiteral("Year %1").arg(pt.year));
        auto *itemInv  = new QTableWidgetItem(formatInrCents(pt.investedAtYearEndCents));
        auto *itemVal  = new QTableWidgetItem(formatInrCents(pt.portfolioValueAtYearEndCents));
        auto *itemGain = new QTableWidgetItem(formatInrCents(pt.wealthGainAtYearEndCents));
        auto *itemPct  = new QTableWidgetItem(QStringLiteral("+%1%").arg(pt.returnPercent, 0, 'f', 1));

        itemYear->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itemInv->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemVal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemGain->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemPct->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Highlight gain with green
        itemGain->setForeground(QColor(QStringLiteral("#16A34A")));
        itemPct->setForeground(QColor(QStringLiteral("#16A34A")));

        m_yearTable->setItem(i, 0, itemYear);
        m_yearTable->setItem(i, 1, itemInv);
        m_yearTable->setItem(i, 2, itemVal);
        m_yearTable->setItem(i, 3, itemGain);
        m_yearTable->setItem(i, 4, itemPct);
    }
}

void SipCalculatorPage::updateDonutChart(const Finance::SipResult &)
{
    // Placeholder for Qt Charts or custom painter if needed
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

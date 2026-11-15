#ifndef INVESTSPHERE_SIPCALCULATORPAGE_H
#define INVESTSPHERE_SIPCALCULATORPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include "SummaryCard.h"
#include "ChartPlaceholder.h"
#include "AnnuityEngine.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Interactive Systematic Investment Plan (SIP) Calculator.
 *
 * Contribution timing: END-OF-MONTH (ordinary annuity).
 * Formula: FV = P × [((1 + r)^n − 1) / r]
 *   where r = annualRate / 12 / 100,  n = total months.
 * For zero rate: FV = P × n.
 * For step-up SIP: year-by-year accumulation with annual contribution increase.
 */
class SipCalculatorPage : public QWidget {
    Q_OBJECT

public:
    explicit SipCalculatorPage(QWidget *parent = nullptr);

private slots:
    void recalculate();
    void resetInputs();

private:
    void setupUi();
    void setupInputPanel(QWidget *parent);
    void setupResultPanel(QWidget *parent);
    void populateYearTable(const Finance::SipResult &res);
    void updateDonutChart(const Finance::SipResult &res);
    QString formatInr(double val) const;
    QString formatInrCents(int64_t cents) const;

    // ── Inputs ───────────────────────────────────────────────────────────────
    QDoubleSpinBox *m_monthlyAmountSpin{nullptr};
    QSlider        *m_monthlyAmountSlider{nullptr};

    QDoubleSpinBox *m_returnRateSpin{nullptr};
    QSlider        *m_returnRateSlider{nullptr};

    QSpinBox       *m_periodYearsSpin{nullptr};
    QSlider        *m_periodYearsSlider{nullptr};

    QDoubleSpinBox *m_stepUpSpin{nullptr};
    QSlider        *m_stepUpSlider{nullptr};

    QPushButton    *m_resetBtn{nullptr};

    // ── Summary cards ─────────────────────────────────────────────────────────
    SummaryCard *m_investedCard{nullptr};
    SummaryCard *m_returnsCard{nullptr};
    SummaryCard *m_totalMaturityCard{nullptr};
    SummaryCard *m_installmentsCard{nullptr};

    // ── Proportion bar ────────────────────────────────────────────────────────
    QFrame *m_breakdownBar{nullptr};
    QLabel *m_breakdownLegend{nullptr};

    // ── Allocation chart ─────────────────────────────────────────────────────
    ChartPlaceholder *m_allocationChart{nullptr};

    // ── Year-wise table ───────────────────────────────────────────────────────
    QTableWidget *m_yearTable{nullptr};

    // ── Validation label ──────────────────────────────────────────────────────
    QLabel *m_validationLabel{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SIPCALCULATORPAGE_H

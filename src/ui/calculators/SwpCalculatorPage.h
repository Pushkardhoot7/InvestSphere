#ifndef INVESTSPHERE_SWPCALCULATORPAGE_H
#define INVESTSPHERE_SWPCALCULATORPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QFrame>

#include "SummaryCard.h"
#include "AnnuityEngine.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Interactive Systematic Withdrawal Plan (SWP) Calculator.
 *
 * Models capital preservation, returns, and corpus depletion for retirees
 * and regular income seekers.
 *
 * Calculation order: Monthly growth first, then withdrawal at month end.
 * Compounding convention: nominal rate r = annualRate / 12 / 100.
 */
class SwpCalculatorPage : public QWidget {
    Q_OBJECT

public:
    explicit SwpCalculatorPage(QWidget *parent = nullptr);

private slots:
    void recalculate();
    void resetInputs();

private:
    void setupUi();
    void setupInputPanel(QWidget *parent);
    void setupResultPanel(QWidget *parent);
    void populateYearTable(const Finance::SwpResult &res);
    QString formatInr(double val) const;
    QString formatInrCents(int64_t cents) const;

    // ── Inputs ───────────────────────────────────────────────────────────────
    QDoubleSpinBox *m_initialCorpusSpin{nullptr};
    QSlider        *m_initialCorpusSlider{nullptr};

    QDoubleSpinBox *m_monthlyWithdrawalSpin{nullptr};
    QSlider        *m_monthlyWithdrawalSlider{nullptr};

    QDoubleSpinBox *m_returnRateSpin{nullptr};
    QSlider        *m_returnRateSlider{nullptr};

    QSpinBox       *m_periodYearsSpin{nullptr};
    QSlider        *m_periodYearsSlider{nullptr};

    QPushButton    *m_resetBtn{nullptr};
    QLabel         *m_validationLabel{nullptr};

    // ── Depletion Alert Banner ───────────────────────────────────────────────
    QFrame         *m_depletionBanner{nullptr};
    QLabel         *m_depletionLabel{nullptr};

    // ── Summary Cards ─────────────────────────────────────────────────────────
    SummaryCard    *m_initialCorpusCard{nullptr};
    SummaryCard    *m_totalWithdrawnCard{nullptr};
    SummaryCard    *m_remainingBalanceCard{nullptr};
    SummaryCard    *m_totalEarningsCard{nullptr};
    SummaryCard    *m_withdrawalsCompletedCard{nullptr};

    // ── Year-wise Projection Table ───────────────────────────────────────────
    QTableWidget   *m_yearTable{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SWPCALCULATORPAGE_H

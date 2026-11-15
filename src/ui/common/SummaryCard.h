#ifndef INVESTSPHERE_SUMMARYCARD_H
#define INVESTSPHERE_SUMMARYCARD_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Professional financial summary card for high-level metrics.
 * Displays title, bold financial figure, trend/delta badge, helper note, and icon.
 */
class SummaryCard : public QFrame {
    Q_OBJECT

public:
    enum class TrendType {
        Neutral,
        Positive,
        Negative
    };

    explicit SummaryCard(const QString &title,
                         const QString &initialValue = QStringLiteral("₹0.00"),
                         const QString &deltaText = QStringLiteral("0.0%"),
                         TrendType trend = TrendType::Neutral,
                         IconHelper::IconType icon = IconHelper::IconType::Dashboard,
                         QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setValue(const QString &value);
    void setTrend(const QString &deltaText, TrendType trend);
    void setSubtitle(const QString &subtitle);
    void setIcon(IconHelper::IconType icon);

private:
    void setupUi(IconHelper::IconType icon);
    void updateTrendBadge(const QString &text, TrendType trend);

    QLabel *m_titleLabel{nullptr};
    QLabel *m_valueLabel{nullptr};
    QLabel *m_trendBadge{nullptr};
    QLabel *m_subtitleLabel{nullptr};
    QLabel *m_iconLabel{nullptr};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_SUMMARYCARD_H

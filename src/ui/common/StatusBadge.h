#ifndef INVESTSPHERE_STATUSBADGE_H
#define INVESTSPHERE_STATUSBADGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QLabel>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Clean pill badge for financial status, instrument types, and risk levels.
 */
class StatusBadge : public QLabel {
    Q_OBJECT

public:
    enum class BadgeVariant {
        Success, // Green (e.g. Active, Profit, Low Risk)
        Danger,  // Red (e.g. Inactive, Loss, High Risk)
        Warning, // Amber (e.g. Pending, Moderate Risk, Maturing Soon)
        Info,    // Blue (e.g. Systematic, Equity, Govt Bond)
        Neutral  // Gray (e.g. Closed, Default, Draft)
    };

    explicit StatusBadge(const QString &text = QString(),
                         BadgeVariant variant = BadgeVariant::Neutral,
                         QWidget *parent = nullptr);

    void setStatus(const QString &text, BadgeVariant variant);
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_STATUSBADGE_H

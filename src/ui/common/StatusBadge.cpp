#include "StatusBadge.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include "Theme.h"

namespace InvestSphere {
namespace Ui {

StatusBadge::StatusBadge(const QString &text, BadgeVariant variant, QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setStatus(text, variant);
}

void StatusBadge::setStatus(const QString &text, BadgeVariant variant)
{
    setText(text);

    QString bgColor;
    QString textColor;
    QString borderColor;

    switch (variant) {
    case BadgeVariant::Success:
        bgColor = QStringLiteral("#DCFCE7");
        textColor = QStringLiteral("#15803D");
        borderColor = QStringLiteral("#BBF7D0");
        break;
    case BadgeVariant::Danger:
        bgColor = QStringLiteral("#FEE2E2");
        textColor = QStringLiteral("#B91C1C");
        borderColor = QStringLiteral("#FECACA");
        break;
    case BadgeVariant::Warning:
        bgColor = QStringLiteral("#FEF3C7");
        textColor = QStringLiteral("#B45309");
        borderColor = QStringLiteral("#FDE68A");
        break;
    case BadgeVariant::Info:
        bgColor = QStringLiteral("#EFF6FF");
        textColor = QStringLiteral("#1D4ED8");
        borderColor = QStringLiteral("#BFDBFE");
        break;
    case BadgeVariant::Neutral:
    default:
        bgColor = QStringLiteral("#F1F5F9");
        textColor = QStringLiteral("#475569");
        borderColor = QStringLiteral("#E2E8F0");
        break;
    }

    setStyleSheet(QStringLiteral(
        "QLabel {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: 4px;"
        "   padding: 3px 8px;"
        "   font-family: %4;"
        "   font-size: 11px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "}"
    ).arg(bgColor).arg(textColor).arg(borderColor).arg(Theme::fontFamily()));
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

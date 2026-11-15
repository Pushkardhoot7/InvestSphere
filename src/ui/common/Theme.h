#ifndef INVESTSPHERE_THEME_H
#define INVESTSPHERE_THEME_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QString>
#include <QColor>
#include <QFont>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Centralized Visual Design System and Theme tokens for InvestSphere.
 * Provides colors, typography specifications, spacing scales, and QSS generators.
 */
class Theme {
public:
    // Color Palette Constants
    static inline const QString ColorPrimarySidebar   = QStringLiteral("#111827"); // Dark navy
    static inline const QString ColorSidebarHover     = QStringLiteral("#1F2937"); // Slate hover
    static inline const QString ColorSidebarActive    = QStringLiteral("#2563EB"); // Royal blue accent
    static inline const QString ColorSidebarText      = QStringLiteral("#9CA3AF"); // Muted slate text
    static inline const QString ColorSidebarTextHover = QStringLiteral("#F9FAFB"); // White-grey

    static inline const QString ColorMainBackground   = QStringLiteral("#F5F7FB"); // Light gray canvas
    static inline const QString ColorCardBackground   = QStringLiteral("#FFFFFF"); // White cards
    static inline const QString ColorCardBorder       = QStringLiteral("#E5E7EB"); // Subtle border
    static inline const QString ColorCardBorderHover  = QStringLiteral("#D1D5DB");

    static inline const QString ColorPrimaryAccent    = QStringLiteral("#2563EB"); // Primary blue
    static inline const QString ColorPrimaryHover     = QStringLiteral("#1D4ED8"); // Darker blue
    static inline const QString ColorPrimaryPressed   = QStringLiteral("#1E40AF");

    static inline const QString ColorPositive         = QStringLiteral("#16A34A"); // Success green
    static inline const QString ColorPositiveBg       = QStringLiteral("#DCFCE7"); // Soft green tint
    static inline const QString ColorNegative         = QStringLiteral("#DC2626"); // Danger red
    static inline const QString ColorNegativeBg       = QStringLiteral("#FEE2E2"); // Soft red tint
    static inline const QString ColorWarning          = QStringLiteral("#D97706"); // Amber warning
    static inline const QString ColorWarningBg        = QStringLiteral("#FEF3C7"); // Soft amber tint
    static inline const QString ColorInfo             = QStringLiteral("#2563EB"); // Info blue
    static inline const QString ColorInfoBg           = QStringLiteral("#EFF6FF"); // Soft blue tint

    static inline const QString ColorTextPrimary      = QStringLiteral("#1F2937"); // Dark slate
    static inline const QString ColorTextSecondary    = QStringLiteral("#64748B"); // Muted slate
    static inline const QString ColorTextTertiary     = QStringLiteral("#94A3B8"); // Subdued label text

    static inline const QString ColorBorder           = QStringLiteral("#E5E7EB"); // Standard divider/border
    static inline const QString ColorBorderLight      = QStringLiteral("#F1F5F9");
    static inline const QString ColorInputBackground  = QStringLiteral("#FFFFFF");
    static inline const QString ColorInputFocusBorder = QStringLiteral("#2563EB");

    // Spacing Scale
    static constexpr int Space4  = 4;
    static constexpr int Space8  = 8;
    static constexpr int Space12 = 12;
    static constexpr int Space16 = 16;
    static constexpr int Space24 = 24;
    static constexpr int Space32 = 32;

    // Border Radius Scale
    static constexpr int RadiusSm   = 6;
    static constexpr int RadiusMd   = 8;
    static constexpr int RadiusLg   = 12;
    static constexpr int RadiusPill = 999;

    // Typography
    static QString fontFamily();
    static QFont headingFont(int pointSize = 16, int weight = QFont::Bold);
    static QFont bodyFont(int pointSize = 10, int weight = QFont::Normal);
    static QFont metricFont(int pointSize = 18, int weight = QFont::Bold);

    // Centralized Application Style Sheet
    static QString getApplicationStyleSheet();

    // Specific Widget Styles
    static QString primaryButtonStyle();
    static QString secondaryButtonStyle();
    static QString destructiveButtonStyle();
    static QString cardFrameStyle();
    static QString inputFieldStyle();
    static QString tableWidgetStyle();
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_THEME_H

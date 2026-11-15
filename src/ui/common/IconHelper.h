#ifndef INVESTSPHERE_ICONHELPER_H
#define INVESTSPHERE_ICONHELPER_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QSize>

namespace InvestSphere {
namespace Ui {

/**
 * @brief High-DPI Resolution-Independent Vector Icon Generator.
 * Renders modern, crisp financial iconography using QPainter paths.
 * Eliminates external asset dependencies, broken paths, or missing fonts.
 */
class IconHelper {
public:
    enum class IconType {
        BrandLogo,
        Dashboard,
        Portfolio,
        Stocks,
        MutualFunds,
        Gold,
        FixedDeposits,
        Bonds,
        Transactions,
        MarketWatch,
        SipCalculator,
        SwpCalculator,
        MaturityTracker,
        Analytics,
        Reports,
        Settings,
        Logout,
        Search,
        Bell,
        User,
        Plus,
        Filter,
        Refresh,
        TrendUp,
        TrendDown,
        Checkmark,
        AlertCircle,
        InfoCircle,
        Calendar,
        ChevronRight
    };

    static QIcon getIcon(IconType type, const QColor &color = QColor(148, 163, 184), int size = 20);
    static QPixmap getPixmap(IconType type, const QColor &color = QColor(148, 163, 184), int size = 20);

private:
    static void drawVectorIcon(QPainter &p, IconType type, const QColor &color, int s);
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_ICONHELPER_H

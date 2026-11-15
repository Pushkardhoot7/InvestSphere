#include "IconHelper.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QPainter>
#include <QPainterPath>

namespace InvestSphere {
namespace Ui {

QIcon IconHelper::getIcon(IconType type, const QColor &color, int size)
{
    QIcon icon;
    icon.addPixmap(getPixmap(type, color, size));
    icon.addPixmap(getPixmap(type, color, size * 2)); // High-DPI support
    return icon;
}

QPixmap IconHelper::getPixmap(IconType type, const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    drawVectorIcon(painter, type, color, size);

    return pixmap;
}

void IconHelper::drawVectorIcon(QPainter &p, IconType type, const QColor &color, int s)
{
    p.save();
    qreal scale = s / 24.0;
    p.scale(scale, scale);

    QPen pen(color);
    pen.setWidthF(1.8);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    switch (type) {
    case IconType::BrandLogo: {
        // Geometric stylized investment emblem: diamond sphere with upward node
        QPainterPath path;
        path.moveTo(12, 2);
        path.lineTo(21, 8);
        path.lineTo(17, 21);
        path.lineTo(7, 21);
        path.lineTo(3, 8);
        path.closeSubpath();
        p.setBrush(QColor(color.red(), color.green(), color.blue(), 40));
        p.drawPath(path);
        p.drawLine(12, 2, 12, 21);
        p.drawLine(3, 8, 21, 8);
        break;
    }
    case IconType::Dashboard: {
        // 4 dashboard metric grid tiles
        p.drawRoundedRect(3, 3, 7, 7, 1.5, 1.5);
        p.drawRoundedRect(14, 3, 7, 7, 1.5, 1.5);
        p.drawRoundedRect(3, 14, 7, 7, 1.5, 1.5);
        p.drawRoundedRect(14, 14, 7, 7, 1.5, 1.5);
        break;
    }
    case IconType::Portfolio: {
        // Business briefcase
        p.drawRoundedRect(3, 7, 18, 13, 2, 2);
        QPainterPath handle;
        handle.moveTo(8, 7);
        handle.lineTo(8, 4);
        handle.lineTo(16, 4);
        handle.lineTo(16, 7);
        p.drawPath(handle);
        p.drawLine(3, 12, 21, 12);
        break;
    }
    case IconType::Stocks: {
        // Upward trending candlestick / bars
        p.drawLine(4, 20, 20, 20);
        p.drawLine(6, 17, 6, 11);
        p.drawLine(11, 17, 11, 8);
        p.drawLine(16, 17, 16, 5);
        QPainterPath trend;
        trend.moveTo(5, 13);
        trend.lineTo(10, 9);
        trend.lineTo(14, 12);
        trend.lineTo(19, 4);
        p.drawPath(trend);
        break;
    }
    case IconType::MutualFunds: {
        // Concentric / multi-asset pie chart
        p.drawEllipse(3, 3, 18, 18);
        p.drawLine(12, 12, 12, 3);
        p.drawLine(12, 12, 19, 16);
        p.drawLine(12, 12, 5, 16);
        break;
    }
    case IconType::Gold: {
        // Ingot / gold bar trapezoid
        QPainterPath ingot;
        ingot.moveTo(6, 6);
        ingot.lineTo(18, 6);
        ingot.lineTo(21, 17);
        ingot.lineTo(3, 17);
        ingot.closeSubpath();
        p.drawPath(ingot);
        p.drawLine(6, 6, 8, 17);
        p.drawLine(18, 6, 16, 17);
        break;
    }
    case IconType::FixedDeposits: {
        // Bank temple / safe facade
        p.drawLine(2, 20, 22, 20);
        p.drawLine(4, 18, 20, 18);
        QPainterPath pediment;
        pediment.moveTo(2, 8);
        pediment.lineTo(12, 3);
        pediment.lineTo(22, 8);
        pediment.closeSubpath();
        p.drawPath(pediment);
        p.drawLine(6, 8, 6, 17);
        p.drawLine(10, 8, 10, 17);
        p.drawLine(14, 8, 14, 17);
        p.drawLine(18, 8, 18, 17);
        break;
    }
    case IconType::Bonds: {
        // Bond certificate / badge ribbon
        p.drawRoundedRect(4, 3, 16, 14, 2, 2);
        p.drawLine(7, 7, 17, 7);
        p.drawLine(7, 11, 14, 11);
        QPainterPath ribbon;
        ribbon.moveTo(14, 17);
        ribbon.lineTo(14, 21);
        ribbon.lineTo(16, 19);
        ribbon.lineTo(18, 21);
        ribbon.lineTo(18, 17);
        p.drawPath(ribbon);
        break;
    }
    case IconType::Transactions: {
        // Dual bidirectional arrows
        p.drawLine(4, 7, 18, 7);
        p.drawLine(14, 3, 18, 7);
        p.drawLine(14, 11, 18, 7);

        p.drawLine(20, 17, 6, 17);
        p.drawLine(10, 13, 6, 17);
        p.drawLine(10, 21, 6, 17);
        break;
    }
    case IconType::MarketWatch: {
        // Eye / market pulse monitor
        QPainterPath wave;
        wave.moveTo(2, 12);
        wave.lineTo(6, 12);
        wave.lineTo(9, 6);
        wave.lineTo(13, 18);
        wave.lineTo(16, 10);
        wave.lineTo(18, 12);
        wave.lineTo(22, 12);
        p.drawPath(wave);
        break;
    }
    case IconType::SipCalculator: {
        // Calculator with growth plus
        p.drawRoundedRect(4, 3, 16, 18, 2, 2);
        p.drawRect(7, 6, 10, 4);
        p.drawPoint(8, 13);
        p.drawPoint(12, 13);
        p.drawPoint(16, 13);
        p.drawPoint(8, 17);
        p.drawPoint(12, 17);
        p.drawPoint(16, 17);
        break;
    }
    case IconType::SwpCalculator: {
        // Calculator with outflow arrow
        p.drawRoundedRect(4, 3, 16, 18, 2, 2);
        p.drawRect(7, 6, 10, 4);
        p.drawLine(12, 12, 12, 18);
        p.drawLine(9, 15, 12, 18);
        p.drawLine(15, 15, 12, 18);
        break;
    }
    case IconType::MaturityTracker: {
        // Calendar with clock alert
        p.drawRoundedRect(3, 5, 18, 16, 2, 2);
        p.drawLine(3, 9, 21, 9);
        p.drawLine(8, 3, 8, 6);
        p.drawLine(16, 3, 16, 6);
        p.drawEllipse(11, 11, 8, 8);
        p.drawLine(15, 13, 15, 15);
        p.drawLine(15, 15, 17, 15);
        break;
    }
    case IconType::Analytics: {
        // Analytics graph with nodes
        p.drawLine(3, 20, 21, 20);
        p.drawLine(3, 20, 3, 4);
        QPainterPath chart;
        chart.moveTo(5, 16);
        chart.lineTo(10, 11);
        chart.lineTo(15, 13);
        chart.lineTo(20, 6);
        p.drawPath(chart);
        p.drawEllipse(QPointF(10, 11), 1.5, 1.5);
        p.drawEllipse(QPointF(15, 13), 1.5, 1.5);
        p.drawEllipse(QPointF(20, 6), 1.5, 1.5);
        break;
    }
    case IconType::Reports: {
        // Document with data lines
        p.drawRoundedRect(4, 3, 16, 18, 2, 2);
        p.drawLine(8, 8, 16, 8);
        p.drawLine(8, 12, 16, 12);
        p.drawLine(8, 16, 12, 16);
        break;
    }
    case IconType::Settings: {
        // Cog / gear
        p.drawEllipse(QPointF(12, 12), 4, 4);
        p.drawLine(12, 3, 12, 6);
        p.drawLine(12, 18, 12, 21);
        p.drawLine(3, 12, 6, 12);
        p.drawLine(18, 12, 21, 12);
        p.drawLine(5.5, 5.5, 7.5, 7.5);
        p.drawLine(16.5, 16.5, 18.5, 18.5);
        p.drawLine(5.5, 18.5, 7.5, 16.5);
        p.drawLine(16.5, 7.5, 18.5, 5.5);
        break;
    }
    case IconType::Logout: {
        // Exit door with arrow
        p.drawLine(9, 3, 4, 3);
        p.drawLine(4, 3, 4, 21);
        p.drawLine(4, 21, 9, 21);
        p.drawLine(9, 12, 20, 12);
        p.drawLine(16, 8, 20, 12);
        p.drawLine(16, 16, 20, 12);
        break;
    }
    case IconType::Search: {
        // Magnifying glass
        p.drawEllipse(4, 4, 11, 11);
        p.drawLine(12.5, 12.5, 20, 20);
        break;
    }
    case IconType::Bell: {
        // Notification bell
        QPainterPath bell;
        bell.moveTo(12, 3);
        bell.arcTo(6, 6, 12, 11, 0, 180);
        bell.lineTo(4, 17);
        bell.lineTo(20, 17);
        bell.lineTo(18, 12);
        p.drawPath(bell);
        p.drawLine(10, 19, 14, 19);
        break;
    }
    case IconType::User: {
        // User silhouette
        p.drawEllipse(QPointF(12, 8), 4, 4);
        QPainterPath shoulders;
        shoulders.moveTo(4, 20);
        shoulders.arcTo(4, 12, 16, 12, 180, -180);
        p.drawPath(shoulders);
        break;
    }
    case IconType::Plus: {
        p.drawLine(12, 5, 12, 19);
        p.drawLine(5, 12, 19, 12);
        break;
    }
    case IconType::Filter: {
        QPainterPath filter;
        filter.moveTo(3, 4);
        filter.lineTo(21, 4);
        filter.lineTo(14, 12);
        filter.lineTo(14, 19);
        filter.lineTo(10, 17);
        filter.lineTo(10, 12);
        filter.closeSubpath();
        p.drawPath(filter);
        break;
    }
    case IconType::Refresh: {
        p.drawArc(4, 4, 16, 16, 45 * 16, 270 * 16);
        p.drawLine(16, 3, 19, 7);
        p.drawLine(13, 7, 19, 7);
        break;
    }
    case IconType::TrendUp: {
        p.drawLine(4, 16, 14, 6);
        p.drawLine(14, 6, 8, 6);
        p.drawLine(14, 6, 14, 12);
        break;
    }
    case IconType::TrendDown: {
        p.drawLine(4, 8, 14, 18);
        p.drawLine(14, 18, 8, 18);
        p.drawLine(14, 18, 14, 12);
        break;
    }
    case IconType::Checkmark: {
        p.drawLine(4, 12, 9, 17);
        p.drawLine(9, 17, 20, 6);
        break;
    }
    case IconType::AlertCircle: {
        p.drawEllipse(3, 3, 18, 18);
        p.drawLine(12, 8, 12, 13);
        p.drawPoint(12, 16);
        break;
    }
    case IconType::InfoCircle: {
        p.drawEllipse(3, 3, 18, 18);
        p.drawPoint(12, 8);
        p.drawLine(12, 11, 12, 16);
        break;
    }
    case IconType::Calendar: {
        p.drawRoundedRect(3, 5, 18, 16, 2, 2);
        p.drawLine(3, 9, 21, 9);
        p.drawLine(7, 3, 7, 6);
        p.drawLine(17, 3, 17, 6);
        break;
    }
    case IconType::ChevronRight: {
        p.drawLine(9, 6, 15, 12);
        p.drawLine(15, 12, 9, 18);
        break;
    }
    }

    p.restore();
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

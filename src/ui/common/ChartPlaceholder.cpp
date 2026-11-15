#include "ChartPlaceholder.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include "Theme.h"
#include "IconHelper.h"

namespace InvestSphere {
namespace Ui {

ChartPlaceholder::ChartPlaceholder(const QString &title,
                                   ChartType type,
                                   const QString &emptyNotice,
                                   QWidget *parent)
    : QFrame(parent)
    , m_title(title)
    , m_type(type)
    , m_notice(emptyNotice)
{
    setupUi();
}

void ChartPlaceholder::setupUi()
{
    setMinimumHeight(240);
    setStyleSheet(QStringLiteral(
        "QFrame {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 12px;"
        "}"
    ));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 16, 18, 16);
    mainLayout->setSpacing(8);

    // Header Row: Title + Time Period Filter Pills
    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(8);

    m_titleLabel = new QLabel(m_title, this);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-family: %1; font-size: 14px; font-weight: 700; color: #0F172A;"
    ).arg(Theme::fontFamily()));
    headerRow->addWidget(m_titleLabel);
    headerRow->addStretch(1);

    if (m_type == ChartType::LineTrend) {
        auto *filterContainer = new QWidget(this);
        filterContainer->setStyleSheet(QStringLiteral(
            "background-color: #F1F5F9; border-radius: 6px; padding: 2px;"
        ));
        auto *pillsLayout = new QHBoxLayout(filterContainer);
        pillsLayout->setContentsMargins(2, 2, 2, 2);
        pillsLayout->setSpacing(2);

        QStringList periods = {QStringLiteral("1M"), QStringLiteral("6M"), QStringLiteral("1Y"), QStringLiteral("ALL")};
        for (int i = 0; i < periods.size(); ++i) {
            auto *btn = new QPushButton(periods[i], filterContainer);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedHeight(24);
            if (i == 3) { // "ALL" active
                btn->setStyleSheet(QStringLiteral(
                    "QPushButton { background-color: #FFFFFF; color: #2563EB; font-weight: 700; "
                    "font-size: 11px; border-radius: 4px; padding: 0px 8px; border: 1px solid #E2E8F0; }"
                ));
            } else {
                btn->setStyleSheet(QStringLiteral(
                    "QPushButton { background: transparent; color: #64748B; font-weight: 500; "
                    "font-size: 11px; border-radius: 4px; padding: 0px 8px; border: none; }"
                    "QPushButton:hover { color: #0F172A; }"
                ));
            }
            pillsLayout->addWidget(btn);
        }
        headerRow->addWidget(filterContainer);
    }

    mainLayout->addLayout(headerRow);
    mainLayout->addStretch(1);
}

void ChartPlaceholder::setChartTitle(const QString &title)
{
    m_title = title;
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void ChartPlaceholder::setEmptyNotice(const QString &notice)
{
    m_notice = notice;
    update();
}

void ChartPlaceholder::setAllocationSlices(const QVector<SliceData> &slices)
{
    m_slices = slices;
    update();
}

void ChartPlaceholder::clearAllocationSlices()
{
    m_slices.clear();
    update();
}

void ChartPlaceholder::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int topMargin = 50;
    int chartHeight = height() - topMargin - 20;
    int chartWidth = width() - 36;
    int startX = 18;
    int startY = topMargin;

    if (m_type == ChartType::LineTrend) {
        // Draw subtle grid lines
        QPen gridPen(QColor(241, 245, 249));
        gridPen.setWidth(1);
        p.setPen(gridPen);

        for (int i = 1; i <= 3; ++i) {
            int y = startY + (chartHeight * i / 4);
            p.drawLine(startX, y, startX + chartWidth, y);
        }

        // Draw baseline
        QPen axisPen(QColor(226, 232, 240));
        axisPen.setWidth(1);
        p.setPen(axisPen);
        p.drawLine(startX, startY + chartHeight, startX + chartWidth, startY + chartHeight);

        // Center notification container
        p.setPen(QColor(148, 163, 184));
        QFont font = p.font();
        font.setFamily(Theme::fontFamily());
        font.setPointSize(11);
        p.setFont(font);

        QRect textRect(startX, startY + 20, chartWidth, chartHeight - 20);
        p.drawText(textRect, Qt::AlignCenter, m_notice);

    } else if (m_type == ChartType::PieAllocation) {
        if (!m_slices.isEmpty()) {
            // Render actual vector Donut Chart with Category Legend
            int donutSize = qMin(chartHeight - 10, chartWidth / 2 - 20);
            if (donutSize < 100) donutSize = 100;

            int donutX = startX + 20;
            int donutY = startY + (chartHeight - donutSize) / 2;
            QRectF donutRect(donutX, donutY, donutSize, donutSize);

            double startAngle = 90.0 * 16.0; // Start at 12 o'clock

            // Draw donut arcs
            for (const auto &slice : m_slices) {
                if (slice.percentage <= 0.001) continue;
                double spanAngle = -(slice.percentage / 100.0) * 360.0 * 16.0;

                QPen arcPen(slice.color);
                arcPen.setWidth(24);
                arcPen.setCapStyle(Qt::FlatCap);
                p.setPen(arcPen);
                p.setBrush(Qt::NoBrush);

                QRectF arcRect = donutRect.adjusted(12, 12, -12, -12);
                p.drawArc(arcRect, static_cast<int>(startAngle), static_cast<int>(spanAngle));

                startAngle += spanAngle;
            }

            // Center hole label
            p.setPen(QColor(100, 116, 139));
            QFont centerFont = p.font();
            centerFont.setFamily(Theme::fontFamily());
            centerFont.setPointSize(10);
            centerFont.setBold(true);
            p.setFont(centerFont);
            p.drawText(donutRect, Qt::AlignCenter, QStringLiteral("Assets\n100%"));

            // Legend on right side
            int legendX = donutX + donutSize + 30;
            int legendY = startY + 15;
            int rowHeight = 26;

            for (int i = 0; i < m_slices.size(); ++i) {
                const auto &s = m_slices[i];
                int curY = legendY + (i * rowHeight);
                if (curY + 20 > height() - 10) break;

                // Color badge dot
                p.setPen(Qt::NoPen);
                p.setBrush(s.color);
                p.drawEllipse(legendX, curY + 4, 10, 10);

                // Label
                p.setPen(QColor(51, 65, 85));
                QFont lblFont = p.font();
                lblFont.setPointSize(11);
                lblFont.setBold(false);
                p.setFont(lblFont);
                p.drawText(legendX + 18, curY + 14, s.label);

                // Percentage badge
                QString pctText = QString::number(s.percentage, 'f', 1) + QStringLiteral("%");
                p.setPen(QColor(100, 116, 139));
                QFont pctFont = p.font();
                pctFont.setBold(true);
                p.setFont(pctFont);
                p.drawText(legendX + 140, curY + 14, pctText);
            }

        } else {
            // Empty state placeholder ring
            int centerRadius = qMin(chartHeight, chartWidth) / 3;
            QPoint center(startX + chartWidth / 2, startY + chartHeight / 2);

            QPen ringPen(QColor(241, 245, 249));
            ringPen.setWidth(16);
            p.setPen(ringPen);
            p.drawEllipse(center, centerRadius, centerRadius);

            p.setPen(QColor(148, 163, 184));
            QFont font = p.font();
            font.setFamily(Theme::fontFamily());
            font.setPointSize(11);
            p.setFont(font);

            QRect textRect(startX, startY + chartHeight / 2 - 20, chartWidth, 40);
            p.drawText(textRect, Qt::AlignCenter, m_notice);
        }
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

#include "ModernTableWidget.h"

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QHeaderView>
#include <QPainter>
#include <QHBoxLayout>
#include "Theme.h"

namespace InvestSphere {
namespace Ui {

ModernTableWidget::ModernTableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent)
{
    initStyling();
}

ModernTableWidget::ModernTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    initStyling();
}

void ModernTableWidget::initStyling()
{
    setStyleSheet(Theme::tableWidgetStyle());
    setShowGrid(false);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader()->setHighlightSections(false);
    setFocusPolicy(Qt::NoFocus);
    formatRowHeight(44);
}

void ModernTableWidget::setHeaders(const QStringList &headers)
{
    setColumnCount(headers.size());
    setHorizontalHeaderLabels(headers);
}

void ModernTableWidget::setEmptyMessage(const QString &message)
{
    m_emptyMessage = message;
    viewport()->update();
}

void ModernTableWidget::formatRowHeight(int height)
{
    verticalHeader()->setDefaultSectionSize(height);
}

void ModernTableWidget::setTextCell(int row, int col, const QString &text, Qt::Alignment align)
{
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(align);
    setItem(row, col, item);
}

void ModernTableWidget::setCurrencyCell(int row, int col, double amount, const QString &currencySymbol)
{
    QString formatted = QStringLiteral("%1 %2").arg(currencySymbol).arg(amount, 0, 'f', 2);
    auto *item = new QTableWidgetItem(formatted);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setFont(Theme::bodyFont(10, QFont::DemiBold));
    setItem(row, col, item);
}

void ModernTableWidget::setPercentageCell(int row, int col, double percent)
{
    QString prefix = (percent > 0.0) ? QStringLiteral("+") : QString();
    QString formatted = QStringLiteral("%1%2%").arg(prefix).arg(percent, 0, 'f', 2);
    auto *item = new QTableWidgetItem(formatted);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item->setFont(Theme::bodyFont(10, QFont::DemiBold));

    if (percent > 0.0) {
        item->setForeground(QColor(22, 163, 74));
    } else if (percent < 0.0) {
        item->setForeground(QColor(220, 38, 38));
    } else {
        item->setForeground(QColor(100, 116, 139));
    }

    setItem(row, col, item);
}

void ModernTableWidget::setBadgeCell(int row, int col, const QString &text, StatusBadge::BadgeVariant variant)
{
    auto *container = new QWidget(this);
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setAlignment(Qt::AlignCenter);

    auto *badge = new StatusBadge(text, variant, container);
    layout->addWidget(badge);

    setCellWidget(row, col, container);
}

void ModernTableWidget::paintEvent(QPaintEvent *event)
{
    QTableWidget::paintEvent(event);

    if (rowCount() == 0) {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QColor(148, 163, 184));
        QFont font = p.font();
        font.setFamily(QStringLiteral("Segoe UI"));
        font.setPointSize(11);
        p.setFont(font);

        p.drawText(viewport()->rect(), Qt::AlignCenter, m_emptyMessage);
    }
}

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT

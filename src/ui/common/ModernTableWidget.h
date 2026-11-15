#ifndef INVESTSPHERE_MODERNTABLEWIDGET_H
#define INVESTSPHERE_MODERNTABLEWIDGET_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QTableWidget>
#include <QLabel>
#include "StatusBadge.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Professional financial table widget with custom headers, alternating rows,
 * monetary formatting helpers, and built-in empty state overlay.
 */
class ModernTableWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit ModernTableWidget(int rows, int columns, QWidget *parent = nullptr);
    explicit ModernTableWidget(QWidget *parent = nullptr);

    void setHeaders(const QStringList &headers);
    void setEmptyMessage(const QString &message);
    void formatRowHeight(int height = 44);

    // Helper functions for cell population
    void setTextCell(int row, int col, const QString &text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);
    void setCurrencyCell(int row, int col, double amount, const QString &currencySymbol = QStringLiteral("₹"));
    void setPercentageCell(int row, int col, double percent);
    void setBadgeCell(int row, int col, const QString &text, StatusBadge::BadgeVariant variant);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initStyling();

    QString m_emptyMessage{QStringLiteral("No records found in this view.")};
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_MODERNTABLEWIDGET_H

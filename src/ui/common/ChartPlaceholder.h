#ifndef INVESTSPHERE_CHARTPLACEHOLDER_H
#define INVESTSPHERE_CHARTPLACEHOLDER_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVector>
#include <QColor>

namespace InvestSphere {
namespace Ui {

/**
 * @brief Dynamic visual chart container with period tabs, legend, and custom vector painted canvas.
 * Renders anti-aliased Donut charts for asset allocation when data is present,
 * and graceful empty states when portfolio data is unpopulated.
 */
class ChartPlaceholder : public QFrame {
    Q_OBJECT

public:
    enum class ChartType {
        PieAllocation,
        LineTrend,
        BarComparison
    };

    struct SliceData {
        QString label;
        double percentage{0.0};
        double value{0.0};
        QColor color;
    };

    explicit ChartPlaceholder(const QString &title,
                             ChartType type = ChartType::LineTrend,
                             const QString &emptyNotice = QStringLiteral("Interactive visualization ready for portfolio data."),
                             QWidget *parent = nullptr);

    void setChartTitle(const QString &title);
    void setEmptyNotice(const QString &notice);
    void setAllocationSlices(const QVector<SliceData> &slices);
    void clearAllocationSlices();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();

    QString m_title;
    ChartType m_type;
    QString m_notice;
    QLabel *m_titleLabel{nullptr};
    QVector<SliceData> m_slices;
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_CHARTPLACEHOLDER_H

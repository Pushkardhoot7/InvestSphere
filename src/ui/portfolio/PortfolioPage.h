#ifndef INVESTSPHERE_PORTFOLIOPAGE_H
#define INVESTSPHERE_PORTFOLIOPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <vector>
#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"
#include "EmptyStateWidget.h"
#include "InvestmentService.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Master Portfolio Overview page displaying multi-asset holdings,
 * classification filters, aggregate performance, and management actions.
 */
class PortfolioPage : public QWidget {
    Q_OBJECT

public:
    explicit PortfolioPage(QWidget *parent = nullptr);

public slots:
    void loadInvestments();

signals:
    void addInvestmentRequested();
    void investmentsChanged();

private slots:
    void onSearchChanged(const QString &text);
    void onFilterChanged();
    void onAddClicked();
    void onViewClicked(int investmentId);
    void onEditClicked(int investmentId);
    void onDeleteClicked(int investmentId);

private:
    void setupUi();
    void updateSummaryMetrics(const std::vector<Services::InvestmentListItemDTO> &items);

    SummaryCard *m_totalCard{nullptr};
    SummaryCard *m_equitiesCard{nullptr};
    SummaryCard *m_debtGoldCard{nullptr};

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_holdingsTable{nullptr};

    std::vector<Services::InvestmentListItemDTO> m_currentItems;
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_PORTFOLIOPAGE_H

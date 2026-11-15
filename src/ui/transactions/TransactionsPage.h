#ifndef INVESTSPHERE_TRANSACTIONSPAGE_H
#define INVESTSPHERE_TRANSACTIONSPAGE_H

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT

#include <QWidget>
#include <vector>

#include "SummaryCard.h"
#include "FilterBar.h"
#include "ModernTableWidget.h"
#include "TransactionModel.h"

namespace InvestSphere {
namespace Ui {

/**
 * @brief Comprehensive financial transactions ledger page with audit filters,
 * category breakdowns, live calculation summaries, and CRUD actions.
 */
class TransactionsPage : public QWidget {
    Q_OBJECT

public:
    explicit TransactionsPage(QWidget *parent = nullptr);

public slots:
    void loadTransactions();

signals:
    void recordTransactionRequested();
    void transactionsChanged();

private slots:
    void onSearchChanged(const QString &text);
    void onFilterChanged();
    void onRecordTransactionClicked();
    void onViewTransactionClicked(int transactionId);
    void onEditTransactionClicked(int transactionId);
    void onDeleteTransactionClicked(int transactionId);

private:
    void setupUi();
    void updateSummaryMetrics(const std::vector<Models::Transaction> &items);

    SummaryCard *m_txnCountCard{nullptr};
    SummaryCard *m_inflowCard{nullptr};
    SummaryCard *m_payoutCard{nullptr};

    FilterBar *m_filterBar{nullptr};
    ModernTableWidget *m_txnTable{nullptr};

    std::vector<Models::Transaction> m_currentItems;
};

} // namespace Ui
} // namespace InvestSphere

#endif // INVESTSPHERE_HAS_QT
#endif // INVESTSPHERE_TRANSACTIONSPAGE_H

#ifndef TRANSACTIONREPOSITORY_H
#define TRANSACTIONREPOSITORY_H

#include "TransactionModel.h"
#include "Compat.h"
#include <vector>
#include <string>

namespace InvestSphere {
namespace Database {

class TransactionRepository {
public:
    TransactionRepository() = default;

    InvestSphere::Optional<int> recordTransaction(const Models::Transaction &tx);
    bool updateTransaction(const Models::Transaction &tx);
    bool deleteTransaction(int transactionId, int userId);

    InvestSphere::Optional<Models::Transaction> getTransactionById(int transactionId);
    std::vector<Models::Transaction> getTransactionsByUserId(int userId);
    std::vector<Models::Transaction> getTransactionsByInvestmentId(int investmentId);
    std::vector<Models::Transaction> getTransactionsByDateRange(int userId, const std::string &startDate, const std::string &endDate);
    std::vector<Models::Transaction> getFilteredTransactions(int userId,
                                                            const std::string &searchQuery = "",
                                                            const std::string &typeFilter = "ALL",
                                                            const std::string &startDate = "",
                                                            const std::string &endDate = "",
                                                            const std::string &sortBy = "DATE",
                                                            bool sortAscending = false);
    int countTransactionsByUserId(int userId);

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // TRANSACTIONREPOSITORY_H

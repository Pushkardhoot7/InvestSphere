#ifndef INVESTSPHERE_TRANSACTIONSERVICE_H
#define INVESTSPHERE_TRANSACTIONSERVICE_H

#include "TransactionModel.h"
#include "InvestmentModel.h"
#include "PortfolioCalculationService.h"
#include "Compat.h"

#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace InvestSphere {
namespace Services {

struct TransactionInputDTO {
    int investmentId{0};
    std::string transactionType{"BUY"}; // BUY, SELL, DEPOSIT, WITHDRAWAL, DIVIDEND, INTEREST, FEE, REDEMPTION
    std::string transactionDate;
    double quantity{0.0};
    int64_t unitPriceCents{0};
    int64_t grossAmountCents{0};
    int64_t feesCents{0};
    int64_t taxesCents{0};
    int64_t netAmountCents{0};
    std::string notes;
};

struct UserInvestmentOptionDTO {
    int investmentId{0};
    std::string symbol;
    std::string instrumentName;
    std::string categoryCode;
    std::string categoryName;
    double currentHoldingQuantity{0.0};
    int64_t averageCostPerUnitCents{0};
    std::string status{"ACTIVE"};
};

/**
 * @brief Service coordinating unified financial transaction operations, validations,
 * category suitability checks, holding balance sufficiency, and atomic execution.
 */
class TransactionService {
public:
    static TransactionService& instance();

    /**
     * @brief Validates all transaction fields, business logic, holding sufficiency, and category suitability.
     * @return pair of (isValid, errorMessage)
     */
    std::pair<bool, std::string> validateTransaction(int userId,
                                                    const TransactionInputDTO &input,
                                                    int existingTxId = 0);

    /**
     * @brief Atomically records a validated financial transaction within a SQLite transaction boundary.
     * @return pair of (isSuccess, errorMessageOrNewId)
     */
    std::pair<bool, std::string> recordTransaction(int userId, const TransactionInputDTO &input);

    /**
     * @brief Safely updates an existing transaction while ensuring historical holding non-negativity.
     */
    std::pair<bool, std::string> updateTransaction(int userId,
                                                   int transactionId,
                                                   const TransactionInputDTO &input);

    /**
     * @brief Safely deletes / reverses a transaction if it does not leave subsequent sales in an oversold state.
     */
    std::pair<bool, std::string> deleteTransaction(int userId, int transactionId);

    /**
     * @brief Retrieves transaction details verifying ownership scoping.
     */
    InvestSphere::Optional<Models::Transaction> getTransactionDetail(int userId, int transactionId);

    /**
     * @brief Retrieves filtered and sorted transactions for the user.
     */
    std::vector<Models::Transaction> getTransactions(int userId,
                                                    const std::string &searchQuery = "",
                                                    const std::string &typeFilter = "ALL",
                                                    const std::string &startDate = "",
                                                    const std::string &endDate = "",
                                                    const std::string &sortBy = "DATE",
                                                    bool sortAscending = false);

    /**
     * @brief Retrieves list of user's holdings with live remaining balances for transaction form dropdowns.
     */
    std::vector<UserInvestmentOptionDTO> getUserInvestmentOptions(int userId);
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_TRANSACTIONSERVICE_H

#include "TransactionService.h"
#include "TransactionRepository.h"
#include "InvestmentRepository.h"
#include "DatabaseManager.h"
#include "TransactionScope.h"

#include <cmath>
#include <sstream>

namespace InvestSphere {
namespace Services {

TransactionService& TransactionService::instance()
{
    static TransactionService s_instance;
    return s_instance;
}

std::pair<bool, std::string> TransactionService::validateTransaction(int userId,
                                                                    const TransactionInputDTO &input,
                                                                    int existingTxId)
{
    if (userId <= 0) {
        return {false, "Invalid authenticated user session."};
    }
    if (input.investmentId <= 0) {
        return {false, "Please select an investment holding for this transaction."};
    }
    if (input.transactionDate.empty()) {
        return {false, "Transaction date is required."};
    }

    // 1. Verify user ownership of the investment
    Database::InvestmentRepository invRepo;
    auto invOpt = invRepo.getInvestmentByIdForUser(input.investmentId, userId);
    if (!invOpt) {
        return {false, "Unauthorized: The selected investment does not belong to the current user."};
    }
    const auto &inv = *invOpt;

    // 2. Validate transaction type compatibility with asset class
    const std::string &type = input.transactionType;
    const std::string &cat = inv.assetTypeCode;

    bool typeAllowed = false;
    if (cat == "STOCK") {
        typeAllowed = (type == "BUY" || type == "SELL" || type == "DIVIDEND" || type == "FEE");
    } else if (cat == "MUTUAL_FUND") {
        typeAllowed = (type == "BUY" || type == "REDEMPTION" || type == "DIVIDEND" || type == "FEE");
    } else if (cat == "GOLD") {
        typeAllowed = (type == "BUY" || type == "SELL" || type == "FEE");
    } else if (cat == "FIXED_DEPOSIT") {
        typeAllowed = (type == "DEPOSIT" || type == "WITHDRAWAL" || type == "INTEREST" || type == "FEE");
    } else if (cat == "BOND") {
        typeAllowed = (type == "BUY" || type == "SELL" || type == "INTEREST" || type == "REDEMPTION" || type == "FEE");
    }

    if (!typeAllowed) {
        return {false, "Transaction type '" + type + "' is not supported for " + inv.assetTypeCode + " investments."};
    }

    // 3. Positive quantities & unit prices where applicable
    if (type == "BUY" || type == "SELL" || type == "REDEMPTION") {
        if (input.quantity <= 0.000001) {
            return {false, "Transaction quantity must be strictly greater than zero."};
        }
        if (input.unitPriceCents <= 0) {
            return {false, "Unit price must be strictly greater than zero."};
        }
    } else if (type == "DEPOSIT" || type == "WITHDRAWAL" || type == "DIVIDEND" || type == "INTEREST") {
        if (input.grossAmountCents <= 0) {
            return {false, "Gross transaction amount must be strictly greater than zero."};
        }
    }

    if (input.feesCents < 0) {
        return {false, "Brokerage / transaction fees cannot be negative."};
    }
    if (input.taxesCents < 0) {
        return {false, "Taxes cannot be negative."};
    }
    if (input.netAmountCents < 0) {
        return {false, "Net transaction amount cannot be negative."};
    }

    // 4. Consistent derived amount calculation checks
    if (type == "BUY" || type == "DEPOSIT") {
        int64_t expectedNet = input.grossAmountCents + input.feesCents + input.taxesCents;
        if (std::abs(input.netAmountCents - expectedNet) > 5) {
            return {false, "Inconsistent net amount for purchase: net amount must equal gross amount + fees + taxes."};
        }
    } else if (type == "SELL" || type == "REDEMPTION") {
        if (input.grossAmountCents < (input.feesCents + input.taxesCents)) {
            return {false, "Brokerage fees and taxes cannot exceed the gross sale proceeds."};
        }
        int64_t expectedNet = input.grossAmountCents - input.feesCents - input.taxesCents;
        if (std::abs(input.netAmountCents - expectedNet) > 5) {
            return {false, "Inconsistent net amount for sale: net amount must equal gross proceeds - fees - taxes."};
        }
    } else if (type == "DIVIDEND" || type == "INTEREST") {
        if (input.grossAmountCents < input.taxesCents) {
            return {false, "Taxes cannot exceed the gross dividend/interest income."};
        }
        int64_t expectedNet = input.grossAmountCents - input.feesCents - input.taxesCents;
        if (std::abs(input.netAmountCents - expectedNet) > 5) {
            return {false, "Inconsistent net amount for income: net amount must equal gross income - taxes."};
        }
    }

    // 5. Holding sufficiency check for SELL, REDEMPTION, and WITHDRAWAL
    if (type == "SELL" || type == "REDEMPTION" || type == "WITHDRAWAL") {
        auto check = PortfolioCalculationService::instance().validateHoldingQuantityFeasibility(
            userId,
            input.investmentId,
            type,
            input.quantity,
            input.transactionDate,
            existingTxId
        );
        if (!check.first) {
            return check;
        }
    }

    // 6. Duplicate submission protection (prevent identical submission if already present)
    if (existingTxId == 0) {
        Database::TransactionRepository txnRepo;
        auto existingList = txnRepo.getTransactionsByInvestmentId(input.investmentId);
        for (const auto &tx : existingList) {
            if (tx.transactionType == input.transactionType &&
                tx.transactionDate == input.transactionDate &&
                std::abs(tx.quantity - input.quantity) < 0.0001 &&
                tx.netAmountCents == input.netAmountCents) {
                return {false, "Duplicate transaction detected: an identical " + input.transactionType +
                               " transaction already exists on " + input.transactionDate + "."};
            }
        }
    }

    return {true, ""};
}

std::pair<bool, std::string> TransactionService::recordTransaction(int userId, const TransactionInputDTO &input)
{
    auto validation = validateTransaction(userId, input, 0);
    if (!validation.first) {
        return {false, validation.second};
    }

    Database::TransactionScope scope;

    Models::Transaction tx;
    tx.userId = userId;
    tx.investmentId = input.investmentId;
    tx.transactionType = input.transactionType;
    tx.transactionDate = input.transactionDate;
    tx.quantity = input.quantity;
    tx.unitPriceCents = input.unitPriceCents;
    tx.grossAmountCents = input.grossAmountCents;
    tx.feesCents = input.feesCents;
    tx.taxesCents = input.taxesCents;
    tx.netAmountCents = input.netAmountCents;
    tx.notes = input.notes;

    Database::TransactionRepository txnRepo;
    auto newIdOpt = txnRepo.recordTransaction(tx);
    if (!newIdOpt) {
        scope.rollback();
        return {false, "Failed to record transaction in SQLite ledger: " + txnRepo.getLastError()};
    }

    // Check if remaining quantity became 0, update holding status if needed
    auto posOpt = PortfolioCalculationService::instance().calculateHoldingPosition(userId, input.investmentId);
    if (posOpt && posOpt->remainingQuantity <= 0.00001) {
        Database::InvestmentRepository invRepo;
        invRepo.updateInvestment(input.investmentId, userId, "", "SOLD", "DEFAULT");
    } else if (posOpt && posOpt->remainingQuantity > 0.00001) {
        Database::InvestmentRepository invRepo2;
        invRepo2.updateInvestment(input.investmentId, userId, "", "ACTIVE", "DEFAULT");
    }

    scope.commit();
    return {true, std::to_string(*newIdOpt)};
}

std::pair<bool, std::string> TransactionService::updateTransaction(int userId,
                                                                   int transactionId,
                                                                   const TransactionInputDTO &input)
{
    Database::TransactionRepository txnRepo;
    auto existingOpt = txnRepo.getTransactionById(transactionId);
    if (!existingOpt || existingOpt->userId != userId) {
        return {false, "Unauthorized: Transaction not found or does not belong to the current user."};
    }

    auto validation = validateTransaction(userId, input, transactionId);
    if (!validation.first) {
        return {false, validation.second};
    }

    Database::TransactionScope scope;

    Models::Transaction tx;
    tx.id = transactionId;
    tx.userId = userId;
    tx.investmentId = input.investmentId;
    tx.transactionType = input.transactionType;
    tx.transactionDate = input.transactionDate;
    tx.quantity = input.quantity;
    tx.unitPriceCents = input.unitPriceCents;
    tx.grossAmountCents = input.grossAmountCents;
    tx.feesCents = input.feesCents;
    tx.taxesCents = input.taxesCents;
    tx.netAmountCents = input.netAmountCents;
    tx.notes = input.notes;

    if (!txnRepo.updateTransaction(tx)) {
        scope.rollback();
        return {false, "Failed to update transaction: " + txnRepo.getLastError()};
    }

    scope.commit();
    return {true, "Transaction updated successfully."};
}

std::pair<bool, std::string> TransactionService::deleteTransaction(int userId, int transactionId)
{
    Database::TransactionRepository txnRepo;
    auto txOpt = txnRepo.getTransactionById(transactionId);
    if (!txOpt || txOpt->userId != userId) {
        return {false, "Unauthorized: Transaction not found or does not belong to the current user."};
    }

    const auto &tx = *txOpt;

    // Safety check: if deleting a BUY or DEPOSIT, verify that removing it doesn't cause future sales to oversell!
    if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
        auto allTxs = txnRepo.getTransactionsByInvestmentId(tx.investmentId);
        double running = 0.0;
        for (const auto &t : allTxs) {
            if (t.id == transactionId) continue; // simulate removal
            if (t.transactionType == "BUY" || t.transactionType == "DEPOSIT") {
                running += t.quantity;
            } else if (t.transactionType == "SELL" || t.transactionType == "REDEMPTION" || t.transactionType == "WITHDRAWAL") {
                running -= t.quantity;
                if (running < -0.00001) {
                    return {false, "Cannot delete transaction: subsequent sales/redemptions depend on these units. Deletion would create an oversold balance."};
                }
            }
        }
    }

    Database::TransactionScope scope;
    if (!txnRepo.deleteTransaction(transactionId, userId)) {
        scope.rollback();
        return {false, "Failed to delete transaction: " + txnRepo.getLastError()};
    }

    scope.commit();
    return {true, "Transaction deleted successfully."};
}

InvestSphere::Optional<Models::Transaction> TransactionService::getTransactionDetail(int userId, int transactionId)
{
    Database::TransactionRepository txnRepo;
    auto txOpt = txnRepo.getTransactionById(transactionId);
    if (txOpt && txOpt->userId == userId) {
        return txOpt;
    }
    return {};
}

std::vector<Models::Transaction> TransactionService::getTransactions(int userId,
                                                                     const std::string &searchQuery,
                                                                     const std::string &typeFilter,
                                                                     const std::string &startDate,
                                                                     const std::string &endDate,
                                                                     const std::string &sortBy,
                                                                     bool sortAscending)
{
    Database::TransactionRepository txnRepo;
    return txnRepo.getFilteredTransactions(userId, searchQuery, typeFilter, startDate, endDate, sortBy, sortAscending);
}

std::vector<UserInvestmentOptionDTO> TransactionService::getUserInvestmentOptions(int userId)
{
    std::vector<UserInvestmentOptionDTO> options;
    auto positions = PortfolioCalculationService::instance().calculateHoldingsPositions(userId);

    for (const auto &pos : positions) {
        UserInvestmentOptionDTO opt;
        opt.investmentId = pos.investmentId;
        opt.symbol = pos.symbol;
        opt.instrumentName = pos.name;
        opt.categoryCode = pos.categoryCode;
        opt.categoryName = pos.categoryName;
        opt.currentHoldingQuantity = pos.remainingQuantity;
        opt.averageCostPerUnitCents = pos.averageCostPerUnitCents;
        opt.status = pos.status;
        options.push_back(opt);
    }

    return options;
}

} // namespace Services
} // namespace InvestSphere

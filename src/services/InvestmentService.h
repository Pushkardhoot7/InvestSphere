#ifndef INVESTSPHERE_INVESTMENTSERVICE_H
#define INVESTSPHERE_INVESTMENTSERVICE_H

#include "InvestmentModel.h"
#include "InstrumentModel.h"
#include "TransactionModel.h"
#include "Compat.h"

#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace InvestSphere {
namespace Services {

/**
 * @brief DTO containing all user-submitted fields for creating or updating an investment.
 */
struct InvestmentInputDTO {
    std::string categoryCode;       // 'STOCK', 'MUTUAL_FUND', 'GOLD', 'FIXED_DEPOSIT', 'BOND'
    std::string symbol;             // Ticker, Scheme Code, or unique instrument identifier
    std::string instrumentName;     // Company name, fund name, bond name, etc.
    std::string purchaseDate;       // YYYY-MM-DD
    double quantity{0.0};           // Shares, units, grams, or bond count
    int64_t unitPriceCents{0};      // Purchase price per unit (or face value) in minor units
    int64_t totalAmountCents{0};    // Explicit total / principal amount (for FDs or lumpsum)
    std::string portfolioTag{"DEFAULT"};
    std::string notes;

    // Category-specific: Stocks
    std::string exchange{"NASDAQ"};
    std::string sector;
    std::string industry;
    std::string isin;
    std::string marketCapCategory{"LARGE_CAP"};

    // Category-specific: Mutual Funds
    std::string amcName;
    std::string fundCategory{"EQUITY"};
    std::string planType{"GROWTH"};
    std::string benchmarkIndex;
    double expenseRatio{0.0};

    // Category-specific: Gold
    std::string goldForm{"PHYSICAL_COIN"}; // 'PHYSICAL_COIN', 'PHYSICAL_BAR', 'DIGITAL_GOLD', 'SOVEREIGN_BOND', 'GOLD_ETF'
    double purityKarat{24.0};
    std::string storageLocation;

    // Category-specific: Fixed Deposits
    std::string institutionName;
    std::string accountNumber;
    int64_t principalAmountCents{0};
    double annualInterestRate{0.0};
    std::string compoundingFrequency{"QUARTERLY"};
    std::string payoutType{"CUMULATIVE"};
    std::string startDate;
    std::string maturityDate;
    int durationMonths{12};
    int64_t estimatedMaturityAmountCents{0};

    // Category-specific: Bonds
    std::string issuerName;
    std::string bondType{"GOVERNMENT"};
    int64_t faceValueCents{0};
    double couponRate{0.0};
    std::string couponFrequency{"SEMI_ANNUALLY"};
    std::string issueDate;
    std::string bondMaturityDate;
    std::string creditRating{"AAA"};
};

/**
 * @brief Representation of an investment in tabular lists with calculated valuation & P&L.
 */
struct InvestmentListItemDTO {
    int investmentId{0};
    int instrumentId{0};
    std::string symbol;
    std::string name;
    std::string categoryCode;       // 'STOCK', 'MUTUAL_FUND', 'GOLD', 'FIXED_DEPOSIT', 'BOND'
    std::string categoryName;
    std::string status{"ACTIVE"};
    std::string portfolioTag{"DEFAULT"};
    std::string purchaseDate;
    std::string notes;
    
    double quantity{0.0};
    int64_t investedAmountCents{0};
    
    // Valuation fields (strictly non-fabricated)
    bool hasMarketPrice{false};
    int64_t latestPriceCents{0};
    bool hasCurrentValue{false};
    int64_t currentValueCents{0};
    bool hasPnl{false};
    int64_t unrealizedPnlCents{0};
    double unrealizedPnlPercent{0.0};
};

/**
 * @brief Comprehensive view DTO with full category specialization and transaction ledger history.
 */
struct InvestmentDetailDTO {
    InvestmentListItemDTO summary;
    
    // Specialization details
    InvestSphere::Optional<Models::StockDetails> stockDetails;
    InvestSphere::Optional<Models::MutualFundDetails> mfDetails;
    InvestSphere::Optional<Models::GoldHoldingDetails> goldDetails;
    InvestSphere::Optional<Models::FixedDepositDetails> fdDetails;
    InvestSphere::Optional<Models::BondDetails> bondDetails;
    
    // Linked transactions ledger
    std::vector<Models::Transaction> transactions;
};

/**
 * @brief Service coordinating multi-asset investment operations, input validations,
 * relational mapping, transaction consistency, and user scoping.
 */
class InvestmentService {
public:
    static InvestmentService& instance();

    /**
     * @brief Validates all required and category-specific inputs.
     * @return pair of (isValid, errorMessage)
     */
    std::pair<bool, std::string> validateInput(const InvestmentInputDTO &input);

    /**
     * @brief Atomically creates an instrument (if needed), user holding, specialization,
     * initial transaction ledger record, and cash flow events.
     * @return pair of (isSuccess, errorMessageOrSuccessId)
     */
    std::pair<bool, std::string> addInvestment(int userId, const InvestmentInputDTO &input);

    /**
     * @brief Retrieves active or filtered investments for the given authenticated user.
     * Calculates live cost basis, remaining quantity, and valuation.
     */
    std::vector<InvestmentListItemDTO> getUserInvestments(int userId,
                                                         const std::string &searchQuery = "",
                                                         const std::string &categoryFilter = "ALL",
                                                         const std::string &statusFilter = "ACTIVE",
                                                         const std::string &sortBy = "NAME",
                                                         bool sortAscending = true);

    /**
     * @brief Retrieves detailed investment attributes, specialization data, and transaction records.
     */
    InvestSphere::Optional<InvestmentDetailDTO> getInvestmentDetail(int userId, int investmentId);

    /**
     * @brief Updates user holding metadata (notes, status, portfolio tag).
     */
    std::pair<bool, std::string> updateInvestment(int userId,
                                                 int investmentId,
                                                 const std::string &notes,
                                                 const std::string &status,
                                                 const std::string &portfolioTag);

    /**
     * @brief Counts linked transaction records for an investment.
     */
    int getTransactionCount(int userId, int investmentId);

    /**
     * @brief Safely deletes an investment holding.
     * @param cascadeTransactions If false, rejects deletion if transaction records exist (RESTRICT policy).
     *                            If true, atomically deletes dependent events, specialization, and transactions.
     */
    std::pair<bool, std::string> deleteInvestment(int userId, int investmentId, bool cascadeTransactions = false);

private:
    InvestmentService() = default;
    ~InvestmentService() = default;
    InvestmentService(const InvestmentService&) = delete;
    InvestmentService& operator=(const InvestmentService&) = delete;

    int getTypeIdForCategory(const std::string &catCode) const;
    std::string getCategoryName(const std::string &catCode) const;
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_INVESTMENTSERVICE_H

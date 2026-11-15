#include "PortfolioCalculationService.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"
#include "CostBasisEngine.h"

#include <algorithm>
#include <cmath>

namespace InvestSphere {
namespace Services {

PortfolioCalculationService& PortfolioCalculationService::instance()
{
    static PortfolioCalculationService s_instance;
    return s_instance;
}

std::vector<HoldingPositionDTO> PortfolioCalculationService::calculateHoldingsPositions(int userId)
{
    std::vector<HoldingPositionDTO> results;
    Database::InvestmentRepository invRepo;
    auto investments = invRepo.getInvestmentsByUserId(userId);

    for (const auto &inv : investments) {
        auto posOpt = calculateHoldingPosition(userId, inv.id);
        if (posOpt) {
            results.push_back(*posOpt);
        }
    }

    return results;
}

InvestSphere::Optional<HoldingPositionDTO> PortfolioCalculationService::calculateHoldingPosition(int userId, int investmentId)
{
    Database::InvestmentRepository invRepo;
    auto invOpt = invRepo.getInvestmentByIdForUser(investmentId, userId);
    if (!invOpt) {
        return {};
    }

    const auto &inv = *invOpt;
    Database::TransactionRepository txnRepo;
    auto txs = txnRepo.getTransactionsByInvestmentId(investmentId);

    // Compute Cost Basis & P&L via CostBasisEngine
    auto cb = Finance::CostBasisEngine::calculateCostBasis(txs);

    HoldingPositionDTO pos;
    pos.investmentId = inv.id;
    pos.instrumentId = inv.instrumentId;
    pos.symbol = inv.symbol;
    pos.name = inv.instrumentName;
    pos.categoryCode = inv.assetTypeCode;
    // Derive human-readable category name from code (Investment model only has assetTypeCode)
    auto categoryNameFromCode = [](const std::string &code) -> std::string {
        if (code == "STOCK")         return "Stocks";
        if (code == "MUTUAL_FUND")   return "Mutual Funds";
        if (code == "GOLD")          return "Gold";
        if (code == "FIXED_DEPOSIT") return "Fixed Deposits";
        if (code == "BOND")          return "Bonds";
        return code;
    };
    pos.categoryName = categoryNameFromCode(inv.assetTypeCode);

    pos.status = inv.status;
    pos.portfolioTag = inv.portfolioTag;

    pos.remainingQuantity = cb.remainingQuantity;
    pos.costBasisCents = cb.totalInvestedCostCents;
    pos.averageCostPerUnitCents = cb.averageCostPerUnitCents;
    pos.realizedPnlCents = cb.totalRealizedPnlCents;
    pos.dividendsCents = cb.totalDividendsCents;
    pos.interestCents = cb.totalInterestCents;
    pos.totalIncomeCents = cb.totalIncomeCents;
    pos.feesCents = cb.totalFeesPaidCents;
    pos.taxesCents = cb.totalTaxesPaidCents;
    pos.transactionCount = cb.transactionCount;

    // Fetch market valuation quote (strictly non-fabricated)
    if (inv.assetTypeCode == "MUTUAL_FUND") {
        auto navOpt = invRepo.getLatestMutualFundNav(inv.instrumentId);
        if (navOpt && *navOpt > 0.0) {
            pos.hasMarketQuote = true;
            pos.currentUnitPriceCents = static_cast<int64_t>(std::round(*navOpt * 100.0));
        }
    } else {
        auto priceOpt = invRepo.getLatestMarketPrice(inv.instrumentId);
        if (priceOpt && *priceOpt > 0) {
            pos.hasMarketQuote = true;
            pos.currentUnitPriceCents = *priceOpt;
        }
    }

    if (pos.hasMarketQuote && pos.remainingQuantity > 0.00001) {
        pos.hasMarketValue = true;
        pos.hasPnl = true;
        Finance::CostBasisEngine::calculateUnrealizedPnl(
            pos.remainingQuantity,
            pos.costBasisCents,
            pos.currentUnitPriceCents,
            pos.marketValueCents,
            pos.unrealizedPnlCents,
            pos.unrealizedPnlPercent
        );
    } else {
        pos.hasMarketValue = false;
        pos.hasPnl = false;
        pos.marketValueCents = 0;
        pos.unrealizedPnlCents = 0;
        pos.unrealizedPnlPercent = 0.0;
    }

    return pos;
}

PortfolioSummaryDTO PortfolioCalculationService::calculatePortfolioSummary(int userId)
{
    PortfolioSummaryDTO summary;
    auto positions = calculateHoldingsPositions(userId);

    int64_t quotedCostBasisCents = 0;
    Database::TransactionRepository txnRepo;
    auto allUserTx = txnRepo.getTransactionsByUserId(userId);
    auto allTxCb = Finance::CostBasisEngine::calculateCostBasis(allUserTx);

    summary.cumulativePurchaseTurnoverCents = allTxCb.cumulativePurchaseTurnoverCents;
    summary.netExternalCashFlowCents = allTxCb.netExternalCashFlowCents;
    summary.totalFeesPaidCents = allTxCb.totalFeesPaidCents;
    summary.totalTaxesPaidCents = allTxCb.totalTaxesPaidCents;
    summary.totalRealizedPnlCents = allTxCb.totalRealizedPnlCents;
    summary.totalDividendsCents = allTxCb.totalDividendsCents;
    summary.totalInterestCents = allTxCb.totalInterestCents;
    summary.totalIncomeCents = allTxCb.totalIncomeCents;

    for (const auto &pos : positions) {
        if (pos.status == "ACTIVE" && pos.remainingQuantity > 0.00001) {
            summary.activeHoldingsCount++;
            summary.totalInvestedCostBasisCents += pos.costBasisCents;

            if (pos.hasMarketValue) {
                summary.quotedHoldingsCount++;
                summary.totalCurrentEstimatedValueCents += pos.marketValueCents;
                quotedCostBasisCents += pos.costBasisCents;
            } else {
                summary.unquotedHoldingsCount++;
            }
        }
    }

    if (summary.activeHoldingsCount > 0) {
        summary.valuationCoveragePercent = (static_cast<double>(summary.quotedHoldingsCount) / summary.activeHoldingsCount) * 100.0;
        summary.hasValuationData = (summary.quotedHoldingsCount > 0);
        summary.isValuationComplete = (summary.unquotedHoldingsCount == 0);
    } else {
        summary.valuationCoveragePercent = 100.0;
        summary.hasValuationData = false;
        summary.isValuationComplete = true;
    }

    // Unrealized P&L = Current Value of Quoted Holdings - Cost Basis of Quoted Holdings
    if (summary.hasValuationData && quotedCostBasisCents > 0) {
        summary.totalUnrealizedPnlCents = summary.totalCurrentEstimatedValueCents - quotedCostBasisCents;
        summary.unrealizedPnlPercent = (static_cast<double>(summary.totalUnrealizedPnlCents) / quotedCostBasisCents) * 100.0;
    } else {
        summary.totalUnrealizedPnlCents = 0;
        summary.unrealizedPnlPercent = 0.0;
    }

    return summary;
}

std::pair<bool, std::string> PortfolioCalculationService::validateHoldingQuantityFeasibility(
    int userId,
    int investmentId,
    const std::string &proposedType,
    double proposedQty,
    const std::string &proposedDate,
    int ignoreTxId)
{
    (void)userId;
    if (proposedType != "SELL" && proposedType != "REDEMPTION" && proposedType != "WITHDRAWAL") {
        return {true, ""};
    }

    if (proposedQty <= 0.000001) {
        return {false, "Quantity to sell/redeem must be strictly greater than zero."};
    }

    Database::TransactionRepository txnRepo;
    auto txs = txnRepo.getTransactionsByInvestmentId(investmentId);

    // Filter out ignoreTxId if updating
    std::vector<Models::Transaction> simTxs;
    for (const auto &tx : txs) {
        if (tx.id != ignoreTxId) {
            simTxs.push_back(tx);
        }
    }

    // Insert the proposed transaction
    Models::Transaction prop;
    prop.id = 999999999;
    prop.investmentId = investmentId;
    prop.transactionType = proposedType;
    prop.quantity = proposedQty;
    prop.transactionDate = proposedDate.empty() ? "9999-12-31" : proposedDate;
    simTxs.push_back(prop);

    // Sort chronologically
    std::sort(simTxs.begin(), simTxs.end(), [](const Models::Transaction &a, const Models::Transaction &b) {
        if (a.transactionDate != b.transactionDate) {
            return a.transactionDate < b.transactionDate;
        }
        return a.id < b.id;
    });

    double runningQty = 0.0;
    for (const auto &tx : simTxs) {
        if (tx.transactionType == "BUY" || tx.transactionType == "DEPOSIT") {
            runningQty += tx.quantity;
        } else if (tx.transactionType == "SELL" || tx.transactionType == "REDEMPTION" || tx.transactionType == "WITHDRAWAL") {
            runningQty -= tx.quantity;
            if (runningQty < -0.00001) {
                double deficit = -runningQty;
                return {false, "Insufficient holdings: selling " + std::to_string(proposedQty) +
                               " units would create an oversold balance deficit of " + std::to_string(deficit) + " units."};
            }
        }
    }

    return {true, ""};
}

} // namespace Services
} // namespace InvestSphere

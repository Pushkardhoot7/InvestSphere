#include "InvestmentService.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"
#include "FinancialEventRepository.h"
#include "CostBasisEngine.h"
#include "CompoundingEngine.h"
#include "TransactionScope.h"

#include <algorithm>
#include <sstream>
#include <cmath>
#include <cctype>

namespace InvestSphere {
namespace Services {

InvestmentService& InvestmentService::instance()
{
    static InvestmentService s_instance;
    return s_instance;
}

int InvestmentService::getTypeIdForCategory(const std::string &catCode) const
{
    if (catCode == "STOCK") return 1;
    if (catCode == "MUTUAL_FUND") return 2;
    if (catCode == "GOLD") return 3;
    if (catCode == "FIXED_DEPOSIT") return 4;
    if (catCode == "BOND") return 5;
    if (catCode == "CASH") return 6;
    return 1;
}

std::string InvestmentService::getCategoryName(const std::string &catCode) const
{
    if (catCode == "STOCK") return "Equity Stocks";
    if (catCode == "MUTUAL_FUND") return "Mutual Funds";
    if (catCode == "GOLD") return "Gold & Precious Metals";
    if (catCode == "FIXED_DEPOSIT") return "Fixed Deposits";
    if (catCode == "BOND") return "Bonds & Debentures";
    if (catCode == "CASH") return "Cash & Savings";
    return catCode;
}

std::pair<bool, std::string> InvestmentService::validateInput(const InvestmentInputDTO &input)
{
    if (input.categoryCode.empty()) {
        return {false, "Investment category must be specified."};
    }
    if (input.instrumentName.empty()) {
        return {false, "Investment name cannot be blank."};
    }
    if (input.symbol.empty()) {
        return {false, "Symbol or scheme identifier cannot be blank."};
    }
    if (input.purchaseDate.empty()) {
        return {false, "Acquisition/purchase date cannot be blank."};
    }

    if (input.categoryCode == "STOCK") {
        if (input.quantity <= 0.0) {
            return {false, "Number of stock shares must be greater than zero."};
        }
        if (input.unitPriceCents <= 0) {
            return {false, "Purchase price per share must be positive."};
        }
    } else if (input.categoryCode == "MUTUAL_FUND") {
        if (input.quantity <= 0.0) {
            return {false, "Mutual fund units must be greater than zero."};
        }
        if (input.unitPriceCents <= 0) {
            return {false, "Purchase NAV must be positive."};
        }
        if (input.expenseRatio < 0.0 || input.expenseRatio > 10.0) {
            return {false, "Expense ratio must be between 0.0% and 10.0%."};
        }
    } else if (input.categoryCode == "GOLD") {
        if (input.quantity <= 0.0) {
            return {false, "Gold weight (grams/units) must be greater than zero."};
        }
        if (input.unitPriceCents <= 0 && input.totalAmountCents <= 0) {
            return {false, "Gold purchase cost must be positive."};
        }
        if (input.purityKarat <= 0.0 || input.purityKarat > 24.0) {
            return {false, "Gold purity must be between 1.0 and 24.0 Karats."};
        }
    } else if (input.categoryCode == "FIXED_DEPOSIT") {
        int64_t principal = input.principalAmountCents > 0 ? input.principalAmountCents : input.totalAmountCents;
        if (principal <= 0) {
            return {false, "Fixed Deposit principal amount must be positive."};
        }
        if (input.annualInterestRate < 0.0 || input.annualInterestRate > 100.0) {
            return {false, "Annual interest rate must be between 0.0% and 100.0%."};
        }
        if (input.institutionName.empty()) {
            return {false, "Issuing bank or financial institution name is required."};
        }
        if (input.maturityDate.empty()) {
            return {false, "Fixed Deposit maturity date is required."};
        }
        if (input.startDate.empty() && input.purchaseDate.empty()) {
            return {false, "Fixed Deposit start date is required."};
        }
        std::string sDate = input.startDate.empty() ? input.purchaseDate : input.startDate;
        if (input.maturityDate <= sDate) {
            return {false, "Maturity date must be after deposit start date."};
        }
    } else if (input.categoryCode == "BOND") {
        int64_t faceVal = input.faceValueCents > 0 ? input.faceValueCents : input.unitPriceCents;
        if (faceVal <= 0) {
            return {false, "Bond face value must be positive."};
        }
        if (input.quantity <= 0.0) {
            return {false, "Bond quantity/units must be greater than zero."};
        }
        if (input.couponRate < 0.0 || input.couponRate > 100.0) {
            return {false, "Coupon rate must be between 0.0% and 100.0%."};
        }
        if (input.issuerName.empty()) {
            return {false, "Bond issuing authority/entity is required."};
        }
        std::string iDate = input.issueDate.empty() ? input.purchaseDate : input.issueDate;
        std::string mDate = input.bondMaturityDate.empty() ? input.maturityDate : input.bondMaturityDate;
        if (!mDate.empty() && !iDate.empty() && mDate <= iDate) {
            return {false, "Bond maturity date must be after issue date."};
        }
    }

    return {true, ""};
}

std::pair<bool, std::string> InvestmentService::addInvestment(int userId, const InvestmentInputDTO &input)
{
    auto validation = validateInput(input);
    if (!validation.first) {
        return validation;
    }

    Database::InvestmentRepository invRepo;
    Database::TransactionRepository txnRepo;
    Database::FinancialEventRepository eventRepo;

    Database::TransactionScope tx;

    // 1. Resolve or create instrument in shared catalog
    int instrumentId = 0;
    auto existingInst = invRepo.getInstrumentBySymbol(input.symbol);
    if (existingInst) {
        instrumentId = existingInst->id;
    } else {
        Models::Instrument inst;
        inst.investmentTypeId = getTypeIdForCategory(input.categoryCode);
        inst.symbol = input.symbol;
        inst.instrumentName = input.instrumentName;
        inst.currency = "USD";
        inst.exchange = input.exchange;
        inst.description = input.notes;
        inst.isActive = true;

        auto newInstId = invRepo.addInstrument(inst);
        if (!newInstId) {
            return {false, "Failed to register instrument: " + invRepo.getLastError()};
        }
        instrumentId = *newInstId;

        // Create instrument specialization if applicable
        if (input.categoryCode == "STOCK") {
            Models::StockDetails sd;
            sd.instrumentId = instrumentId;
            sd.isin = input.isin;
            sd.sector = input.sector;
            sd.industry = input.industry;
            sd.marketCapCategory = input.marketCapCategory.empty() ? "LARGE_CAP" : input.marketCapCategory;
            invRepo.addStockDetails(sd);
        } else if (input.categoryCode == "MUTUAL_FUND") {
            Models::MutualFundDetails mfd;
            mfd.instrumentId = instrumentId;
            mfd.amcName = input.amcName;
            mfd.fundCategory = input.fundCategory.empty() ? "EQUITY" : input.fundCategory;
            mfd.planType = input.planType.empty() ? "GROWTH" : input.planType;
            mfd.benchmarkIndex = input.benchmarkIndex;
            mfd.expenseRatio = input.expenseRatio;
            invRepo.addMutualFundDetails(mfd);
        }
    }

    // 2. Create holding record in investments table
    Models::Investment holding;
    holding.userId = userId;
    holding.instrumentId = instrumentId;
    holding.portfolioTag = input.portfolioTag.empty() ? "DEFAULT" : input.portfolioTag;
    holding.notes = input.notes;
    holding.status = "ACTIVE";

    auto newHoldingId = invRepo.createInvestment(holding);
    if (!newHoldingId) {
        return {false, "Failed to create investment holding: " + invRepo.getLastError()};
    }
    int investmentId = *newHoldingId;

    // 3. Create holding specialization if applicable
    if (input.categoryCode == "GOLD") {
        Models::GoldHoldingDetails gd;
        gd.investmentId = investmentId;
        gd.goldForm = input.goldForm.empty() ? "PHYSICAL_COIN" : input.goldForm;
        gd.purityKarat = input.purityKarat > 0.0 ? input.purityKarat : 24.0;
        gd.storageLocation = input.storageLocation;
        invRepo.addGoldDetails(gd);
    } else if (input.categoryCode == "FIXED_DEPOSIT") {
        int64_t principal = input.principalAmountCents > 0 ? input.principalAmountCents : input.totalAmountCents;
        int64_t estMaturity = input.estimatedMaturityAmountCents;
        if (estMaturity <= 0) {
            estMaturity = Finance::CompoundingEngine::calculateMaturityAmount(
                principal, input.annualInterestRate, input.durationMonths, input.compoundingFrequency, input.payoutType
            );
        }

        Models::FixedDepositDetails fd;
        fd.investmentId = investmentId;
        fd.institutionName = input.institutionName;
        fd.accountNumber = input.accountNumber;
        fd.principalAmountCents = principal;
        fd.annualInterestRate = input.annualInterestRate;
        fd.compoundingFrequency = input.compoundingFrequency.empty() ? "QUARTERLY" : input.compoundingFrequency;
        fd.payoutType = input.payoutType.empty() ? "CUMULATIVE" : input.payoutType;
        fd.startDate = input.startDate.empty() ? input.purchaseDate : input.startDate;
        fd.maturityDate = input.maturityDate;
        fd.estimatedMaturityAmountCents = estMaturity;
        fd.status = "ACTIVE";
        invRepo.addFixedDepositDetails(fd);

        // Register upcoming maturity financial event
        Models::FinancialEvent ev;
        ev.userId = userId;
        ev.investmentId = investmentId;
        ev.eventType = "FD_MATURITY";
        ev.eventDate = input.maturityDate;
        ev.amountCents = estMaturity;
        ev.status = "PENDING";
        ev.description = input.institutionName + " Fixed Deposit Maturity Proceeds";
        eventRepo.createEvent(ev);

    } else if (input.categoryCode == "BOND") {
        int64_t faceVal = input.faceValueCents > 0 ? input.faceValueCents : input.unitPriceCents;
        std::string iDate = input.issueDate.empty() ? input.purchaseDate : input.issueDate;
        std::string mDate = input.bondMaturityDate.empty() ? input.maturityDate : input.bondMaturityDate;

        Models::BondDetails bd;
        bd.investmentId = investmentId;
        bd.issuerName = input.issuerName;
        bd.bondType = input.bondType.empty() ? "GOVERNMENT" : input.bondType;
        bd.faceValueCents = faceVal;
        bd.couponRate = input.couponRate;
        bd.couponFrequency = input.couponFrequency.empty() ? "SEMI_ANNUALLY" : input.couponFrequency;
        bd.issueDate = iDate;
        bd.maturityDate = mDate;
        bd.creditRating = input.creditRating.empty() ? "AAA" : input.creditRating;
        bd.status = "ACTIVE";
        invRepo.addBondDetails(bd);

        // Register upcoming maturity financial event
        if (!mDate.empty()) {
            Models::FinancialEvent ev;
            ev.userId = userId;
            ev.investmentId = investmentId;
            ev.eventType = "BOND_MATURITY";
            ev.eventDate = mDate;
            ev.amountCents = static_cast<int64_t>(faceVal * input.quantity);
            ev.status = "PENDING";
            ev.description = input.instrumentName + " Bond Redemption / Maturity";
            eventRepo.createEvent(ev);
        }
    }

    // 4. Record initial purchase / deposit transaction in ledger
    Models::Transaction initialTx;
    initialTx.userId = userId;
    initialTx.investmentId = investmentId;
    initialTx.transactionDate = input.purchaseDate.empty() ? "2026-10-27 12:00:00" : input.purchaseDate;

    if (input.categoryCode == "FIXED_DEPOSIT") {
        initialTx.transactionType = "DEPOSIT";
        initialTx.quantity = 1.0;
        int64_t principal = input.principalAmountCents > 0 ? input.principalAmountCents : input.totalAmountCents;
        initialTx.unitPriceCents = principal;
        initialTx.grossAmountCents = principal;
        initialTx.feesCents = 0;
        initialTx.taxesCents = 0;
        initialTx.netAmountCents = principal;
        initialTx.notes = "Initial term deposit funding: " + input.institutionName;
    } else {
        initialTx.transactionType = "BUY";
        initialTx.quantity = input.quantity;
        int64_t unitPrice = input.unitPriceCents;
        int64_t gross = (input.totalAmountCents > 0)
            ? input.totalAmountCents
            : static_cast<int64_t>(std::round(input.quantity * static_cast<double>(unitPrice)));
        initialTx.unitPriceCents = unitPrice;
        initialTx.grossAmountCents = gross;
        initialTx.feesCents = 0;
        initialTx.taxesCents = 0;
        initialTx.netAmountCents = gross;
        initialTx.notes = "Initial purchase acquisition";
    }

    auto txId = txnRepo.recordTransaction(initialTx);
    if (!txId) {
        return {false, "Failed to record purchase transaction: " + txnRepo.getLastError()};
    }

    if (!tx.commit()) {
        return {false, "Atomic transaction commit failed."};
    }

    return {true, std::to_string(investmentId)};
}

std::vector<InvestmentListItemDTO> InvestmentService::getUserInvestments(int userId,
                                                                       const std::string &searchQuery,
                                                                       const std::string &categoryFilter,
                                                                       const std::string &statusFilter,
                                                                       const std::string &sortBy,
                                                                       bool sortAscending)
{
    Database::InvestmentRepository invRepo;
    Database::TransactionRepository txnRepo;

    auto rawInvestments = invRepo.getInvestmentsByUserId(userId, statusFilter);
    std::vector<InvestmentListItemDTO> results;

    std::string lowerQuery = searchQuery;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), [](unsigned char c) { return std::tolower(c); });

    for (const auto &raw : rawInvestments) {
        // Category filtering
        if (categoryFilter != "ALL" && !categoryFilter.empty() && raw.assetTypeCode != categoryFilter) {
            continue;
        }

        // Live Cost Basis and Quantity from transactions
        auto txList = txnRepo.getTransactionsByInvestmentId(raw.id);
        auto costBasis = Finance::CostBasisEngine::calculateCostBasis(txList);

        double holdingQty = costBasis.remainingQuantity;
        int64_t investedCost = costBasis.totalInvestedCostCents;

        // If no transactions found yet (or raw contains values), fallback to raw values
        if (txList.empty() && raw.remainingQuantity > 0.0) {
            holdingQty = raw.remainingQuantity;
            investedCost = raw.netInvestedAmountCents;
        }

        // If filtering for ACTIVE holdings, skip zero positions if closed
        if (statusFilter == "ACTIVE" && holdingQty <= 0.00001 && raw.assetTypeCode != "FIXED_DEPOSIT") {
            continue;
        }

        // String matching for search
        if (!lowerQuery.empty()) {
            std::string sym = raw.symbol;
            std::string name = raw.instrumentName;
            std::string notes = raw.notes;
            std::transform(sym.begin(), sym.end(), sym.begin(), [](unsigned char c) { return std::tolower(c); });
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
            std::transform(notes.begin(), notes.end(), notes.begin(), [](unsigned char c) { return std::tolower(c); });

            if (sym.find(lowerQuery) == std::string::npos &&
                name.find(lowerQuery) == std::string::npos &&
                notes.find(lowerQuery) == std::string::npos) {
                continue;
            }
        }

        InvestmentListItemDTO item;
        item.investmentId = raw.id;
        item.instrumentId = raw.instrumentId;
        item.symbol = raw.symbol;
        item.name = raw.instrumentName;
        item.categoryCode = raw.assetTypeCode;
        item.categoryName = getCategoryName(raw.assetTypeCode);
        item.status = raw.status;
        item.portfolioTag = raw.portfolioTag;
        item.purchaseDate = !txList.empty() ? txList.front().transactionDate : raw.createdAt;
        item.notes = raw.notes;
        item.quantity = holdingQty;
        item.investedAmountCents = investedCost;

        // Valuation determination
        if (raw.assetTypeCode == "STOCK" || raw.assetTypeCode == "GOLD") {
            auto latestPrice = invRepo.getLatestMarketPrice(raw.instrumentId);
            if (latestPrice && *latestPrice > 0) {
                item.hasMarketPrice = true;
                item.latestPriceCents = *latestPrice;
                int64_t curVal = 0, pnl = 0;
                double pnlPct = 0.0;
                if (Finance::CostBasisEngine::calculateUnrealizedPnl(holdingQty, investedCost, *latestPrice, curVal, pnl, pnlPct)) {
                    item.hasCurrentValue = true;
                    item.currentValueCents = curVal;
                    item.hasPnl = true;
                    item.unrealizedPnlCents = pnl;
                    item.unrealizedPnlPercent = pnlPct;
                }
            }
        } else if (raw.assetTypeCode == "MUTUAL_FUND") {
            auto latestNav = invRepo.getLatestMutualFundNav(raw.instrumentId);
            if (latestNav && *latestNav > 0.0) {
                int64_t navCents = static_cast<int64_t>(std::round(*latestNav * 100.0));
                item.hasMarketPrice = true;
                item.latestPriceCents = navCents;
                int64_t curVal = static_cast<int64_t>(std::round(holdingQty * (*latestNav) * 100.0));
                item.hasCurrentValue = true;
                item.currentValueCents = curVal;
                item.hasPnl = true;
                item.unrealizedPnlCents = curVal - investedCost;
                if (investedCost > 0) {
                    item.unrealizedPnlPercent = (static_cast<double>(item.unrealizedPnlCents) / static_cast<double>(investedCost)) * 100.0;
                }
            }
        } else if (raw.assetTypeCode == "FIXED_DEPOSIT") {
            auto fd = invRepo.getFixedDepositDetails(raw.id);
            if (fd) {
                item.hasMarketPrice = true;
                item.latestPriceCents = fd->principalAmountCents;
                item.hasCurrentValue = true;
                item.currentValueCents = fd->estimatedMaturityAmountCents > 0 ? fd->estimatedMaturityAmountCents : fd->principalAmountCents;
                item.hasPnl = true;
                item.unrealizedPnlCents = item.currentValueCents - investedCost;
                if (investedCost > 0) {
                    item.unrealizedPnlPercent = (static_cast<double>(item.unrealizedPnlCents) / static_cast<double>(investedCost)) * 100.0;
                }
            } else {
                item.hasCurrentValue = true;
                item.currentValueCents = investedCost;
            }
        } else if (raw.assetTypeCode == "BOND") {
            auto bd = invRepo.getBondDetails(raw.id);
            if (bd) {
                item.hasMarketPrice = true;
                item.latestPriceCents = bd->faceValueCents;
                item.hasCurrentValue = true;
                item.currentValueCents = static_cast<int64_t>(std::round(bd->faceValueCents * holdingQty));
                item.hasPnl = true;
                item.unrealizedPnlCents = item.currentValueCents - investedCost;
                if (investedCost > 0) {
                    item.unrealizedPnlPercent = (static_cast<double>(item.unrealizedPnlCents) / static_cast<double>(investedCost)) * 100.0;
                }
            }
        }

        results.push_back(item);
    }

    // Sort results
    std::sort(results.begin(), results.end(), [sortBy, sortAscending](const InvestmentListItemDTO &a, const InvestmentListItemDTO &b) {
        if (sortBy == "NAME") {
            return sortAscending ? (a.name < b.name) : (a.name > b.name);
        } else if (sortBy == "DATE") {
            return sortAscending ? (a.purchaseDate < b.purchaseDate) : (a.purchaseDate > b.purchaseDate);
        } else if (sortBy == "INVESTED") {
            return sortAscending ? (a.investedAmountCents < b.investedAmountCents) : (a.investedAmountCents > b.investedAmountCents);
        } else if (sortBy == "VALUE") {
            int64_t valA = a.hasCurrentValue ? a.currentValueCents : a.investedAmountCents;
            int64_t valB = b.hasCurrentValue ? b.currentValueCents : b.investedAmountCents;
            return sortAscending ? (valA < valB) : (valA > valB);
        }
        return sortAscending ? (a.name < b.name) : (a.name > b.name);
    });

    return results;
}

InvestSphere::Optional<InvestmentDetailDTO> InvestmentService::getInvestmentDetail(int userId, int investmentId)
{
    Database::InvestmentRepository invRepo;
    Database::TransactionRepository txnRepo;

    auto holding = invRepo.getInvestmentByIdForUser(investmentId, userId);
    if (!holding) {
        return {};
    }

    InvestmentDetailDTO detail;
    detail.summary.investmentId = holding->id;
    detail.summary.instrumentId = holding->instrumentId;
    detail.summary.symbol = holding->symbol;
    detail.summary.name = holding->instrumentName;
    detail.summary.categoryCode = holding->assetTypeCode;
    detail.summary.categoryName = getCategoryName(holding->assetTypeCode);
    detail.summary.status = holding->status;
    detail.summary.portfolioTag = holding->portfolioTag;
    detail.summary.notes = holding->notes;

    // Fetch linked transactions
    detail.transactions = txnRepo.getTransactionsByInvestmentId(investmentId);
    if (!detail.transactions.empty()) {
        detail.summary.purchaseDate = detail.transactions.front().transactionDate;
    } else {
        detail.summary.purchaseDate = holding->createdAt;
    }

    auto costBasis = Finance::CostBasisEngine::calculateCostBasis(detail.transactions);
    detail.summary.quantity = costBasis.remainingQuantity;
    detail.summary.investedAmountCents = costBasis.totalInvestedCostCents;

    // Specializations
    if (holding->assetTypeCode == "STOCK") {
        detail.stockDetails = invRepo.getStockDetails(holding->instrumentId);
        auto latestPrice = invRepo.getLatestMarketPrice(holding->instrumentId);
        if (latestPrice && *latestPrice > 0) {
            detail.summary.hasMarketPrice = true;
            detail.summary.latestPriceCents = *latestPrice;
            int64_t curVal = 0, pnl = 0;
            double pnlPct = 0.0;
            if (Finance::CostBasisEngine::calculateUnrealizedPnl(detail.summary.quantity, detail.summary.investedAmountCents, *latestPrice, curVal, pnl, pnlPct)) {
                detail.summary.hasCurrentValue = true;
                detail.summary.currentValueCents = curVal;
                detail.summary.hasPnl = true;
                detail.summary.unrealizedPnlCents = pnl;
                detail.summary.unrealizedPnlPercent = pnlPct;
            }
        }
    } else if (holding->assetTypeCode == "MUTUAL_FUND") {
        detail.mfDetails = invRepo.getMutualFundDetails(holding->instrumentId);
        auto latestNav = invRepo.getLatestMutualFundNav(holding->instrumentId);
        if (latestNav && *latestNav > 0.0) {
            int64_t navCents = static_cast<int64_t>(std::round(*latestNav * 100.0));
            detail.summary.hasMarketPrice = true;
            detail.summary.latestPriceCents = navCents;
            int64_t curVal = static_cast<int64_t>(std::round(detail.summary.quantity * (*latestNav) * 100.0));
            detail.summary.hasCurrentValue = true;
            detail.summary.currentValueCents = curVal;
            detail.summary.hasPnl = true;
            detail.summary.unrealizedPnlCents = curVal - detail.summary.investedAmountCents;
            if (detail.summary.investedAmountCents > 0) {
                detail.summary.unrealizedPnlPercent = (static_cast<double>(detail.summary.unrealizedPnlCents) / static_cast<double>(detail.summary.investedAmountCents)) * 100.0;
            }
        }
    } else if (holding->assetTypeCode == "GOLD") {
        detail.goldDetails = invRepo.getGoldDetails(holding->id);
        auto latestPrice = invRepo.getLatestMarketPrice(holding->instrumentId);
        if (latestPrice && *latestPrice > 0) {
            detail.summary.hasMarketPrice = true;
            detail.summary.latestPriceCents = *latestPrice;
            int64_t curVal = 0, pnl = 0;
            double pnlPct = 0.0;
            if (Finance::CostBasisEngine::calculateUnrealizedPnl(detail.summary.quantity, detail.summary.investedAmountCents, *latestPrice, curVal, pnl, pnlPct)) {
                detail.summary.hasCurrentValue = true;
                detail.summary.currentValueCents = curVal;
                detail.summary.hasPnl = true;
                detail.summary.unrealizedPnlCents = pnl;
                detail.summary.unrealizedPnlPercent = pnlPct;
            }
        }
    } else if (holding->assetTypeCode == "FIXED_DEPOSIT") {
        detail.fdDetails = invRepo.getFixedDepositDetails(holding->id);
        if (detail.fdDetails) {
            detail.summary.hasMarketPrice = true;
            detail.summary.latestPriceCents = detail.fdDetails->principalAmountCents;
            detail.summary.hasCurrentValue = true;
            detail.summary.currentValueCents = detail.fdDetails->estimatedMaturityAmountCents;
            detail.summary.hasPnl = true;
            detail.summary.unrealizedPnlCents = detail.summary.currentValueCents - detail.summary.investedAmountCents;
            if (detail.summary.investedAmountCents > 0) {
                detail.summary.unrealizedPnlPercent = (static_cast<double>(detail.summary.unrealizedPnlCents) / static_cast<double>(detail.summary.investedAmountCents)) * 100.0;
            }
        }
    } else if (holding->assetTypeCode == "BOND") {
        detail.bondDetails = invRepo.getBondDetails(holding->id);
        if (detail.bondDetails) {
            detail.summary.hasMarketPrice = true;
            detail.summary.latestPriceCents = detail.bondDetails->faceValueCents;
            detail.summary.hasCurrentValue = true;
            detail.summary.currentValueCents = static_cast<int64_t>(std::round(detail.bondDetails->faceValueCents * detail.summary.quantity));
            detail.summary.hasPnl = true;
            detail.summary.unrealizedPnlCents = detail.summary.currentValueCents - detail.summary.investedAmountCents;
            if (detail.summary.investedAmountCents > 0) {
                detail.summary.unrealizedPnlPercent = (static_cast<double>(detail.summary.unrealizedPnlCents) / static_cast<double>(detail.summary.investedAmountCents)) * 100.0;
            }
        }
    }

    return detail;
}

std::pair<bool, std::string> InvestmentService::updateInvestment(int userId,
                                                                 int investmentId,
                                                                 const std::string &notes,
                                                                 const std::string &status,
                                                                 const std::string &portfolioTag)
{
    Database::InvestmentRepository invRepo;
    bool ok = invRepo.updateInvestment(investmentId, userId, notes, status, portfolioTag);
    if (!ok) {
        return {false, invRepo.getLastError().empty() ? "Failed to update investment" : invRepo.getLastError()};
    }
    return {true, "Investment updated successfully"};
}

int InvestmentService::getTransactionCount(int userId, int investmentId)
{
    Database::InvestmentRepository invRepo;
    Database::TransactionRepository txnRepo;

    auto holding = invRepo.getInvestmentByIdForUser(investmentId, userId);
    if (!holding) {
        return 0;
    }
    auto txs = txnRepo.getTransactionsByInvestmentId(investmentId);
    return static_cast<int>(txs.size());
}

std::pair<bool, std::string> InvestmentService::deleteInvestment(int userId, int investmentId, bool cascadeTransactions)
{
    Database::InvestmentRepository invRepo;
    if (!cascadeTransactions) {
        int txCount = getTransactionCount(userId, investmentId);
        if (txCount > 0) {
            return {false, "Cannot delete investment: active transaction ledger records exist (Referential Policy RESTRICT)."};
        }
        bool ok = invRepo.deleteInvestmentForUser(investmentId, userId);
        if (!ok) {
            return {false, invRepo.getLastError().empty() ? "Failed to delete investment" : invRepo.getLastError()};
        }
        return {true, "Investment deleted successfully."};
    } else {
        bool ok = invRepo.deleteInvestmentWithDependencies(investmentId, userId);
        if (!ok) {
            return {false, invRepo.getLastError().empty() ? "Failed to delete investment" : invRepo.getLastError()};
        }
        return {true, "Investment and associated records deleted successfully."};
    }
}

} // namespace Services
} // namespace InvestSphere

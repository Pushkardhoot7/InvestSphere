#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>

#include "CostBasisEngine.h"
#include "CompoundingEngine.h"
#include "InvestmentService.h"
#include "DashboardService.h"
#include "DatabaseManager.h"
#include "UserRepository.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"

int main()
{
    std::cout << "======================================================================\n";
    std::cout << "  INVESTSPHERE — PHASE 6 DASHBOARD & INVESTMENT MANAGEMENT TEST SUITE \n";
    std::cout << "======================================================================\n";

    auto &db = InvestSphere::Database::DatabaseManager::instance();
    bool dbOk = db.initialize("test_phase6_investsphere.db");
    std::cout << " [SETUP] Database Engine Initialization                    : " << (dbOk ? "PASS" : "FAIL") << "\n";
    assert(dbOk);

    db.initializeSchema("sql/schema.sql");
    db.initializeViews("sql/views.sql");
    db.initializeTriggers("sql/triggers.sql");

    auto &invService = InvestSphere::Services::InvestmentService::instance();
    auto &dashService = InvestSphere::Services::DashboardService::instance();
    InvestSphere::Database::UserRepository userRepo;

    // Create 2 test users for strict multi-user isolation verification
    InvestSphere::Models::User userA;
    userA.fullName = "Alice Sterling";
    userA.email = "alice@investsphere.test";
    userA.passwordHash = "hashA";
    auto userAIdOpt = userRepo.createUser(userA);
    int userAId = userAIdOpt.value_or(1);

    InvestSphere::Models::User userB;
    userB.fullName = "Bob Vance";
    userB.email = "bob@investsphere.test";
    userB.passwordHash = "hashB";
    auto userBIdOpt = userRepo.createUser(userB);
    int userBId = userBIdOpt.value_or(2);

    std::cout << " [SETUP] Multi-User Identities Created (Alice ID: " << userAId << ", Bob ID: " << userBId << ") : PASS\n";

    // -------------------------------------------------------------------------
    // TEST 1: CompoundingEngine (Fixed Deposit Math)
    // -------------------------------------------------------------------------
    {
        // $10,000 at 7.10% quarterly compounding for 12 months
        // Formula: 10000 * (1 + 0.071/4)^4 = 10729.00 -> 1072900 cents
        int64_t maturity = InvestSphere::Finance::CompoundingEngine::calculateMaturityAmount(
            1000000, 7.10, 12, "QUARTERLY", "CUMULATIVE"
        );
        int64_t interest = InvestSphere::Finance::CompoundingEngine::calculateTotalInterest(
            1000000, 7.10, 12, "QUARTERLY", "CUMULATIVE"
        );
        bool mathOk = (maturity >= 1072800 && maturity <= 1073000) && (interest == maturity - 1000000);
        std::cout << " [TEST  1] CompoundingEngine - FD Interest & Maturity Math  : " << (mathOk ? "PASS" : "FAIL") 
                  << " (Projected: $" << (maturity / 100.0) << ")\n";
        assert(mathOk);
    }

    // -------------------------------------------------------------------------
    // TEST 2: CostBasisEngine (Weighted Average Cost & Realized/Unrealized P&L)
    // -------------------------------------------------------------------------
    {
        std::vector<InvestSphere::Models::Transaction> txs;
        // Tranche 1: Buy 50 shares @ $100 ($5,000)
        InvestSphere::Models::Transaction t1;
        t1.id = 1;
        t1.transactionType = "BUY";
        t1.transactionDate = "2026-01-01 10:00:00";
        t1.quantity = 50.0;
        t1.unitPriceCents = 10000;
        t1.netAmountCents = 500000;
        txs.push_back(t1);

        // Tranche 2: Buy 50 shares @ $150 ($7,500) -> Total 100 shares, total cost $12,500 -> WAC = $125/share
        InvestSphere::Models::Transaction t2;
        t2.id = 2;
        t2.transactionType = "BUY";
        t2.transactionDate = "2026-02-01 10:00:00";
        t2.quantity = 50.0;
        t2.unitPriceCents = 15000;
        t2.netAmountCents = 750000;
        txs.push_back(t2);

        // Tranche 3: Sell 40 shares @ $200 ($8,000 proceeds)
        // Cost of 40 shares sold = 40 * $125 = $5,000 -> Realized Gain = $8,000 - $5,000 = $3,000
        // Remaining shares = 60 @ $125 cost basis = $7,500
        InvestSphere::Models::Transaction t3;
        t3.id = 3;
        t3.transactionType = "SELL";
        t3.transactionDate = "2026-03-01 10:00:00";
        t3.quantity = 40.0;
        t3.unitPriceCents = 20000;
        t3.netAmountCents = 800000;
        txs.push_back(t3);

        auto res = InvestSphere::Finance::CostBasisEngine::calculateCostBasis(txs);
        bool cbOk = (std::abs(res.remainingQuantity - 60.0) < 0.001) &&
                    (res.totalInvestedCostCents == 750000) &&
                    (res.averageCostPerUnitCents == 12500) &&
                    (res.totalRealizedPnlCents == 300000);

        // Unrealized P&L: if current market price is $220/share
        int64_t curVal = 0, unRealPnl = 0;
        double pnlPct = 0.0;
        bool unRealOk = InvestSphere::Finance::CostBasisEngine::calculateUnrealizedPnl(
            res.remainingQuantity, res.totalInvestedCostCents, 22000, curVal, unRealPnl, pnlPct
        );
        // curVal = 60 * 220 = $13,200 (1320000 cents); unRealPnl = 1320000 - 750000 = 570000 ($5,700)
        unRealOk = unRealOk && (curVal == 1320000) && (unRealPnl == 570000);

        std::cout << " [TEST  2] CostBasisEngine - WAC Algorithm & Gain Split    : " << (cbOk && unRealOk ? "PASS" : "FAIL") << "\n";
        assert(cbOk && unRealOk);
    }

    // -------------------------------------------------------------------------
    // TEST 3: Input Validation Rules
    // -------------------------------------------------------------------------
    {
        InvestSphere::Services::InvestmentInputDTO badInput;
        badInput.categoryCode = "STOCK";
        badInput.symbol = ""; // missing
        auto v1 = invService.validateInput(badInput);
        assert(!v1.first);

        badInput.symbol = "AAPL";
        badInput.instrumentName = "Apple Inc.";
        badInput.purchaseDate = "2026-01-10";
        badInput.quantity = -10.0; // negative quantity
        badInput.unitPriceCents = 15000;
        auto v2 = invService.validateInput(badInput);
        assert(!v2.first);

        badInput.quantity = 10.0;
        badInput.unitPriceCents = -50; // negative price
        auto v3 = invService.validateInput(badInput);
        assert(!v3.first);

        // Valid stock input
        badInput.unitPriceCents = 15000;
        auto v4 = invService.validateInput(badInput);
        assert(v4.first);

        std::cout << " [TEST  3] InvestmentService - Input Validation Rejections   : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 4: Adding Investments Across All 5 Categories
    // -------------------------------------------------------------------------
    int invStockId = 0;
    int invMfId = 0;
    int invGoldId = 0;
    int invFdId = 0;
    int invBondId = 0;

    // 1. Stock
    {
        InvestSphere::Services::InvestmentInputDTO s;
        s.categoryCode = "STOCK";
        s.symbol = "TCS";
        s.instrumentName = "Tata Consultancy Services";
        s.purchaseDate = "2026-02-15";
        s.quantity = 20.0;
        s.unitPriceCents = 380000; // Rs 3,800.00
        s.exchange = "NSE";
        s.sector = "Technology";
        s.industry = "IT Services";
        s.notes = "Core Indian IT holding";
        auto res = invService.addInvestment(userAId, s);
        assert(res.first);
        invStockId = std::stoi(res.second);
    }

    // 2. Mutual Fund
    {
        InvestSphere::Services::InvestmentInputDTO mf;
        mf.categoryCode = "MUTUAL_FUND";
        mf.symbol = "PPFAS-FLEXI";
        mf.instrumentName = "Parag Parikh Flexi Cap Fund";
        mf.purchaseDate = "2026-02-20";
        mf.quantity = 50.0;
        mf.unitPriceCents = 7500; // NAV 75.00
        mf.amcName = "PPFAS Mutual Fund";
        mf.fundCategory = "EQUITY";
        mf.planType = "GROWTH";
        mf.expenseRatio = 0.65;
        mf.notes = "Systematic long term fund";
        auto res = invService.addInvestment(userAId, mf);
        assert(res.first);
        invMfId = std::stoi(res.second);
    }

    // 3. Gold
    {
        InvestSphere::Services::InvestmentInputDTO g;
        g.categoryCode = "GOLD";
        g.symbol = "GOLD-24K-BAR";
        g.instrumentName = "24K Fine Gold Bar";
        g.purchaseDate = "2026-03-01";
        g.quantity = 15.0; // 15 grams
        g.unitPriceCents = 760000; // Rs 7,600 / g
        g.goldForm = "PHYSICAL_BAR";
        g.purityKarat = 24.0;
        g.storageLocation = "Secure Bank Locker";
        g.notes = "Bullion reserve";
        auto res = invService.addInvestment(userAId, g);
        assert(res.first);
        invGoldId = std::stoi(res.second);
    }

    // 4. Fixed Deposit
    {
        InvestSphere::Services::InvestmentInputDTO fd;
        fd.categoryCode = "FIXED_DEPOSIT";
        fd.symbol = "FD-SBI-2026";
        fd.instrumentName = "SBI 1-Year Fixed Deposit";
        fd.institutionName = "State Bank of India";
        fd.accountNumber = "SBI-FD-49102";
        fd.principalAmountCents = 50000000; // Rs 5,00,000.00
        fd.annualInterestRate = 7.25;
        fd.compoundingFrequency = "QUARTERLY";
        fd.payoutType = "CUMULATIVE";
        fd.startDate = "2026-03-10";
        fd.purchaseDate = "2026-03-10";
        fd.maturityDate = "2027-03-10";
        fd.durationMonths = 12;
        fd.notes = "Safety bucket term deposit";
        auto res = invService.addInvestment(userAId, fd);
        assert(res.first);
        invFdId = std::stoi(res.second);
    }

    // 5. Bond
    {
        InvestSphere::Services::InvestmentInputDTO b;
        b.categoryCode = "BOND";
        b.symbol = "GSEC-726-2033";
        b.instrumentName = "Government of India 7.26% 2033";
        b.issuerName = "Reserve Bank of India";
        b.bondType = "GOVERNMENT";
        b.quantity = 10.0;
        b.unitPriceCents = 100000; // Rs 1,000.00 face value
        b.faceValueCents = 100000;
        b.couponRate = 7.26;
        b.couponFrequency = "SEMI_ANNUALLY";
        b.issueDate = "2023-05-15";
        b.purchaseDate = "2026-03-15";
        b.bondMaturityDate = "2033-05-15";
        b.creditRating = "SOVEREIGN";
        b.notes = "Risk-free sovereign coupon income";
        auto res = invService.addInvestment(userAId, b);
        assert(res.first);
        invBondId = std::stoi(res.second);
    }

    std::cout << " [TEST  4] Ingest 5 Distinct Asset Classes (Stock/MF/Gold/FD/Bond): PASS\n";

    // -------------------------------------------------------------------------
    // TEST 5: Automatic Transaction Ledger & Initial Purchase Recording
    // -------------------------------------------------------------------------
    {
        int txCountStock = invService.getTransactionCount(userAId, invStockId);
        int txCountFd = invService.getTransactionCount(userAId, invFdId);
        bool txRecorded = (txCountStock == 1) && (txCountFd == 1);
        std::cout << " [TEST  5] Automatic Initial Ledger Transaction Creation   : " << (txRecorded ? "PASS" : "FAIL") << "\n";
        assert(txRecorded);
    }

    // -------------------------------------------------------------------------
    // TEST 6: Searching Investments
    // -------------------------------------------------------------------------
    {
        auto searchTcs = invService.getUserInvestments(userAId, "TCS");
        assert(searchTcs.size() == 1 && searchTcs[0].symbol == "TCS");

        auto searchGold = invService.getUserInvestments(userAId, "Gold");
        assert(searchGold.size() == 1 && searchGold[0].categoryCode == "GOLD");

        auto searchNone = invService.getUserInvestments(userAId, "NONEXISTENT_XYZ");
        assert(searchNone.empty());

        std::cout << " [TEST  6] Search Holdings by Ticker, Name & Keyword       : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 7: Category and Status Filtering
    // -------------------------------------------------------------------------
    {
        auto stocksOnly = invService.getUserInvestments(userAId, "", "STOCK");
        assert(stocksOnly.size() == 1 && stocksOnly[0].categoryCode == "STOCK");

        auto fdsOnly = invService.getUserInvestments(userAId, "", "FIXED_DEPOSIT");
        assert(fdsOnly.size() == 1 && fdsOnly[0].categoryCode == "FIXED_DEPOSIT");

        auto allActive = invService.getUserInvestments(userAId, "", "ALL", "ACTIVE");
        assert(allActive.size() == 5);

        std::cout << " [TEST  7] Multi-Asset Category & Status Filtering         : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 8: Missing Market Quotes (Valuation Non-Fabrication)
    // -------------------------------------------------------------------------
    {
        // Currently TCS has no market price set in market_prices
        auto list = invService.getUserInvestments(userAId, "TCS");
        assert(!list.empty());
        bool priceMissing = !list[0].hasMarketPrice && !list[0].hasCurrentValue;
        std::cout << " [TEST  8] Unquoted Holdings Clearly Marked Unavailable    : " << (priceMissing ? "PASS" : "FAIL") 
                  << " (Not fabricated)\n";
        assert(priceMissing);
    }

    // -------------------------------------------------------------------------
    // TEST 9: Market Price Ingestion & Real-Time Valuation Update
    // -------------------------------------------------------------------------
    {
        InvestSphere::Database::InvestmentRepository invRepo;
        // Provide market quote for TCS: Rs 4,150.00 (bought at Rs 3,800.00)
        auto tcsInst = invRepo.getInstrumentBySymbol("TCS");
        assert(tcsInst);
        invRepo.setMarketPrice(tcsInst->id, 415000, "EXCHANGE_FEED");

        auto list = invService.getUserInvestments(userAId, "TCS");
        assert(!list.empty());
        bool priceFound = list[0].hasMarketPrice && list[0].hasCurrentValue;
        // 20 shares * Rs 4,150 = Rs 83,000 (8300000 cents); cost = 20 * 3800 = 7600000 cents; P&L = +700000 cents (+9.21%)
        bool valCorrect = (list[0].currentValueCents == 8300000) && (list[0].unrealizedPnlCents == 700000);
        std::cout << " [TEST  9] Live Market Price Valuation & Unrealized P&L    : " << (priceFound && valCorrect ? "PASS" : "FAIL") << "\n";
        assert(priceFound && valCorrect);
    }

    // -------------------------------------------------------------------------
    // TEST 10: Sorting by Name, Date, and Invested Capital
    // -------------------------------------------------------------------------
    {
        auto sortNameAsc = invService.getUserInvestments(userAId, "", "ALL", "ACTIVE", "NAME", true);
        auto sortNameDesc = invService.getUserInvestments(userAId, "", "ALL", "ACTIVE", "NAME", false);
        assert(sortNameAsc.front().name <= sortNameAsc.back().name);
        assert(sortNameDesc.front().name >= sortNameDesc.back().name);

        auto sortInvestedDesc = invService.getUserInvestments(userAId, "", "ALL", "ACTIVE", "INVESTED", false);
        assert(sortInvestedDesc.front().investedAmountCents >= sortInvestedDesc.back().investedAmountCents);

        std::cout << " [TEST 10] Column Sorting (Name, Date, Invested Capital)   : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 11: Viewing Investment Details and Linked Ledger History
    // -------------------------------------------------------------------------
    {
        auto detail = invService.getInvestmentDetail(userAId, invStockId);
        assert(detail);
        assert(detail->summary.symbol == "TCS");
        assert(static_cast<bool>(detail->stockDetails));
        assert(detail->stockDetails->sector == "Technology");
        assert(detail->transactions.size() == 1);
        assert(detail->transactions[0].transactionType == "BUY");

        std::cout << " [TEST 11] View Investment Details & Linked Ledger Audit   : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 12: Editing Investment Metadata
    // -------------------------------------------------------------------------
    {
        auto updateRes = invService.updateInvestment(userAId, invStockId, "Updated notes: Core long-term holding", "ACTIVE", "LONG_TERM");
        assert(updateRes.first);

        auto detail = invService.getInvestmentDetail(userAId, invStockId);
        assert(detail);
        assert(detail->summary.notes == "Updated notes: Core long-term holding");
        assert(detail->summary.portfolioTag == "LONG_TERM");

        std::cout << " [TEST 12] Edit Investment Holding Metadata                : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 13: Dashboard Metrics Aggregation
    // -------------------------------------------------------------------------
    {
        auto dash = dashService.getDashboardData(userAId);
        assert(dash.summary.activeHoldingsCount == 5);
        assert(dash.summary.totalInvestedCents > 0);
        assert(dash.summary.hasValuationData);
        assert(!dash.allocation.empty());
        assert(!dash.recentTransactions.empty());
        assert(!dash.upcomingEvents.empty());
        assert(!dash.topHoldings.empty());

        std::cout << " [TEST 13] Dashboard Data Aggregation from SQLite          : PASS (Active: " 
                  << dash.summary.activeHoldingsCount << ", Total Invested: ₹" << (dash.summary.totalInvestedCents / 100.0) << ")\n";
    }

    // -------------------------------------------------------------------------
    // TEST 14: Strict User Isolation (Alice vs Bob)
    // -------------------------------------------------------------------------
    {
        // Bob Vance has zero investments
        auto bobHoldings = invService.getUserInvestments(userBId);
        assert(bobHoldings.empty());

        auto bobDashboard = dashService.getDashboardData(userBId);
        assert(bobDashboard.summary.activeHoldingsCount == 0);
        assert(bobDashboard.summary.totalInvestedCents == 0);
        assert(!bobDashboard.summary.hasValuationData);
        assert(bobDashboard.recentTransactions.empty());
        assert(bobDashboard.upcomingEvents.empty());

        // Bob cannot view or edit Alice's stock
        auto illegalView = invService.getInvestmentDetail(userBId, invStockId);
        assert(!illegalView);

        auto illegalEdit = invService.updateInvestment(userBId, invStockId, "Hacked notes", "CLOSED", "HACK");
        assert(!illegalEdit.first);

        std::cout << " [TEST 14] Strict Multi-User Data Isolation & Empty State  : PASS\n";
    }

    // -------------------------------------------------------------------------
    // TEST 15: Foreign-Key Integrity & Safe Deletion Policy
    // -------------------------------------------------------------------------
    {
        // Alice attempts to delete TCS without cascadeTransactions -> Must fail due to active transactions!
        auto rejectDel = invService.deleteInvestment(userAId, invStockId, false);
        assert(!rejectDel.first);
        std::cout << " [TEST 15a] Deletion Policy RESTRICT on Dependent History  : PASS (Blocked silent delete)\n";

        // Confirmed deletion with cascadeTransactions -> Safely deletes holding, transactions, and events
        auto confirmedDel = invService.deleteInvestment(userAId, invStockId, true);
        assert(confirmedDel.first);

        // Verify holding is gone
        auto checkDeleted = invService.getInvestmentDetail(userAId, invStockId);
        assert(!checkDeleted);

        // Dashboard active count decremented
        auto updatedDash = dashService.getDashboardData(userAId);
        assert(updatedDash.summary.activeHoldingsCount == 4);
        std::cout << " [TEST 15b] Confirmed Safe Atomic Deletion & Dashboard Sync: PASS\n";
    }

    std::cout << "======================================================================\n";
    std::cout << "  ALL 15 PHASE 6 TESTS PASSED SUCCESSFULLY!                          \n";
    std::cout << "======================================================================\n";

    return 0;
}

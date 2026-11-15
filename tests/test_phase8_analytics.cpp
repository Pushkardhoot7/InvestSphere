/**
 * @file test_phase8_analytics.cpp
 * @brief Comprehensive automated test suite for Phase 8:
 *        SIP Calculator + SWP Calculator + Portfolio Analytics + Snapshots
 *        InvestSphere Smart Investment & Portfolio Management System.
 *
 * Test Coverage (23 scenarios):
 *  ── SIP Calculator ──
 *  1.  Zero return rate (FV = P * n exactly)
 *  2.  Positive expected return (monthly compounding ordinary annuity)
 *  3.  1-year investment duration
 *  4.  Long duration investment (20 years)
 *  5.  Invalid monthly contribution (rejects < ₹1)
 *  6.  Invalid duration (< 1 or > 50) or invalid return rate
 *  7.  Step-up SIP contribution schedule (annual step-up compounding)
 *  8.  Comparison against independently calculated expected values
 *
 *  ── SWP Calculator ──
 *  9.  Zero return rate (corpus decreases monotonically by withdrawal)
 * 10.  Positive return rate (growth applied before withdrawal)
 * 11.  Withdrawal smaller than monthly interest (sustainable balance)
 * 12.  Withdrawal exhausting corpus before duration (depletion tracking)
 * 13.  Zero monthly withdrawal (pure compounding without outflow)
 * 14.  Invalid inputs (negative withdrawal or corpus < ₹1,000)
 * 15.  Multi-year withdrawal balance timeline accuracy
 *
 *  ── Portfolio Analytics & Snapshots ──
 * 16.  Empty portfolio handling (graceful zero-state and note)
 * 17.  PortfolioSnapshotRepository — record and retrieve snapshots
 * 18.  PortfolioSnapshotRepository — daily deduplication (update existing row)
 * 19.  AnalyticsService — multi-category aggregation & allocation percentages
 * 20.  AnalyticsService — valuation coverage & unquoted holdings separation
 * 21.  AnalyticsService — top and worst performer ranking logic
 * 22.  AnalyticsService — category filtering
 * 23.  Multi-user isolation — User A cannot view User B's analytics or snapshots
 */

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

#include "AnnuityEngine.h"
#include "PortfolioSnapshotRepository.h"
#include "AnalyticsService.h"
#include "PortfolioCalculationService.h"
#include "InvestmentService.h"
#include "TransactionService.h"
#include "DatabaseManager.h"
#include "UserRepository.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"

// ─── Test Harness ─────────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

static void reportTest(int testNum, const std::string &label, bool ok,
                       const std::string &extra = "")
{
    std::cout << " [TEST " << (testNum < 10 ? " " : "") << testNum << "] "
              << label;

    int pad = 62 - static_cast<int>(label.size());
    for (int i = 0; i < pad; ++i) std::cout << ' ';
    std::cout << ": " << (ok ? "PASS" : "FAIL");
    if (!extra.empty()) std::cout << " (" << extra << ")";
    std::cout << '\n';

    if (ok) ++g_passed; else ++g_failed;
    assert(ok && "Test assertion failed — see output above.");
}

// ─── DB Helpers ───────────────────────────────────────────────────────────────

static int createTestUser(const std::string &name, const std::string &email)
{
    InvestSphere::Database::UserRepository repo;
    InvestSphere::Models::User u;
    u.fullName     = name;
    u.email        = email;
    u.passwordHash = "hash_phase8";
    auto idOpt = repo.createUser(u);
    return idOpt ? *idOpt : -1;
}

static int addTestHolding(int userId, const std::string &cat, const std::string &symbol,
                          const std::string &name, double qty, int64_t buyPriceCents,
                          int64_t marketPriceCents = 0)
{
    auto &invSvc = InvestSphere::Services::InvestmentService::instance();
    InvestSphere::Services::InvestmentInputDTO dto;
    dto.categoryCode    = cat;
    dto.symbol          = symbol;
    dto.instrumentName  = name;
    dto.purchaseDate    = "2026-01-01";
    dto.quantity        = qty;
    dto.unitPriceCents  = buyPriceCents;
    dto.exchange        = "NSE";
    dto.sector          = "General";
    dto.notes           = "Phase8 Test Holding";

    auto res = invSvc.addInvestment(userId, dto);
    if (!res.first || res.second.empty()) return -1;

    int invId = -1;
    try {
        invId = std::stoi(res.second);
    } catch (...) {
        return -1;
    }

    if (marketPriceCents > 0) {
        InvestSphere::Database::InvestmentRepository invRepo;
        auto invOpt = invRepo.getInvestmentById(invId);
        if (invOpt) {
            if (cat == "MUTUAL_FUND") {
                invRepo.setMutualFundNav(invOpt->instrumentId, static_cast<double>(marketPriceCents) / 100.0);
            } else {
                invRepo.setMarketPrice(invOpt->instrumentId, marketPriceCents);
            }
        }
    }
    return invId;
}

// ─── Main Test Runner ─────────────────────────────────────────────────────────

int main()
{
    std::cout << "=================================================================\n";
    std::cout << " InvestSphere — Phase 8: Calculators & Analytics Test Suite\n";
    std::cout << "=================================================================\n\n";

    // ─────────────────────────────────────────────────────────────────────────
    // Part 1: SIP Calculator Tests
    // ─────────────────────────────────────────────────────────────────────────

    // Test 1: Zero return rate
    {
        double P = 10000.0;
        int years = 5;
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(P, 0.0, years, 0.0);
        int64_t expectedInvestedCents = static_cast<int64_t>(P * years * 12 * 100);
        bool ok = res.valid &&
                  res.totalInvestedCents == expectedInvestedCents &&
                  res.maturityValueCents == expectedInvestedCents &&
                  res.estimatedGainCents == 0 &&
                  res.totalInstallments == 60;
        reportTest(1, "SIP — Zero return rate (FV = P * n)", ok);
    }

    // Test 2: Positive expected return (10k/m, 12% p.a., 10 yrs)
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(10000.0, 12.0, 10, 0.0);
        double fv = static_cast<double>(res.maturityValueCents) / 100.0;
        double diff = std::abs(fv - 2300386.89);
        bool ok = res.valid && (diff < 2.0) && (res.totalInvestedCents == 120000000);
        reportTest(2, "SIP — Positive return rate (12% 10yr compounding)", ok,
                   "FV=" + std::to_string(static_cast<int>(fv)));
    }

    // Test 3: 1-year investment
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(5000.0, 12.0, 1, 0.0);
        double fv = static_cast<double>(res.maturityValueCents) / 100.0;
        double diff = std::abs(fv - 63412.09);
        bool ok = res.valid && (diff < 1.0) && (res.totalInstallments == 12);
        reportTest(3, "SIP — 1-year investment horizon", ok);
    }

    // Test 4: Long duration investment (20 years)
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(15000.0, 15.0, 20, 0.0);
        bool ok = res.valid && res.totalInstallments == 240 &&
                  (res.maturityValueCents > res.totalInvestedCents * 3);
        reportTest(4, "SIP — 20-year long duration compounding", ok);
    }

    // Test 5: Invalid monthly contribution (< 1.0)
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(0.0, 10.0, 5, 0.0);
        bool ok = !res.valid && !res.errorMessage.empty();
        reportTest(5, "SIP — Invalid monthly contribution rejected", ok);
    }

    // Test 6: Invalid duration or return rate
    {
        auto res1 = InvestSphere::Finance::AnnuityEngine::calculateSip(1000.0, -5.0, 5, 0.0);
        auto res2 = InvestSphere::Finance::AnnuityEngine::calculateSip(1000.0, 10.0, 0, 0.0);
        auto res3 = InvestSphere::Finance::AnnuityEngine::calculateSip(1000.0, 10.0, 55, 0.0);
        bool ok = !res1.valid && !res2.valid && !res3.valid;
        reportTest(6, "SIP — Invalid duration and rate ranges rejected", ok);
    }

    // Test 7: Step-up SIP contribution schedule (10% step-up annually)
    {
        auto resStd = InvestSphere::Finance::AnnuityEngine::calculateSip(10000.0, 12.0, 5, 0.0);
        auto resStep = InvestSphere::Finance::AnnuityEngine::calculateSip(10000.0, 12.0, 5, 10.0);
        bool ok = resStep.valid &&
                  (resStep.totalInvestedCents > resStd.totalInvestedCents) &&
                  (resStep.maturityValueCents > resStd.maturityValueCents) &&
                  (resStep.yearlyBreakdown.size() == 5);
        reportTest(7, "SIP — Step-up SIP annual accumulation schedule", ok);
    }

    // Test 8: Comparison against independently calculated expected values
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSip(1000.0, 6.0, 2, 0.0);
        double fv = static_cast<double>(res.maturityValueCents) / 100.0;
        double diff = std::abs(fv - 25431.96);
        bool ok = res.valid && (diff < 1.0);
        reportTest(8, "SIP — Benchmark verification against textbook value", ok);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Part 2: SWP Calculator Tests
    // ─────────────────────────────────────────────────────────────────────────

    // Test 9: Zero return rate
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(100000.0, 10000.0, 0.0, 1);
        bool ok = res.valid && res.depleted && (res.depletedAtMonth == 10) &&
                  (res.finalBalanceCents == 0) && (res.totalWithdrawnCents == 10000000);
        reportTest(9, "SWP — Zero return rate linear depletion", ok);
    }

    // Test 10: Positive return rate
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(1000000.0, 5000.0, 12.0, 1);
        bool ok = res.valid && !res.depleted &&
                  (res.finalBalanceCents > res.initialCorpusCents) &&
                  (res.withdrawalsCompleted == 12);
        reportTest(10, "SWP — Positive return with growth before withdrawal", ok);
    }

    // Test 11: Withdrawal smaller than monthly interest (sustainable balance)
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(2000000.0, 10000.0, 10.0, 5);
        bool ok = res.valid && !res.depleted && (res.finalBalanceCents > res.initialCorpusCents);
        reportTest(11, "SWP — Sustainable withdrawal preserves corpus", ok);
    }

    // Test 12: Corpus depletion before duration
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(500000.0, 50000.0, 6.0, 3);
        bool ok = res.valid && res.depleted && (res.depletedAtMonth > 0) &&
                  (res.depletedAtMonth < 36) && (res.finalBalanceCents == 0);
        reportTest(12, "SWP — Depletion tracking with partial final withdrawal", ok,
                   "Depleted at month " + std::to_string(res.depletedAtMonth));
    }

    // Test 13: Zero monthly withdrawal
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(100000.0, 0.0, 12.0, 1);
        double expected = 100000.0 * std::pow(1.01, 12);
        double actual = static_cast<double>(res.finalBalanceCents) / 100.0;
        bool ok = res.valid && !res.depleted && (res.totalWithdrawnCents == 0) &&
                  std::abs(actual - expected) < 2.0;
        reportTest(13, "SWP — Zero withdrawal pure compounding", ok);
    }

    // Test 14: Invalid inputs
    {
        auto res1 = InvestSphere::Finance::AnnuityEngine::calculateSwp(500.0, 100.0, 8.0, 5);
        auto res2 = InvestSphere::Finance::AnnuityEngine::calculateSwp(100000.0, -100.0, 8.0, 5);
        bool ok = !res1.valid && !res2.valid;
        reportTest(14, "SWP — Invalid corpus and negative withdrawal rejected", ok);
    }

    // Test 15: Multi-year withdrawal balance timeline
    {
        auto res = InvestSphere::Finance::AnnuityEngine::calculateSwp(1500000.0, 12000.0, 9.0, 5);
        bool ok = res.valid && res.yearlyBreakdown.size() == 5 &&
                  res.withdrawalsCompleted == 60;
        reportTest(15, "SWP — 5-year timeline breakdown structure", ok);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Part 3: Portfolio Analytics & Snapshots Tests
    // ─────────────────────────────────────────────────────────────────────────

    auto &db = InvestSphere::Database::DatabaseManager::instance();
    bool dbOk = db.initialize("test_phase8_investsphere.db");
    if (!dbOk) {
        dbOk = db.initialize(":memory:");
    }
    assert(dbOk && "Test database initialization failed");

    // Test 16: Empty portfolio handling
    {
        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto = analytics.computeAnalytics(88888); // non-existent user
        bool ok = dto.isEmptyPortfolio &&
                  (dto.summary.activeHoldingsCount == 0) &&
                  (dto.categories.empty()) &&
                  (!dto.performanceHistoryNote.empty());
        reportTest(16, "Analytics — Empty portfolio graceful response", ok);
    }

    // Test 17: PortfolioSnapshotRepository — record and retrieve
    {
        int snapUserId = createTestUser("Snap User", "snap1@investsphere.test");
        InvestSphere::Database::PortfolioSnapshotRepository snapRepo;
        InvestSphere::Database::PortfolioSnapshot s;
        s.userId              = snapUserId;
        s.snapshotTimestamp   = "2026-05-01 10:00:00";
        s.totalInvestedCents  = 50000000;
        s.estimatedValueCents = 55000000;
        s.realizedPnlCents    = 2000000;
        s.unrealizedPnlCents  = 5000000;
        s.notes               = "Test baseline";

        auto idOpt = snapRepo.recordSnapshot(s);
        bool okRecord = static_cast<bool>(idOpt);

        auto list = snapRepo.getSnapshots(snapUserId);
        bool okFetch = (list.size() == 1) &&
                       (list[0].totalInvestedCents == 50000000) &&
                       (list[0].estimatedValueCents == 55000000);
        reportTest(17, "Snapshots — Record and retrieve portfolio snapshot", okRecord && okFetch);
    }

    // Test 18: PortfolioSnapshotRepository — daily deduplication
    {
        int dedupUserId = createTestUser("Dedup User", "dedup1@investsphere.test");
        InvestSphere::Database::PortfolioSnapshotRepository snapRepo;

        InvestSphere::Database::PortfolioSnapshot s1;
        s1.userId              = dedupUserId;
        s1.snapshotTimestamp   = "2026-05-01 09:00:00";
        s1.totalInvestedCents  = 50000000;
        s1.estimatedValueCents = 55000000;
        snapRepo.recordSnapshot(s1);

        InvestSphere::Database::PortfolioSnapshot s2;
        s2.userId              = dedupUserId;
        s2.snapshotTimestamp   = "2026-05-01 18:30:00"; // Same day
        s2.totalInvestedCents  = 50000000;
        s2.estimatedValueCents = 57000000; // Updated value
        snapRepo.recordSnapshot(s2);

        auto list = snapRepo.getSnapshots(dedupUserId);
        bool ok = (list.size() == 1) && (list[0].estimatedValueCents == 57000000);
        reportTest(18, "Snapshots — Same-day deduplication and update", ok);
    }

    // Test 19: AnalyticsService — multi-category aggregation & allocation percentages
    int user19 = createTestUser("MultiCat User", "multicat@investsphere.test");
    {
        // 1. Stock: 10 units @ ₹3,000 (Cost = ₹30,000, Market = ₹36,000)
        addTestHolding(user19, "STOCK", "TCS", "Tata Consultancy", 10.0, 300000, 360000);

        // 2. Mutual Fund: 500 units @ ₹100 (Cost = ₹50,000, Market = ₹60,000)
        addTestHolding(user19, "MUTUAL_FUND", "NIFTY50", "Nifty 50 Index Fund", 500.0, 10000, 12000);

        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto = analytics.computeAnalytics(user19);

        bool ok = (dto.summary.activeHoldingsCount == 2) &&
                  (dto.categories.size() == 2) &&
                  (dto.summary.totalInvestedCostBasisCents == 8000000) &&
                  (dto.summary.totalCurrentEstimatedValueCents == 9600000);
        reportTest(19, "Analytics — Multi-category aggregation", ok);
    }

    // Test 20: AnalyticsService — valuation coverage & unquoted holdings separation
    int user20 = createTestUser("Coverage User", "cov@investsphere.test");
    {
        // 1. Quoted stock: 20 units @ ₹1,500 (Market = ₹1,800)
        addTestHolding(user20, "STOCK", "INFY", "Infosys", 20.0, 150000, 180000);

        // 2. Unquoted stock: 10 units @ ₹500 (No market price set in market_prices)
        addTestHolding(user20, "STOCK", "UNQ", "Unquoted Co", 10.0, 50000, 0);

        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto = analytics.computeAnalytics(user20);

        bool ok = (dto.summary.activeHoldingsCount == 2) &&
                  (dto.summary.quotedHoldingsCount == 1) &&
                  (dto.summary.unquotedHoldingsCount == 1) &&
                  (dto.summary.valuationCoveragePercent == 50.0);
        reportTest(20, "Analytics — Valuation coverage distinguishes unquoted assets", ok);
    }

    // Test 21: AnalyticsService — top and worst performer ranking logic
    int user21 = createTestUser("Performer User", "perf@investsphere.test");
    {
        // WIN: Cost 100, Market 150 (+50%)
        addTestHolding(user21, "STOCK", "WIN", "Winner Corp", 10.0, 10000, 15000);
        // LAG: Cost 100, Market 70 (-30%)
        addTestHolding(user21, "STOCK", "LAG", "Laggard Inc", 10.0, 10000, 7000);

        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto = analytics.computeAnalytics(user21);

        bool ok = !dto.topPerformers.empty() && !dto.worstPerformers.empty() &&
                  (dto.topPerformers[0].symbol == "WIN") &&
                  (dto.worstPerformers[0].symbol == "LAG") &&
                  (dto.topPerformers[0].unrealizedPnlPercent > 45.0) &&
                  (dto.worstPerformers[0].unrealizedPnlPercent < -25.0);
        reportTest(21, "Analytics — Top and worst performer ranking", ok);
    }

    // Test 22: AnalyticsService — category filtering
    {
        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto = analytics.computeAnalytics(user19, "STOCK");
        bool ok = (dto.categories.size() == 1) &&
                  (dto.categories[0].categoryCode == "STOCK");
        reportTest(22, "Analytics — Category filtering", ok);
    }

    // Test 23: Multi-user isolation
    {
        auto &analytics = InvestSphere::Services::AnalyticsService::instance();
        auto dto19 = analytics.computeAnalytics(user19);
        auto dto21 = analytics.computeAnalytics(user21);

        bool ok = (dto19.summary.activeHoldingsCount == 2) &&
                  (dto21.summary.activeHoldingsCount == 2) &&
                  (dto19.summary.totalInvestedCostBasisCents != dto21.summary.totalInvestedCostBasisCents);
        reportTest(23, "Analytics — Strict multi-user portfolio data isolation", ok);
    }

    std::cout << "\n=================================================================\n";
    std::cout << " Test Summary: " << g_passed << " Passed, " << g_failed << " Failed\n";
    std::cout << "=================================================================\n\n";

    return (g_failed == 0) ? 0 : 1;
}

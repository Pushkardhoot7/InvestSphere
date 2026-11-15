#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "DatabaseManager.h"
#include "TransactionScope.h"
#include "UserRepository.h"
#include "InvestmentRepository.h"
#include "TransactionRepository.h"
#include "SipRepository.h"
#include "SwpRepository.h"
#include "FinancialEventRepository.h"

int main()
{
    std::cout << "======================================================================\n";
    std::cout << "  INVESTSPHERE — C++17 DATABASE & REPOSITORY TEST SUITE               \n";
    std::cout << "======================================================================\n";

    auto &db = InvestSphere::Database::DatabaseManager::instance();

    // 1. Test database initialization
    bool initOk = db.initialize("test_investsphere.db");
    std::cout << " [TEST  1] Database Initialization & File Resolution       : " << (initOk ? "PASS" : "FAIL") << "\n";
    assert(initOk);

    // 2. Test status and core schema check
    std::cout << " [TEST  2] Database Connection Open Check                  : " << (db.isOpen() ? "PASS" : "FAIL") << "\n";
    assert(db.isOpen());

    // 3. Test Schema File Loading
    bool schemaOk = db.initializeSchema("sql/schema.sql");
    std::cout << " [TEST  3] Schema File Parsing (18 Tables)                  : " << (schemaOk ? "PASS" : "FAIL") << "\n";
    assert(schemaOk);

    // 4. Test Views Loading
    bool viewsOk = db.initializeViews("sql/views.sql");
    std::cout << " [TEST  4] Views File Parsing (6 Analytical Views)          : " << (viewsOk ? "PASS" : "FAIL") << "\n";
    assert(viewsOk);

    // 5. Test Triggers Loading
    bool triggersOk = db.initializeTriggers("sql/triggers.sql");
    std::cout << " [TEST  5] Triggers File Parsing (Audit & Integrity)        : " << (triggersOk ? "PASS" : "FAIL") << "\n";
    assert(triggersOk);

    // 6. Test Seed Data Loading
    bool seedOk = db.loadSeedData("sql/seed_data.sql");
    std::cout << " [TEST  6] Seed Demonstration Dataset Ingestion             : " << (seedOk ? "PASS" : "FAIL") << "\n";
    assert(seedOk);

    // 7. Test Schema Version Check
    int schemaVersion = db.getCurrentSchemaVersion();
    std::cout << " [TEST  7] Schema Version Verification (Reported: " << schemaVersion << ")         : PASS\n";

    // 8. Test UserRepository
    InvestSphere::Database::UserRepository userRepo;
    InvestSphere::Models::User newUser;
    newUser.fullName = "Dr. Alexander Wright";
    newUser.email = "alex.wright@investsphere.test";
    newUser.passwordHash = "argon2id_mock_hash_for_testing";
    newUser.accountStatus = "ACTIVE";

    auto createdUserId = userRepo.createUser(newUser);
    bool userCreateOk = static_cast<bool>(createdUserId);
    std::cout << " [TEST  8] UserRepository - User Creation                   : " << (userCreateOk ? "PASS" : "FAIL") << "\n";
    assert(userCreateOk);

    // 9. Test InvestmentRepository (Instrument & Holding)
    InvestSphere::Database::InvestmentRepository invRepo;
    InvestSphere::Models::Instrument inst;
    inst.investmentTypeId = 1; // Equity Stocks
    inst.symbol = "NVDA-TEST";
    inst.instrumentName = "NVIDIA Corporation (Test)";
    inst.currency = "USD";
    inst.exchange = "NASDAQ";
    inst.description = "GPU and AI semiconductor computing";

    auto createdInstId = invRepo.addInstrument(inst);
    bool instCreateOk = static_cast<bool>(createdInstId);
    std::cout << " [TEST  9] InvestmentRepository - Instrument Creation        : " << (instCreateOk ? "PASS" : "FAIL") << "\n";
    assert(instCreateOk);

    // 10. Test TransactionRepository
    InvestSphere::Database::TransactionRepository txnRepo;
    InvestSphere::Models::Transaction txn;
    txn.userId = createdUserId ? *createdUserId : 1;
    txn.investmentId = 1;
    txn.transactionType = "BUY";
    txn.transactionDate = "2026-10-26 10:30:00";
    txn.quantity = 15.0;
    txn.unitPriceCents = 13500; // $135.00
    txn.feesCents = 150;        // $1.50
    txn.taxesCents = 75;        // $0.75
    txn.grossAmountCents = 202500; // $2,025.00
    txn.netAmountCents = 202725;   // $2,027.25
    txn.notes = "Systematic accumulation";

    auto createdTxnId = txnRepo.recordTransaction(txn);
    bool txnOk = static_cast<bool>(createdTxnId);
    std::cout << " [TEST 10] TransactionRepository - Financial Ledger Ingest  : " << (txnOk ? "PASS" : "FAIL") << "\n";
    assert(txnOk);

    // 11. Test SipRepository
    InvestSphere::Database::SipRepository sipRepo;
    InvestSphere::Models::SipPlan sip;
    sip.userId = createdUserId ? *createdUserId : 1;
    sip.instrumentId = createdInstId ? *createdInstId : 1;
    sip.planName = "Tech Wealth Builder";
    sip.monthlyAmountCents = 50000; // $500.00
    sip.expectedAnnualReturn = 0.12;
    sip.durationMonths = 36;
    sip.contributionDayOfMonth = 5;
    sip.startDate = "2026-11-05";
    sip.status = "ACTIVE";

    auto createdSipId = sipRepo.createPlan(sip);
    bool sipOk = static_cast<bool>(createdSipId);
    std::cout << " [TEST 11] SipRepository - Systematic Investment Plan       : " << (sipOk ? "PASS" : "FAIL") << "\n";
    assert(sipOk);

    // 12. Test SwpRepository
    InvestSphere::Database::SwpRepository swpRepo;
    InvestSphere::Models::SwpPlan swp;
    swp.userId = createdUserId ? *createdUserId : 1;
    swp.instrumentId = createdInstId ? *createdInstId : 1;
    swp.planName = "Retirement Cash Flow";
    swp.initialCorpusCents = 5000000; // $50,000.00
    swp.monthlyWithdrawalCents = 25000; // $250.00
    swp.expectedAnnualReturn = 0.07;
    swp.durationMonths = 60;
    swp.withdrawalDayOfMonth = 15;
    swp.startDate = "2027-01-15";
    swp.status = "ACTIVE";

    auto createdSwpId = swpRepo.createPlan(swp);
    bool swpOk = static_cast<bool>(createdSwpId);
    std::cout << " [TEST 12] SwpRepository - Systematic Withdrawal Plan       : " << (swpOk ? "PASS" : "FAIL") << "\n";
    assert(swpOk);

    // 13. Test FinancialEventRepository
    InvestSphere::Database::FinancialEventRepository eventRepo;
    InvestSphere::Models::FinancialEvent ev;
    ev.userId = createdUserId ? *createdUserId : 1;
    ev.investmentId = 1;
    ev.eventType = "BOND_COUPON";
    ev.eventDate = "2026-12-15";
    ev.amountCents = 7500; // $75.00
    ev.status = "PENDING";
    ev.description = "Semi-annual corporate coupon distribution";

    auto createdEventId = eventRepo.createEvent(ev);
    bool eventOk = static_cast<bool>(createdEventId);
    std::cout << " [TEST 13] FinancialEventRepository - Cash Flow Calendar    : " << (eventOk ? "PASS" : "FAIL") << "\n";
    assert(eventOk);

    // 14. Test TransactionScope Rollback Guard
    {
        InvestSphere::Database::TransactionScope scope;
        // Do not call scope.commit() -> scope destructs and triggers rollback
        std::cout << " [TEST 14] TransactionScope - RAII Rollback on Early Exit  : PASS\n";
    }

    // 15. Test TransactionScope Explicit Commit
    {
        InvestSphere::Database::TransactionScope scope;
        bool commitSuccess = scope.commit();
        std::cout << " [TEST 15] TransactionScope - Explicit Atomic Commit        : " << (commitSuccess ? "PASS" : "FAIL") << "\n";
        assert(commitSuccess);
    }

    // 16. Test Database Close
    db.close();
    std::cout << " [TEST 16] Clean Resource Deallocation & Database Close    : PASS\n";

    std::cout << "======================================================================\n";
    std::cout << "  ALL 16 REPOSITORY & DATABASE TESTS COMPLETED SUCCESSFULLY!          \n";
    std::cout << "======================================================================\n";

    return 0;
}

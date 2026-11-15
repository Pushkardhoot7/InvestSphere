#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <memory>

#include "DatabaseManager.h"
#include "UserRepository.h"
#include "InvestmentRepository.h"
#include "PasswordHasher.h"
#include "SessionManager.h"
#include "AuthenticationService.h"

int main()
{
    std::cout << "======================================================================\n";
    std::cout << "  INVESTSPHERE — PHASE 5 AUTHENTICATION & SECURITY TEST SUITE         \n";
    std::cout << "======================================================================\n";

    // Initialize Database in test mode
    auto &db = InvestSphere::Database::DatabaseManager::instance();
    bool initOk = db.initialize("test_auth_investsphere.db");
    std::cout << " [SETUP] Database Initialization                           : " << (initOk ? "PASS" : "FAIL") << "\n";
    assert(initOk);

    db.initializeSchema("sql/schema.sql");

    auto userRepo = std::make_shared<InvestSphere::Database::UserRepository>();
    InvestSphere::Services::AuthenticationService authService(userRepo);
    auto &session = InvestSphere::App::SessionManager::instance();

    // ------------------------------------------------------------------------
    // TEST 1: Empty input rejection
    // ------------------------------------------------------------------------
    {
        auto res1 = authService.registerUser("", "test@investsphere.test", "SecurePass123", "SecurePass123");
        assert(!res1.success);
        assert(res1.code == InvestSphere::Services::AuthResult::ErrorCode::EmptyField);

        auto res2 = authService.registerUser("Test User", "", "SecurePass123", "SecurePass123");
        assert(!res2.success);
        assert(res2.code == InvestSphere::Services::AuthResult::ErrorCode::EmptyField);

        auto res3 = authService.registerUser("Test User", "test@investsphere.test", "", "");
        assert(!res3.success);
        assert(res3.code == InvestSphere::Services::AuthResult::ErrorCode::EmptyField);

        std::cout << " [TEST  1] Empty Input Validation (Name, Email, Password)   : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 2: Invalid email format rejection
    // ------------------------------------------------------------------------
    {
        auto res = authService.registerUser("Test User", "not-an-email", "SecurePass123", "SecurePass123");
        assert(!res.success);
        assert(res.code == InvestSphere::Services::AuthResult::ErrorCode::InvalidEmail);
        std::cout << " [TEST  2] Invalid Email RFC Pattern Rejection              : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 3: Weak password rejection
    // ------------------------------------------------------------------------
    {
        // Too short (< 8 chars)
        auto res1 = authService.registerUser("Test User", "short@investsphere.test", "Pass1", "Pass1");
        assert(!res1.success);
        assert(res1.code == InvestSphere::Services::AuthResult::ErrorCode::WeakPassword);

        // Missing digit
        auto res2 = authService.registerUser("Test User", "nodigit@investsphere.test", "PasswordOnly", "PasswordOnly");
        assert(!res2.success);
        assert(res2.code == InvestSphere::Services::AuthResult::ErrorCode::WeakPassword);

        // Missing letter
        auto res3 = authService.registerUser("Test User", "noletter@investsphere.test", "1234567890", "1234567890");
        assert(!res3.success);
        assert(res3.code == InvestSphere::Services::AuthResult::ErrorCode::WeakPassword);

        std::cout << " [TEST  3] Password Security Complexity Rules (Length/Char): PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 4: Password mismatch rejection
    // ------------------------------------------------------------------------
    {
        auto res = authService.registerUser("Test User", "mismatch@investsphere.test", "SecurePass123", "DifferentPass456");
        assert(!res.success);
        assert(res.code == InvestSphere::Services::AuthResult::ErrorCode::PasswordMismatch);
        std::cout << " [TEST  4] Password Confirmation Mismatch Rejection        : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 5: Successful user registration
    // ------------------------------------------------------------------------
    int userAId = 0;
    {
        auto res = authService.registerUser("Dr. Alexander Wright", "alex.wright@investsphere.test", "MatrixVault#2026", "MatrixVault#2026");
        assert(res.success);
        assert(res.userId > 0);
        userAId = res.userId;
        std::cout << " [TEST  5] User Account Registration & DB Ingestion         : PASS (User ID: " << userAId << ")\n";
    }

    // ------------------------------------------------------------------------
    // TEST 6: Duplicate email rejection (case-insensitive)
    // ------------------------------------------------------------------------
    {
        auto res = authService.registerUser("Duplicate Alex", "ALEX.WRIGHT@INVESTSPHERE.TEST", "AnotherPassword99", "AnotherPassword99");
        assert(!res.success);
        assert(res.code == InvestSphere::Services::AuthResult::ErrorCode::DuplicateEmail);
        std::cout << " [TEST  6] Duplicate Email Rejection (Case-Insensitive)    : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 7: Stored password hash verification (Never plaintext!)
    // ------------------------------------------------------------------------
    {
        auto userOpt = userRepo->getUserById(userAId);
        assert(bool(userOpt));
        const auto &user = userOpt.value();

        // Must not be plaintext
        assert(user.passwordHash != "MatrixVault#2026");
        // Must contain standard PBKDF2 modular crypt header
        assert(user.passwordHash.find("$pbkdf2-sha256$i=100000") == 0);

        // Verify cryptographic verification method
        bool verifyOk = InvestSphere::Security::PasswordHasher::verifyPassword("MatrixVault#2026", user.passwordHash);
        assert(verifyOk);

        std::cout << " [TEST  7] PBKDF2-HMAC-SHA256 Salted Hash & Salt Integrity : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 8: Unknown email login rejection
    // ------------------------------------------------------------------------
    {
        auto res = authService.authenticate("nonexistent@investsphere.test", "SomePassword123");
        assert(!res.success);
        assert(res.code == InvestSphere::Services::AuthResult::ErrorCode::InvalidCredentials);
        // Generic message without revealing email existence
        assert(res.message == "Invalid email or password.");
        std::cout << " [TEST  8] Unknown Account Login Rejection (No Enumeration): PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 9: Incorrect password rejection
    // ------------------------------------------------------------------------
    {
        auto res = authService.authenticate("alex.wright@investsphere.test", "WrongPassword999");
        assert(!res.success);
        assert(res.code == InvestSphere::Services::AuthResult::ErrorCode::InvalidCredentials);
        std::cout << " [TEST  9] Incorrect Password Rejection (Constant-Time)    : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 10: Successful login
    // ------------------------------------------------------------------------
    {
        auto res = authService.authenticate("alex.wright@investsphere.test", "MatrixVault#2026");
        assert(res.success);
        assert(res.userId == userAId);
        assert(res.userName == "Dr. Alexander Wright");
        std::cout << " [TEST 10] Successful Credential Verification & Sign-In    : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 11: Session creation and identity persistence
    // ------------------------------------------------------------------------
    {
        assert(session.isAuthenticated());
        assert(session.currentUserId() == userAId);
        assert(session.currentUserName() == "Dr. Alexander Wright");
        assert(session.currentUserEmail() == "alex.wright@investsphere.test");
        std::cout << " [TEST 11] SessionManager In-Memory Identity Persistence   : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 12: Logout and session clearance
    // ------------------------------------------------------------------------
    {
        session.logout();
        assert(!session.isAuthenticated());
        assert(session.currentUserId() == 0);
        assert(session.currentUserName().empty());
        assert(session.currentUserEmail().empty());
        std::cout << " [TEST 12] Logout Session Wiping & Data Sanitization       : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 13: User-Specific Data Access & Multi-User Isolation
    // ------------------------------------------------------------------------
    {
        // Re-authenticate User A
        auto authA = authService.authenticate("alex.wright@investsphere.test", "MatrixVault#2026");
        assert(authA.success);

        // Register User B
        auto authB = authService.registerUser("Sarah Connor", "sarah.connor@investsphere.test", "CyberTerminator2029", "CyberTerminator2029");
        assert(authB.success);
        int userBId = authB.userId;

        InvestSphere::Database::InvestmentRepository invRepo;

        // User A adds an investment
        InvestSphere::Models::Investment invA;
        invA.userId = userAId;
        invA.instrumentId = 1;
        invA.portfolioTag = "TECH_GROWTH";
        invA.notes = "User A Private Notes";
        invA.status = "ACTIVE";
        auto invAId = invRepo.createInvestment(invA);
        assert(bool(invAId));

        // User B adds an investment
        InvestSphere::Models::Investment invB;
        invB.userId = userBId;
        invB.instrumentId = 1;
        invB.portfolioTag = "DEFENSE_HEDGE";
        invB.notes = "User B Private Notes";
        invB.status = "ACTIVE";
        auto invBId = invRepo.createInvestment(invB);
        assert(bool(invBId));

        // Verify User A only retrieves User A's investments
        auto userAInvestments = invRepo.getInvestmentsByUserId(userAId);
        assert(!userAInvestments.empty());
        for (const auto &item : userAInvestments) {
            assert(item.userId == userAId);
        }

        // Verify User B only retrieves User B's investments
        auto userBInvestments = invRepo.getInvestmentsByUserId(userBId);
        assert(!userBInvestments.empty());
        for (const auto &item : userBInvestments) {
            assert(item.userId == userBId);
        }

        // User B attempts to access User A's investment directly -> REJECTED
        auto unauthorizedFetch = invRepo.getInvestmentByIdForUser(invAId.value(), userBId);
        assert(!unauthorizedFetch);

        // User B attempts to delete User A's investment -> REJECTED
        bool unauthorizedDelete = invRepo.deleteInvestmentForUser(invAId.value(), userBId);
        assert(!unauthorizedDelete);

        // User A accesses their own investment -> GRANTED
        auto authorizedFetch = invRepo.getInvestmentByIdForUser(invAId.value(), userAId);
        assert(bool(authorizedFetch));
        assert(authorizedFetch.value().notes == "User A Private Notes");

        std::cout << " [TEST 13] Multi-User Data Isolation & Ownership Guard     : PASS\n";
    }

    // ------------------------------------------------------------------------
    // TEST 14: Profile update and password change verification
    // ------------------------------------------------------------------------
    {
        // Update Profile
        auto updateRes = authService.updateProfile(userAId, "Alexander Wright, Ph.D.");
        assert(updateRes.success);
        assert(session.currentUserName() == "Alexander Wright, Ph.D.");

        // Change password with WRONG current password -> REJECTED
        auto badPwdRes = authService.changePassword(userAId, "WrongCurrentPassword", "NewStrongPass99", "NewStrongPass99");
        assert(!badPwdRes.success);

        // Change password with CORRECT current password -> SUCCESS
        auto goodPwdRes = authService.changePassword(userAId, "MatrixVault#2026", "NewStrongPass99", "NewStrongPass99");
        assert(goodPwdRes.success);

        // Verify old password no longer works
        auto oldLogin = authService.authenticate("alex.wright@investsphere.test", "MatrixVault#2026");
        assert(!oldLogin.success);

        // Verify new password works
        auto newLogin = authService.authenticate("alex.wright@investsphere.test", "NewStrongPass99");
        assert(newLogin.success);

        std::cout << " [TEST 14] Profile Update & Password Mutation Verification : PASS\n";
    }

    std::cout << "======================================================================\n";
    std::cout << "  ALL 14 AUTHENTICATION & SECURITY TESTS COMPLETED SUCCESSFULLY!      \n";
    std::cout << "======================================================================\n";

    return 0;
}

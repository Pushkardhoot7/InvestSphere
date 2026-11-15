#include "AuthenticationService.h"
#include "PasswordHasher.h"

#include <regex>
#include <algorithm>
#include <cctype>

namespace InvestSphere {
namespace Services {

namespace {

std::string trim(const std::string &str)
{
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    return (start < end ? std::string(start, end) : std::string());
}

} // anonymous namespace

AuthenticationService::AuthenticationService(std::shared_ptr<Database::UserRepository> userRepo)
    : m_userRepo(userRepo ? userRepo : std::make_shared<Database::UserRepository>())
{
}

bool AuthenticationService::isValidEmailFormat(const std::string &email)
{
    static const std::regex emailPattern(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );
    return std::regex_match(email, emailPattern);
}

bool AuthenticationService::isStrongPassword(const std::string &password, std::string &feedback)
{
    if (password.length() < 8) {
        feedback = "Password must be at least 8 characters long.";
        return false;
    }

    bool hasDigit = false;
    bool hasLetter = false;

    for (char ch : password) {
        if (std::isdigit(static_cast<unsigned char>(ch))) hasDigit = true;
        if (std::isalpha(static_cast<unsigned char>(ch))) hasLetter = true;
    }

    if (!hasLetter) {
        feedback = "Password must contain at least one alphabetical letter.";
        return false;
    }
    if (!hasDigit) {
        feedback = "Password must contain at least one numeric digit.";
        return false;
    }

    return true;
}

bool AuthenticationService::isEmailAvailable(const std::string &email)
{
    std::string cleanEmail = trim(email);
    if (cleanEmail.empty() || !m_userRepo) return false;
    auto existing = m_userRepo->getUserByEmail(cleanEmail);
    return !existing;
}

AuthResult AuthenticationService::registerUser(const std::string &fullName,
                                              const std::string &email,
                                              const std::string &password,
                                              const std::string &confirmPassword)
{
    AuthResult result;
    std::string cleanName = trim(fullName);
    std::string cleanEmail = trim(email);

    // 1. Check for empty fields
    if (cleanName.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "Full name is required.";
        return result;
    }
    if (cleanEmail.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "Email address is required.";
        return result;
    }
    if (password.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "Password is required.";
        return result;
    }

    // 2. Validate Email Format
    if (!isValidEmailFormat(cleanEmail)) {
        result.code = AuthResult::ErrorCode::InvalidEmail;
        result.message = "Please enter a valid email address (e.g. user@example.com).";
        return result;
    }

    // 3. Validate Password Security
    std::string passwordFeedback;
    if (!isStrongPassword(password, passwordFeedback)) {
        result.code = AuthResult::ErrorCode::WeakPassword;
        result.message = passwordFeedback;
        return result;
    }

    // 4. Validate Password Confirmation Match
    if (password != confirmPassword) {
        result.code = AuthResult::ErrorCode::PasswordMismatch;
        result.message = "Passwords do not match. Please verify and retype.";
        return result;
    }

    // 5. Enforce Unique Email
    if (!isEmailAvailable(cleanEmail)) {
        result.code = AuthResult::ErrorCode::DuplicateEmail;
        result.message = "An account with this email address already exists.";
        return result;
    }

    // 6. Secure Salted PBKDF2 Hashing
    std::string passwordHash = Security::PasswordHasher::hashPassword(password);

    // 7. Persist to Database
    Models::User newUser;
    newUser.fullName = cleanName;
    newUser.email = cleanEmail;
    newUser.passwordHash = passwordHash;
    newUser.accountStatus = "ACTIVE";

    auto createdId = m_userRepo->createUser(newUser);
    if (!createdId) {
        result.code = AuthResult::ErrorCode::DatabaseError;
        result.message = "Database error: unable to create user account. Please try again.";
        return result;
    }

    result.success = true;
    result.code = AuthResult::ErrorCode::None;
    result.userId = createdId.value();
    result.userName = cleanName;
    result.userEmail = cleanEmail;
    result.message = "Account created successfully!";

    // Automatically establish session
    newUser.id = createdId.value();
    App::SessionManager::instance().startSession(newUser);

    return result;
}

AuthResult AuthenticationService::authenticate(const std::string &email, const std::string &password)
{
    AuthResult result;
    std::string cleanEmail = trim(email);

    if (cleanEmail.empty() || password.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "Email and password are required.";
        return result;
    }

    if (!m_userRepo) {
        result.code = AuthResult::ErrorCode::DatabaseError;
        result.message = "Database connection unavailable.";
        return result;
    }

    // Security practice: Always use a generic failure message to prevent user enumeration
    const std::string genericError = "Invalid email or password.";

    auto userOpt = m_userRepo->getUserByEmail(cleanEmail);
    if (!userOpt) {
        result.code = AuthResult::ErrorCode::InvalidCredentials;
        result.message = genericError;
        return result;
    }

    const auto &user = userOpt.value();

    // Verify password hash in constant time
    if (!Security::PasswordHasher::verifyPassword(password, user.passwordHash)) {
        result.code = AuthResult::ErrorCode::InvalidCredentials;
        result.message = genericError;
        return result;
    }

    // Check account status
    if (user.accountStatus == "SUSPENDED" || user.accountStatus == "CLOSED") {
        result.code = AuthResult::ErrorCode::AccountSuspended;
        result.message = "Your account has been deactivated or suspended. Please contact support.";
        return result;
    }

    // Authentication succeeded
    result.success = true;
    result.code = AuthResult::ErrorCode::None;
    result.userId = user.id;
    result.userName = user.fullName;
    result.userEmail = user.email;
    result.message = "Login successful.";

    // Initialize authenticated session
    App::SessionManager::instance().startSession(user);

    return result;
}

AuthResult AuthenticationService::changePassword(int userId,
                                                const std::string &currentPassword,
                                                const std::string &newPassword,
                                                const std::string &confirmNewPassword)
{
    AuthResult result;

    if (currentPassword.empty() || newPassword.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "All password fields are required.";
        return result;
    }

    if (newPassword != confirmNewPassword) {
        result.code = AuthResult::ErrorCode::PasswordMismatch;
        result.message = "New password and confirmation do not match.";
        return result;
    }

    std::string feedback;
    if (!isStrongPassword(newPassword, feedback)) {
        result.code = AuthResult::ErrorCode::WeakPassword;
        result.message = feedback;
        return result;
    }

    auto userOpt = m_userRepo->getUserById(userId);
    if (!userOpt) {
        result.code = AuthResult::ErrorCode::InvalidCredentials;
        result.message = "User not found.";
        return result;
    }

    const auto &user = userOpt.value();
    if (!Security::PasswordHasher::verifyPassword(currentPassword, user.passwordHash)) {
        result.code = AuthResult::ErrorCode::InvalidCredentials;
        result.message = "Current password is incorrect.";
        return result;
    }

    std::string newHash = Security::PasswordHasher::hashPassword(newPassword);
    if (!m_userRepo->updatePassword(userId, newHash)) {
        result.code = AuthResult::ErrorCode::DatabaseError;
        result.message = "Failed to update password in database.";
        return result;
    }

    result.success = true;
    result.message = "Password updated successfully.";
    return result;
}

AuthResult AuthenticationService::updateProfile(int userId, const std::string &fullName)
{
    AuthResult result;
    std::string cleanName = trim(fullName);

    if (cleanName.empty()) {
        result.code = AuthResult::ErrorCode::EmptyField;
        result.message = "Full name cannot be empty.";
        return result;
    }

    auto userOpt = m_userRepo->getUserById(userId);
    if (!userOpt) {
        result.code = AuthResult::ErrorCode::InvalidCredentials;
        result.message = "User not found.";
        return result;
    }

    if (!m_userRepo->updateProfile(userId, cleanName, userOpt.value().accountStatus)) {
        result.code = AuthResult::ErrorCode::DatabaseError;
        result.message = "Failed to update profile.";
        return result;
    }

    App::SessionManager::instance().updateProfile(cleanName);

    result.success = true;
    result.userName = cleanName;
    result.message = "Profile updated successfully.";
    return result;
}

} // namespace Services
} // namespace InvestSphere

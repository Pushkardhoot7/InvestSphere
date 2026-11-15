#ifndef INVESTSPHERE_AUTHENTICATIONSERVICE_H
#define INVESTSPHERE_AUTHENTICATIONSERVICE_H

#include "UserModel.h"
#include "UserRepository.h"
#include "SessionManager.h"
#include <string>
#include <memory>

namespace InvestSphere {
namespace Services {

/**
 * @brief Authentication Result structure conveying status, error codes, and user info.
 */
struct AuthResult {
    enum class ErrorCode {
        None,
        EmptyField,
        InvalidEmail,
        WeakPassword,
        PasswordMismatch,
        DuplicateEmail,
        InvalidCredentials,
        AccountSuspended,
        DatabaseError
    };

    bool success{false};
    ErrorCode code{ErrorCode::None};
    std::string message;
    int userId{0};
    std::string userName;
    std::string userEmail;
};

/**
 * @brief Domain Authentication and Account Security Service.
 * Coordinates input validation, PBKDF2 hashing, database user persistence,
 * and session state management.
 */
class AuthenticationService {
public:
    explicit AuthenticationService(std::shared_ptr<Database::UserRepository> userRepo = nullptr);

    /**
     * @brief Registers a new user account with validated input and PBKDF2 hashed password.
     */
    AuthResult registerUser(const std::string &fullName,
                           const std::string &email,
                           const std::string &password,
                           const std::string &confirmPassword);

    /**
     * @brief Authenticates an existing user and initializes the active session on success.
     */
    AuthResult authenticate(const std::string &email, const std::string &password);

    /**
     * @brief Changes password for the authenticated user, strictly verifying the current password.
     */
    AuthResult changePassword(int userId,
                             const std::string &currentPassword,
                             const std::string &newPassword,
                             const std::string &confirmNewPassword);

    /**
     * @brief Updates permitted profile fields (e.g. full name) for the authenticated user.
     */
    AuthResult updateProfile(int userId, const std::string &fullName);

    /**
     * @brief Checks if an email address is already registered (case-insensitive).
     */
    bool isEmailAvailable(const std::string &email);

    /**
     * @brief Helper to validate email RFC format.
     */
    static bool isValidEmailFormat(const std::string &email);

    /**
     * @brief Helper to validate minimum password complexity (>= 8 chars, >= 1 digit, >= 1 letter).
     */
    static bool isStrongPassword(const std::string &password, std::string &feedback);

private:
    std::shared_ptr<Database::UserRepository> m_userRepo;
};

} // namespace Services
} // namespace InvestSphere

#endif // INVESTSPHERE_AUTHENTICATIONSERVICE_H

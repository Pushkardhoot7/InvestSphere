#ifndef INVESTSPHERE_PASSWORDHASHER_H
#define INVESTSPHERE_PASSWORDHASHER_H

#include <string>
#include <vector>
#include <cstdint>

namespace InvestSphere {
namespace Security {

/**
 * @brief Enterprise-grade Password Hashing and Verification Service.
 * Implements PBKDF2-HMAC-SHA256 with cryptographically random salts and 100,000 iterations
 * following NIST SP 800-132 and OWASP Password Storage Guidelines.
 * Encoded format: $pbkdf2-sha256$i=100000,l=32$<salt_hex>$<hash_hex>
 */
class PasswordHasher {
public:
    static constexpr int DefaultIterations = 100000;
    static constexpr size_t DefaultSaltBytes = 16;
    static constexpr size_t DefaultHashBytes = 32;

    /**
     * @brief Computes a secure, salted PBKDF2-HMAC-SHA256 password hash.
     * @param password Plaintext password to hash.
     * @param iterations Number of PBKDF2 rounds (default: 100,000).
     * @return Formatted modular crypt hash string containing algorithm, parameters, salt, and digest.
     */
    static std::string hashPassword(const std::string &password, int iterations = DefaultIterations);

    /**
     * @brief Verifies a plaintext password against a stored encoded hash in constant time.
     * @param password Plaintext password candidate.
     * @param storedHash The encoded hash retrieved from the database.
     * @return true if password matches, false otherwise.
     */
    static bool verifyPassword(const std::string &password, const std::string &storedHash);

    /**
     * @brief Generates cryptographically secure random bytes as a hex string.
     */
    static std::string generateRandomSaltHex(size_t numBytes = DefaultSaltBytes);

    /**
     * @brief Constant-time string equality comparison to prevent timing attacks.
     */
    static bool constantTimeEquals(const std::string &a, const std::string &b);

    /**
     * @brief Core PBKDF2-HMAC-SHA256 key derivation.
     */
    static std::vector<uint8_t> deriveKey(const std::string &password,
                                         const std::vector<uint8_t> &salt,
                                         int iterations,
                                         size_t keyLength);

private:
    static std::string toHex(const std::vector<uint8_t> &data);
    static std::vector<uint8_t> fromHex(const std::string &hex);
};

} // namespace Security
} // namespace InvestSphere

#endif // INVESTSPHERE_PASSWORDHASHER_H

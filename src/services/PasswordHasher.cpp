#include "PasswordHasher.h"

#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <array>
#include <algorithm>

namespace InvestSphere {
namespace Security {

namespace {

// ============================================================================
// Standard NIST FIPS 180-4 SHA-256 Implementation
// ============================================================================

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t ep0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t ep1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t sig0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t sig1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

struct Sha256Context {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
};

void sha256_init(Sha256Context &ctx) {
    ctx.state[0] = 0x6a09e667;
    ctx.state[1] = 0xbb67ae85;
    ctx.state[2] = 0x3c6ef372;
    ctx.state[3] = 0xa54ff53a;
    ctx.state[4] = 0x510e527f;
    ctx.state[5] = 0x9b05688c;
    ctx.state[6] = 0x1f83d9ab;
    ctx.state[7] = 0x5be0cd19;
    ctx.count = 0;
}

void sha256_transform(Sha256Context &ctx, const uint8_t data[64]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (static_cast<uint32_t>(data[i * 4]) << 24) |
               (static_cast<uint32_t>(data[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(data[i * 4 + 2]) << 8) |
               (static_cast<uint32_t>(data[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];
    }

    uint32_t a = ctx.state[0];
    uint32_t b = ctx.state[1];
    uint32_t c = ctx.state[2];
    uint32_t d = ctx.state[3];
    uint32_t e = ctx.state[4];
    uint32_t f = ctx.state[5];
    uint32_t g = ctx.state[6];
    uint32_t h = ctx.state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch(e, f, g) + K256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
    ctx.state[4] += e;
    ctx.state[5] += f;
    ctx.state[6] += g;
    ctx.state[7] += h;
}

void sha256_update(Sha256Context &ctx, const uint8_t *data, size_t len) {
    size_t buffer_idx = static_cast<size_t>(ctx.count & 63);
    ctx.count += len;

    if (buffer_idx > 0) {
        size_t copy_len = 64 - buffer_idx;
        if (copy_len > len) {
            std::memcpy(&ctx.buffer[buffer_idx], data, len);
            return;
        }
        std::memcpy(&ctx.buffer[buffer_idx], data, copy_len);
        sha256_transform(ctx, ctx.buffer);
        data += copy_len;
        len -= copy_len;
    }

    while (len >= 64) {
        sha256_transform(ctx, data);
        data += 64;
        len -= 64;
    }

    if (len > 0) {
        std::memcpy(ctx.buffer, data, len);
    }
}

void sha256_final(Sha256Context &ctx, uint8_t digest[32]) {
    uint8_t pad = 0x80;
    uint64_t total_bits = ctx.count * 8;

    sha256_update(ctx, &pad, 1);
    uint8_t zero = 0;
    while ((ctx.count & 63) != 56) {
        sha256_update(ctx, &zero, 1);
    }

    uint8_t len_bytes[8];
    for (int i = 0; i < 8; ++i) {
        len_bytes[i] = static_cast<uint8_t>(total_bits >> ((7 - i) * 8));
    }
    sha256_update(ctx, len_bytes, 8);

    for (int i = 0; i < 8; ++i) {
        digest[i * 4]     = static_cast<uint8_t>(ctx.state[i] >> 24);
        digest[i * 4 + 1] = static_cast<uint8_t>(ctx.state[i] >> 16);
        digest[i * 4 + 2] = static_cast<uint8_t>(ctx.state[i] >> 8);
        digest[i * 4 + 3] = static_cast<uint8_t>(ctx.state[i]);
    }
}

std::vector<uint8_t> sha256(const uint8_t *data, size_t len) {
    Sha256Context ctx;
    sha256_init(ctx);
    sha256_update(ctx, data, len);
    std::vector<uint8_t> digest(32);
    sha256_final(ctx, digest.data());
    return digest;
}

// ============================================================================
// RFC 2104 HMAC-SHA256 Implementation
// ============================================================================

std::vector<uint8_t> hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len) {
    uint8_t k_block[64];
    std::memset(k_block, 0, 64);

    if (key_len > 64) {
        auto hashed = sha256(key, key_len);
        std::memcpy(k_block, hashed.data(), 32);
    } else {
        std::memcpy(k_block, key, key_len);
    }

    uint8_t o_key_pad[64];
    uint8_t i_key_pad[64];
    for (int i = 0; i < 64; ++i) {
        o_key_pad[i] = k_block[i] ^ 0x5c;
        i_key_pad[i] = k_block[i] ^ 0x36;
    }

    Sha256Context inner_ctx;
    sha256_init(inner_ctx);
    sha256_update(inner_ctx, i_key_pad, 64);
    sha256_update(inner_ctx, data, data_len);
    uint8_t inner_hash[32];
    sha256_final(inner_ctx, inner_hash);

    Sha256Context outer_ctx;
    sha256_init(outer_ctx);
    sha256_update(outer_ctx, o_key_pad, 64);
    sha256_update(outer_ctx, inner_hash, 32);
    std::vector<uint8_t> result(32);
    sha256_final(outer_ctx, result.data());

    return result;
}

} // anonymous namespace

// ============================================================================
// PasswordHasher Implementation
// ============================================================================

std::string PasswordHasher::toHex(const std::vector<uint8_t> &data)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t b : data) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

std::vector<uint8_t> PasswordHasher::fromHex(const std::string &hex)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 1 < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        uint8_t byteVal = static_cast<uint8_t>(std::strtoul(byteStr.c_str(), nullptr, 16));
        bytes.push_back(byteVal);
    }
    return bytes;
}

std::string PasswordHasher::generateRandomSaltHex(size_t numBytes)
{
    std::vector<uint8_t> salt(numBytes);
    std::random_device rd;
    for (size_t i = 0; i < numBytes; ++i) {
        salt[i] = static_cast<uint8_t>(rd() & 0xFF);
    }
    return toHex(salt);
}

std::vector<uint8_t> PasswordHasher::deriveKey(const std::string &password,
                                               const std::vector<uint8_t> &salt,
                                               int iterations,
                                               size_t keyLength)
{
    // RFC 2898 PBKDF2 implementation with HMAC-SHA-256
    std::vector<uint8_t> derivedKey;
    derivedKey.reserve(keyLength);

    uint32_t blockIndex = 1;
    const uint8_t *pwdData = reinterpret_cast<const uint8_t*>(password.data());
    size_t pwdLen = password.length();

    while (derivedKey.size() < keyLength) {
        // U1 = HMAC-SHA256(password, salt || INT_32_BE(blockIndex))
        std::vector<uint8_t> saltAndBlock = salt;
        saltAndBlock.push_back(static_cast<uint8_t>((blockIndex >> 24) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>((blockIndex >> 16) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>((blockIndex >> 8) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>(blockIndex & 0xFF));

        std::vector<uint8_t> u = hmac_sha256(pwdData, pwdLen, saltAndBlock.data(), saltAndBlock.size());
        std::vector<uint8_t> f = u;

        // U2 .. U_iterations
        for (int iter = 1; iter < iterations; ++iter) {
            u = hmac_sha256(pwdData, pwdLen, u.data(), u.size());
            for (size_t k = 0; k < 32; ++k) {
                f[k] ^= u[k];
            }
        }

        size_t copyLen = std::min<size_t>(keyLength - derivedKey.size(), 32);
        derivedKey.insert(derivedKey.end(), f.begin(), f.begin() + copyLen);
        blockIndex++;
    }

    return derivedKey;
}

std::string PasswordHasher::hashPassword(const std::string &password, int iterations)
{
    std::string saltHex = generateRandomSaltHex(DefaultSaltBytes);
    std::vector<uint8_t> saltBytes = fromHex(saltHex);

    std::vector<uint8_t> derived = deriveKey(password, saltBytes, iterations, DefaultHashBytes);
    std::string hashHex = toHex(derived);

    // Format: $pbkdf2-sha256$i=100000,l=32$<salt_hex>$<hash_hex>
    std::ostringstream oss;
    oss << "$pbkdf2-sha256$i=" << iterations << ",l=" << DefaultHashBytes << "$" << saltHex << "$" << hashHex;
    return oss.str();
}

bool PasswordHasher::constantTimeEquals(const std::string &a, const std::string &b)
{
    if (a.length() != b.length()) {
        return false;
    }
    volatile uint8_t diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= static_cast<uint8_t>(a[i] ^ b[i]);
    }
    return diff == 0;
}

bool PasswordHasher::verifyPassword(const std::string &password, const std::string &storedHash)
{
    // Check format: $pbkdf2-sha256$i=<iter>,l=<len>$<salt>$<hash>
    if (storedHash.rfind("$pbkdf2-sha256$", 0) != 0) {
        return false;
    }

    // Split components by '$'
    std::vector<std::string> parts;
    std::stringstream ss(storedHash);
    std::string item;
    while (std::getline(ss, item, '$')) {
        if (!item.empty()) {
            parts.push_back(item);
        }
    }

    // Expected parts: ["pbkdf2-sha256", "i=100000,l=32", "<salt_hex>", "<hash_hex>"]
    if (parts.size() < 4) {
        return false;
    }

    int iterations = DefaultIterations;
    size_t keyLength = DefaultHashBytes;

    // Parse parameters
    const std::string &params = parts[1];
    auto iPos = params.find("i=");
    if (iPos != std::string::npos) {
        iterations = std::stoi(params.substr(iPos + 2));
    }
    auto lPos = params.find("l=");
    if (lPos != std::string::npos) {
        keyLength = static_cast<size_t>(std::stoi(params.substr(lPos + 2)));
    }

    const std::string &saltHex = parts[2];
    const std::string &expectedHashHex = parts[3];

    std::vector<uint8_t> saltBytes = fromHex(saltHex);
    std::vector<uint8_t> derived = deriveKey(password, saltBytes, iterations, keyLength);
    std::string actualHashHex = toHex(derived);

    return constantTimeEquals(actualHashHex, expectedHashHex);
}

} // namespace Security
} // namespace InvestSphere

#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "UserModel.h"
#include "Compat.h"
#include <vector>
#include <string>

namespace InvestSphere {
namespace Database {

class UserRepository {
public:
    UserRepository() = default;

    InvestSphere::Optional<int> createUser(const Models::User &user);
    InvestSphere::Optional<Models::User> getUserById(int userId);
    InvestSphere::Optional<Models::User> getUserByEmail(const std::string &email);
    bool updateProfile(int userId, const std::string &fullName, const std::string &status);
    bool updatePassword(int userId, const std::string &newPasswordHash);
    bool emailExists(const std::string &email);
    std::vector<Models::User> getAllUsers();

    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace Database
} // namespace InvestSphere

#endif // USERREPOSITORY_H

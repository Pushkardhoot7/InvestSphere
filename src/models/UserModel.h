#ifndef USERMODEL_H
#define USERMODEL_H

#include <string>

namespace InvestSphere {
namespace Models {

struct User {
    int id{0};
    std::string fullName;
    std::string email;
    std::string passwordHash;
    std::string accountStatus{"ACTIVE"};
    std::string createdAt;
    std::string updatedAt;

    bool isValid() const {
        return !fullName.empty() && !email.empty() && !passwordHash.empty();
    }
};

} // namespace Models
} // namespace InvestSphere

#endif // USERMODEL_H

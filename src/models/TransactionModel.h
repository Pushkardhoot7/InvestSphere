#ifndef TRANSACTIONMODEL_H
#define TRANSACTIONMODEL_H

#include "Compat.h"
#include <cstdint>
#include <string>

namespace InvestSphere {
namespace Models {

struct Transaction {
    int id{0};
    int userId{0};
    int investmentId{0};
    std::string transactionType{"BUY"}; // BUY, SELL, DEPOSIT, WITHDRAWAL, DIVIDEND, INTEREST, FEE, REDEMPTION
    std::string transactionDate;
    double quantity{0.0};
    int64_t unitPriceCents{0};
    int64_t grossAmountCents{0};
    int64_t feesCents{0};
    int64_t taxesCents{0};
    int64_t netAmountCents{0};
    std::string notes;
    std::string createdAt;

    // Joined view attributes
    std::string symbol;
    std::string instrumentName;
    std::string categoryCode;
    std::string categoryName;
};

} // namespace Models
} // namespace InvestSphere

#endif // TRANSACTIONMODEL_H

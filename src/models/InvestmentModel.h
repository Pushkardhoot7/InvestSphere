#ifndef INVESTMENTMODEL_H
#define INVESTMENTMODEL_H

#include "Compat.h"
#include <cstdint>

namespace InvestSphere {
namespace Models {

struct Investment {
    int id{0};
    int userId{0};
    int instrumentId{0};
    std::string portfolioTag{"DEFAULT"};
    std::string notes;
    std::string status{"ACTIVE"};
    std::string createdAt;
    std::string updatedAt;

    // Joined view attributes
    std::string symbol;
    std::string instrumentName;
    std::string assetTypeCode;
    double remainingQuantity{0.0};
    int64_t netInvestedAmountCents{0};
};

struct FixedDepositDetails {
    int investmentId{0};
    std::string institutionName;
    std::string accountNumber;
    int64_t principalAmountCents{0};
    double annualInterestRate{0.0};
    std::string compoundingFrequency{"QUARTERLY"};
    std::string payoutType{"CUMULATIVE"};
    std::string startDate;
    std::string maturityDate;
    int64_t estimatedMaturityAmountCents{0};
    std::string status{"ACTIVE"};
};

struct BondDetails {
    int investmentId{0};
    std::string issuerName;
    std::string bondType{"GOVERNMENT"};
    int64_t faceValueCents{0};
    double couponRate{0.0};
    std::string couponFrequency{"SEMI_ANNUALLY"};
    std::string issueDate;
    std::string maturityDate;
    std::string creditRating;
    std::string status{"ACTIVE"};
};

struct GoldHoldingDetails {
    int investmentId{0};
    std::string goldForm{"PHYSICAL_COIN"};
    double purityKarat{24.0};
    std::string storageLocation;
};

} // namespace Models
} // namespace InvestSphere

#endif // INVESTMENTMODEL_H

#ifndef INSTRUMENTMODEL_H
#define INSTRUMENTMODEL_H

#include "Compat.h"

namespace InvestSphere {
namespace Models {

struct Instrument {
    int id{0};
    int investmentTypeId{1};
    std::string symbol;
    std::string instrumentName;
    std::string currency{"USD"};
    std::string exchange;
    std::string description;
    bool isActive{true};
    std::string createdAt;
};

struct StockDetails {
    int instrumentId{0};
    std::string isin;
    std::string sector;
    std::string industry;
    std::string marketCapCategory{"LARGE_CAP"};
};

struct MutualFundDetails {
    int instrumentId{0};
    std::string amcName;
    std::string fundCategory{"EQUITY"};
    std::string planType{"GROWTH"};
    std::string benchmarkIndex;
    double expenseRatio{0.0};
};

} // namespace Models
} // namespace InvestSphere

#endif // INSTRUMENTMODEL_H

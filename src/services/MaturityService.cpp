#include "MaturityService.h"
#include "SipRepository.h"
#include "SwpRepository.h"

#include <ctime>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace InvestSphere {
namespace Services {

namespace {

std::tm parseDate(const std::string &dateStr)
{
    std::tm t = {};
    if (dateStr.length() >= 10) {
        int year = 0, month = 0, day = 0;
        char s1 = '-', s2 = '-';
        std::istringstream ss(dateStr.substr(0, 10));
        if (ss >> year >> s1 >> month >> s2 >> day) {
            t.tm_year = year - 1900;
            t.tm_mon  = month - 1;
            t.tm_mday = day;
        }
    }
    return t;
}

} // anonymous namespace

MaturityService& MaturityService::instance()
{
    static MaturityService s_instance;
    return s_instance;
}

int MaturityService::calculateDaysRemaining(const std::string &targetDate, const std::string &currentDate)
{
    if (targetDate.empty()) return 0;

    std::tm targetTm = parseDate(targetDate);
    std::time_t targetTime = std::mktime(&targetTm);
    if (targetTime == -1) return 0;

    std::time_t refTime;
    if (!currentDate.empty()) {
        std::tm curTm = parseDate(currentDate);
        refTime = std::mktime(&curTm);
        if (refTime == -1) refTime = std::time(nullptr);
    } else {
        refTime = std::time(nullptr);
    }

    double diffSeconds = std::difftime(targetTime, refTime);
    // Round to nearest day
    return static_cast<int>(diffSeconds / (60 * 60 * 24));
}

std::vector<MaturityItemDTO> MaturityService::getMaturitySchedule(
    int userId,
    const std::string &windowFilter,
    const std::string &assetClassFilter,
    const std::string &searchQuery,
    const std::string &currentDate)
{
    std::vector<MaturityItemDTO> items;
    Database::InvestmentRepository invRepo;

    auto investments = invRepo.getInvestmentsByUserId(userId);

    for (const auto &inv : investments) {
        // 1. Check Fixed Deposits
        auto fdOpt = invRepo.getFixedDepositDetails(inv.id);
        if (fdOpt) {
            const auto &fd = *fdOpt;
            int days = calculateDaysRemaining(fd.maturityDate, currentDate);

            MaturityItemDTO item;
            item.investmentId = inv.id;
            item.assetType = "FIXED_DEPOSIT";
            item.title = fd.institutionName + " Term Deposit";
            item.institutionOrIssuer = fd.institutionName;
            item.accountOrRating = fd.accountNumber.empty() ? "N/A" : fd.accountNumber;
            item.principalOrFaceValueCents = fd.principalAmountCents;
            item.quantity = 1.0;
            item.totalPrincipalCents = fd.principalAmountCents;
            item.ratePercent = fd.annualInterestRate;
            item.frequency = fd.compoundingFrequency;
            item.payoutType = fd.payoutType;
            item.startDate = fd.startDate;
            item.maturityDate = fd.maturityDate;
            item.daysRemaining = days;
            item.estimatedProceedsCents = fd.estimatedMaturityAmountCents > 0 ? fd.estimatedMaturityAmountCents : fd.principalAmountCents;
            item.hasEstimatedProceeds = true;
            item.status = fd.status;

            if (days < 0 || fd.status == "MATURED") {
                item.windowBadge = "MATURED";
            } else if (days <= 7) {
                item.windowBadge = "7_DAYS";
            } else if (days <= 30) {
                item.windowBadge = "30_DAYS";
            } else if (days <= 90) {
                item.windowBadge = "90_DAYS";
            } else {
                item.windowBadge = "FUTURE";
            }

            items.push_back(item);
        }

        // 2. Check Bonds
        auto bondOpt = invRepo.getBondDetails(inv.id);
        if (bondOpt) {
            const auto &b = *bondOpt;
            int days = calculateDaysRemaining(b.maturityDate, currentDate);
            double qty = inv.remainingQuantity > 0 ? inv.remainingQuantity : 1.0;
            int64_t totalPar = static_cast<int64_t>(b.faceValueCents * qty);

            MaturityItemDTO item;
            item.investmentId = inv.id;
            item.assetType = "BOND";
            item.title = b.issuerName + " (" + b.bondType + ")";
            item.institutionOrIssuer = b.issuerName;
            item.accountOrRating = b.creditRating.empty() ? "NR" : b.creditRating;
            item.principalOrFaceValueCents = b.faceValueCents;
            item.quantity = qty;
            item.totalPrincipalCents = totalPar;
            item.ratePercent = b.couponRate;
            item.frequency = b.couponFrequency;
            item.payoutType = "COUPON";
            item.startDate = b.issueDate;
            item.maturityDate = b.maturityDate;
            item.daysRemaining = days;
            item.estimatedProceedsCents = totalPar; // Par value returned at maturity
            item.hasEstimatedProceeds = true;
            item.status = b.status;

            if (days < 0 || b.status == "REDEEMED") {
                item.windowBadge = "MATURED";
            } else if (days <= 7) {
                item.windowBadge = "7_DAYS";
            } else if (days <= 30) {
                item.windowBadge = "30_DAYS";
            } else if (days <= 90) {
                item.windowBadge = "90_DAYS";
            } else {
                item.windowBadge = "FUTURE";
            }

            items.push_back(item);
        }
    }

    // Apply Filters
    std::vector<MaturityItemDTO> filtered;
    for (const auto &item : items) {
        // Asset class filter
        if (assetClassFilter != "ALL" && !assetClassFilter.empty() && item.assetType != assetClassFilter) {
            continue;
        }

        // Window filter
        if (windowFilter == "7_DAYS" && item.windowBadge != "7_DAYS") {
            continue;
        }
        if (windowFilter == "30_DAYS" && item.windowBadge != "7_DAYS" && item.windowBadge != "30_DAYS") {
            continue;
        }
        if (windowFilter == "90_DAYS" && item.windowBadge != "7_DAYS" && item.windowBadge != "30_DAYS" && item.windowBadge != "90_DAYS") {
            continue;
        }
        if (windowFilter == "ALL_FUTURE" && item.windowBadge == "MATURED") {
            continue;
        }
        if (windowFilter == "MATURED" && item.windowBadge != "MATURED") {
            continue;
        }

        // Search query
        if (!searchQuery.empty()) {
            std::string q = searchQuery;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);
            std::string text = item.title + " " + item.institutionOrIssuer + " " + item.accountOrRating;
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            if (text.find(q) == std::string::npos) {
                continue;
            }
        }

        filtered.push_back(item);
    }

    // Sort by days remaining ascending
    std::sort(filtered.begin(), filtered.end(), [](const MaturityItemDTO &a, const MaturityItemDTO &b) {
        return a.daysRemaining < b.daysRemaining;
    });

    return filtered;
}

MaturitySummaryDTO MaturityService::getMaturitySummary(int userId, const std::string &currentDate)
{
    MaturitySummaryDTO summary;
    auto allItems = getMaturitySchedule(userId, "ALL", "ALL", "", currentDate);

    for (const auto &item : allItems) {
        if (item.daysRemaining < 0 || item.status == "MATURED" || item.status == "REDEEMED") {
            ++summary.maturedCount;
            summary.maturedAmountCents += item.estimatedProceedsCents;
        } else {
            ++summary.totalFutureCount;
            summary.totalFutureAmountCents += item.estimatedProceedsCents;

            if (item.daysRemaining <= 7) {
                ++summary.in7DaysCount;
                summary.in7DaysAmountCents += item.estimatedProceedsCents;
            }
            if (item.daysRemaining <= 30) {
                ++summary.in30DaysCount;
                summary.in30DaysAmountCents += item.estimatedProceedsCents;
            }
            if (item.daysRemaining <= 90) {
                ++summary.in90DaysCount;
                summary.in90DaysAmountCents += item.estimatedProceedsCents;
            }
        }
    }

    return summary;
}

int MaturityService::syncFinancialEventsForUser(int userId, const std::string &currentDate)
{
    Database::FinancialEventRepository eventRepo;
    Database::InvestmentRepository invRepo;
    int eventsCreated = 0;

    auto investments = invRepo.getInvestmentsByUserId(userId);

    for (const auto &inv : investments) {
        // 1. Fixed Deposit maturity event
        auto fdOpt = invRepo.getFixedDepositDetails(inv.id);
        if (fdOpt && fdOpt->status == "ACTIVE") {
            const auto &fd = *fdOpt;
            if (!eventRepo.eventExists(userId, inv.id, "FD_MATURITY", fd.maturityDate)) {
                Models::FinancialEvent ev;
                ev.userId = userId;
                ev.investmentId = inv.id;
                ev.eventType = "FD_MATURITY";
                ev.eventDate = fd.maturityDate;
                ev.amountCents = fd.estimatedMaturityAmountCents > 0 ? fd.estimatedMaturityAmountCents : fd.principalAmountCents;
                ev.status = (calculateDaysRemaining(fd.maturityDate, currentDate) < 0) ? "OVERDUE" : "PENDING";
                ev.description = fd.institutionName + " Term Deposit maturity proceeds (" + fd.accountNumber + ")";
                if (eventRepo.createEvent(ev)) {
                    ++eventsCreated;
                }
            }
        }

        // 2. Bond maturity event
        auto bondOpt = invRepo.getBondDetails(inv.id);
        if (bondOpt && bondOpt->status == "ACTIVE") {
            const auto &b = *bondOpt;
            double qty = inv.remainingQuantity > 0 ? inv.remainingQuantity : 1.0;
            int64_t totalPar = static_cast<int64_t>(b.faceValueCents * qty);
            if (!eventRepo.eventExists(userId, inv.id, "BOND_MATURITY", b.maturityDate)) {
                Models::FinancialEvent ev;
                ev.userId = userId;
                ev.investmentId = inv.id;
                ev.eventType = "BOND_MATURITY";
                ev.eventDate = b.maturityDate;
                ev.amountCents = totalPar;
                ev.status = (calculateDaysRemaining(b.maturityDate, currentDate) < 0) ? "OVERDUE" : "PENDING";
                ev.description = b.issuerName + " (" + b.bondType + ") redemption of face value";
                if (eventRepo.createEvent(ev)) {
                    ++eventsCreated;
                }
            }
        }
    }

    // 3. SIP scheduled installments
    Database::SipRepository sipRepo;
    auto sips = sipRepo.getPlansByUserId(userId);
    for (const auto &sip : sips) {
        if (sip.status == "ACTIVE") {
            std::string nextDate = sip.startDate; // Sample demonstration schedule date
            if (!eventRepo.eventExists(userId, InvestSphere::Optional<int>(), "SIP_INSTALLMENT", nextDate)) {
                Models::FinancialEvent ev;
                ev.userId = userId;
                ev.eventType = "SIP_INSTALLMENT";
                ev.eventDate = nextDate;
                ev.amountCents = sip.monthlyAmountCents;
                ev.status = "PENDING";
                ev.description = "Scheduled SIP installment for " + sip.planName;
                if (eventRepo.createEvent(ev)) {
                    ++eventsCreated;
                }
            }
        }
    }

    // Mark overdue events
    eventRepo.markOverdueEvents(userId, currentDate);

    return eventsCreated;
}

std::vector<Models::FinancialEvent> MaturityService::getEvents(int userId, const std::string &statusFilter)
{
    Database::FinancialEventRepository repo;
    return repo.getEventsForUser(userId, statusFilter);
}

bool MaturityService::updateEventStatus(int eventId, const std::string &newStatus)
{
    Database::FinancialEventRepository repo;
    return repo.updateEventStatus(eventId, newStatus);
}

bool MaturityService::markDepositAsMatured(int userId, int investmentId)
{
    Database::InvestmentRepository repo;
    auto inv = repo.getInvestmentByIdForUser(investmentId, userId);
    if (!inv) return false;

    // In Qt mode or foundation mode, update status in DB
#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = Database::DatabaseManager::instance().getQtDatabase();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE fixed_deposits SET status = 'MATURED' WHERE investment_id = :id;"));
    q.bindValue(QStringLiteral(":id"), investmentId);
    return q.exec();
#else
    return repo.updateStatusForUser(investmentId, userId, "MATURED");
#endif
}

bool MaturityService::markBondAsRedeemed(int userId, int investmentId)
{
    Database::InvestmentRepository repo;
    auto inv = repo.getInvestmentByIdForUser(investmentId, userId);
    if (!inv) return false;

#if defined(INVESTSPHERE_HAS_QT) && INVESTSPHERE_HAS_QT
    auto db = Database::DatabaseManager::instance().getQtDatabase();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE bonds SET status = 'REDEEMED' WHERE investment_id = :id;"));
    q.bindValue(QStringLiteral(":id"), investmentId);
    return q.exec();
#else
    return repo.updateStatusForUser(investmentId, userId, "REDEEMED");
#endif
}

} // namespace Services
} // namespace InvestSphere

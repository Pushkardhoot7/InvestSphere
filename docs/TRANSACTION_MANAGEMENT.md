# Transaction Management — Developer Reference

## Overview

Phase 7 introduces a complete, production-grade **Transaction Management** subsystem. Every financial event that changes a user's portfolio (purchases, sales, income, fees, and corporate actions) is expressed as a `Transaction` record. The subsystem enforces strict business rules, uses SQLite RAII transaction scopes for atomicity, and feeds the `PortfolioCalculationService` for accurate P&L reporting.

---

## Architecture

```
TransactionDialog / TransactionsPage (UI)
         │
         ▼
  TransactionService         ← single entry-point for all CRUD operations
    ├─ validateTransaction    ← rejects invalid input before touching DB
    ├─ recordTransaction      ← atomically inserts; auto-updates holding status
    ├─ updateTransaction      ← idempotent re-validation then atomic UPDATE
    └─ deleteTransaction      ← safety-checks before DELETE
         │
         ├── TransactionRepository   ← raw SQL persistence layer
         └── PortfolioCalculationService
                 └── CostBasisEngine  ← WAC algorithm + unrealized P&L math
```

---

## Supported Transaction Types

| Code          | Asset Classes Allowed          | Direction  |
|---------------|-------------------------------|------------|
| `BUY`         | STOCK, MUTUAL\_FUND, GOLD, BOND | Cash out   |
| `SELL`        | STOCK, GOLD                    | Cash in    |
| `REDEMPTION`  | MUTUAL\_FUND, BOND             | Cash in    |
| `DEPOSIT`     | FIXED\_DEPOSIT                 | Cash out   |
| `WITHDRAWAL`  | FIXED\_DEPOSIT                 | Cash in    |
| `DIVIDEND`    | STOCK, MUTUAL\_FUND            | Income     |
| `INTEREST`    | FIXED\_DEPOSIT, BOND           | Income     |
| `FEE`         | All                            | Cash out   |

---

## TransactionInputDTO Fields

| Field               | Type       | Description |
|---------------------|------------|-------------|
| `investmentId`      | `int`      | Foreign key to `investments.investment_id` |
| `transactionType`   | `string`   | One of the codes above |
| `transactionDate`   | `string`   | ISO-8601 `YYYY-MM-DD` or `YYYY-MM-DD HH:MM:SS` |
| `quantity`          | `double`   | Units bought/sold (0 for income/fee events) |
| `unitPriceCents`    | `int64_t`  | Price per unit in minor units (₹ paise) |
| `grossAmountCents`  | `int64_t`  | Pre-fee gross value in minor units |
| `feesCents`         | `int64_t`  | Brokerage / transaction fees (≥ 0) |
| `taxesCents`        | `int64_t`  | STT / TDS / withholding tax (≥ 0) |
| `netAmountCents`    | `int64_t`  | Derived net (see consistency rules below) |
| `notes`             | `string`   | Free-form memo (searchable) |

### Net Amount Consistency Rules

| Type             | Expected `netAmountCents`               |
|------------------|-----------------------------------------|
| BUY / DEPOSIT    | `grossAmount + fees + taxes`            |
| SELL / REDEMPTION| `grossAmount - fees - taxes`            |
| DIVIDEND / INTEREST | `grossAmount - fees - taxes`         |
| FEE              | Same as `grossAmount` (cost to user)    |

---

## Validation Rules (enforced in `TransactionService::validateTransaction`)

1. **User ownership** — investment must belong to the authenticated user.
2. **Asset class suitability** — only permitted types per category (see table above).
3. **Positive quantities** — required for BUY / SELL / REDEMPTION.
4. **Non-negative fees and taxes** — validated independently.
5. **Net amount consistency** — tolerance of ±5 minor units for rounding.
6. **Holding sufficiency** — SELL / REDEMPTION / WITHDRAWAL cannot create a negative running balance (checked chronologically via `PortfolioCalculationService::validateHoldingQuantityFeasibility`).
7. **Duplicate protection** — identical type + date + quantity + net amount within the same investment is rejected for new submissions.

---

## Atomicity

All mutations use `Database::TransactionScope` (RAII guard):

```cpp
Database::TransactionScope scope;
// ...perform DB writes...
if (error) { scope.rollback(); return {false, msg}; }
scope.commit();
```

`TransactionScope` automatically rolls back on destruction if `commit()` was never called — protecting against partial writes.

---

## Filtered Queries

`TransactionService::getTransactions` supports:

| Parameter     | Behaviour |
|---------------|-----------|
| `searchQuery` | Full-text match on `instrument_name`, `symbol`, and `notes` |
| `typeFilter`  | Exact match on `transaction_type` (`"ALL"` skips filter) |
| `startDate`   | Inclusive lower bound on `transaction_date` |
| `endDate`     | Inclusive upper bound on `transaction_date` |
| `sortBy`      | `"DATE"` or `"AMOUNT"` |
| `sortAscending` | `true` = ASC, `false` = DESC (default) |

---

## Key Files

| File | Purpose |
|------|---------|
| [`src/services/TransactionService.h`](file:///c:/InvestSphere/src/services/TransactionService.h) | Public API + DTOs |
| [`src/services/TransactionService.cpp`](file:///c:/InvestSphere/src/services/TransactionService.cpp) | Validation, CRUD, atomic execution |
| [`src/database/TransactionRepository.h`](file:///c:/InvestSphere/src/database/TransactionRepository.h) | Data-access interface |
| [`src/database/TransactionScope.h`](file:///c:/InvestSphere/src/database/TransactionScope.h) | RAII SQLite transaction guard |
| [`src/ui/transactions/TransactionDialog.h`](file:///c:/InvestSphere/src/ui/transactions/TransactionDialog.h) | New/Edit transaction modal |
| [`src/ui/transactions/TransactionsPage.h`](file:///c:/InvestSphere/src/ui/transactions/TransactionsPage.h) | Master ledger with filters |
| [`tests/test_phase7_transactions.cpp`](file:///c:/InvestSphere/tests/test_phase7_transactions.cpp) | 20-scenario automated test suite |

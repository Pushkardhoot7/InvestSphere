# InvestSphere — Relational Database Design Specification

## 1. Executive Database Overview

InvestSphere utilizes **SQLite 3** as its permanent embedded relational database engine. Designed specifically for an academic Database Management System (DBMS) course evaluation, the schema guarantees strict data integrity, adherence to Third Normal Form (3NF), ACID compliance, and complete multi-user isolation.

### Key Database Metrics
* **Engine:** SQLite 3 (WAL Mode, Foreign Keys Enforced)
* **Tables:** 18 Relational Tables (including lookup catalogs and schema migration tracking)
* **Views:** 6 Analytical and Reporting Views
* **Triggers:** 5 Integrity and Audit Automation Triggers
* **Indexes:** 16 B-Tree Indexes on Foreign Keys, Dates, and Lookups
* **Monetary Representation:** Integer Minor Units (Cents/Paise) to eliminate floating-point arithmetic errors.

---

## 2. Financial Precision & Money Representation Standard

### The Floating-Point Problem in Financial DBMS
Standard IEEE 754 floating-point types (`FLOAT`, `DOUBLE`, `REAL`) are binary approximations. They cannot represent decimal fractions precisely (e.g., $0.10 + $0.20 = 0.30000000000000004$). In portfolio accounting, rounding drifts accumulate across hundreds of transactions, causing balance sheets to fail audit reconciliations.

### The InvestSphere Standard
1. **Monetary Quantities (`INTEGER` Minor Units):**
   - All cash amounts, transaction values, gross/net trade totals, principal deposits, and fees are stored as standard 64-bit integers representing currency minor units (e.g., cents for USD/EUR, paise for INR).
   - Example: $\$1,450.50$ is stored as `145050`.
   - Conversions for presentation occur exclusively at the UI boundary: $\text{Dollars} = \frac{\text{Cents}}{100.0}$.
2. **Asset Quantities (`REAL` with 4-Decimal Precision):**
   - Stock shares, mutual fund units, and gold weights in grams require fractional amounts (e.g., $15.4520$ shares or $25.5000$ grams).
   - Quantities are stored as `REAL` and rounded to 4 decimal places ($0.0001$ fractional precision) using deterministic `ROUND(quantity, 4)` functions.
3. **Interest and Percentage Rates (`REAL`):**
   - Coupon rates, FD annual yields, and expense ratios are stored as true percentages ($7.10$ representing $7.10\%$, $0.04$ representing $0.04\%$) with strict `CHECK` constraints ($0.0 \le \text{rate} \le 100.0$).

---

## 3. Data Dictionary & Table Definitions

### 1. `users`
Stores user authentication credentials, profiles, and account statuses.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `user_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique surrogate key for user |
| `full_name` | `TEXT` | `NOT NULL` | User full display name |
| `email` | `TEXT` | `NOT NULL UNIQUE COLLATE NOCASE` | Unique login email address |
| `password_hash` | `TEXT` | `NOT NULL` | Cryptographic SHA-256 salt & hash |
| `account_status`| `TEXT` | `NOT NULL DEFAULT 'ACTIVE'` | Status: `ACTIVE`, `SUSPENDED`, `CLOSED` |
| `created_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Account registration timestamp |
| `updated_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Timestamp of last profile update |

### 2. `investment_types`
Lookup table defining approved asset categories.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `investment_type_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique category identifier |
| `type_code` | `TEXT` | `NOT NULL UNIQUE CHECK(...)` | Code: `STOCK`, `MUTUAL_FUND`, `GOLD`, `FIXED_DEPOSIT`, `BOND`, `CASH` |
| `type_name` | `TEXT` | `NOT NULL UNIQUE` | Human-readable name |
| `description` | `TEXT` | — | Asset class descriptive notes |
| `is_active` | `INTEGER` | `NOT NULL DEFAULT 1` | 1 if active, 0 if disabled |

### 3. `instruments`
Central market catalog of financial assets (3NF shared entity).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `instrument_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique instrument ID |
| `investment_type_id` | `INTEGER` | `NOT NULL REFERENCES investment_types(...)` | Asset category foreign key |
| `symbol` | `TEXT` | `NOT NULL UNIQUE COLLATE NOCASE` | Market ticker or scheme code |
| `instrument_name` | `TEXT` | `NOT NULL` | Company, scheme, or bond title |
| `currency` | `TEXT` | `NOT NULL DEFAULT 'USD'` | Denominated currency code |
| `exchange` | `TEXT` | — | Listing venue (NASDAQ, NYSE, etc.) |
| `description` | `TEXT` | — | Asset overview |
| `is_active` | `INTEGER` | `NOT NULL DEFAULT 1` | Active trading flag |
| `created_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Catalog entry timestamp |

### 4. `investments`
User-owned investment positions linking a user to an instrument.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `investment_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique holding ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | Holding owner |
| `instrument_id` | `INTEGER` | `NOT NULL REFERENCES instruments(...) ON DELETE RESTRICT`| Underlying instrument |
| `portfolio_tag` | `TEXT` | `NOT NULL DEFAULT 'DEFAULT'` | Custom tag (e.g. RETIREMENT, GROWTH) |
| `notes` | `TEXT` | — | Personal investment thesis |
| `status` | `TEXT` | `NOT NULL DEFAULT 'ACTIVE' CHECK(...)`| `ACTIVE`, `CLOSED`, `WATCHLIST` |
| `created_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Position inception timestamp |
| `updated_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Last modification timestamp |

### 5. `stocks`
Specialization table extending `instruments` (1-to-1).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `instrument_id` | `INTEGER` | `PRIMARY KEY REFERENCES instruments(...) ON DELETE CASCADE`| Shared PK/FK |
| `isin` | `TEXT` | `UNIQUE` | International Securities ID Number |
| `sector` | `TEXT` | — | Industry sector (e.g. Technology) |
| `industry` | `TEXT` | — | Sub-industry classification |
| `market_cap_category` | `TEXT` | `CHECK(...)` | `LARGE_CAP`, `MID_CAP`, `SMALL_CAP` |

### 6. `mutual_funds`
Specialization table extending `instruments` (1-to-1).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `instrument_id` | `INTEGER` | `PRIMARY KEY REFERENCES instruments(...) ON DELETE CASCADE`| Shared PK/FK |
| `amc_name` | `TEXT` | `NOT NULL` | Asset Management Company |
| `fund_category` | `TEXT` | `NOT NULL CHECK(...)` | `EQUITY`, `DEBT`, `HYBRID`, `INDEX` |
| `plan_type` | `TEXT` | `NOT NULL DEFAULT 'GROWTH'` | `GROWTH`, `DIVIDEND_PAYOUT` |
| `benchmark_index` | `TEXT` | — | Benchmark (e.g. S&P 500) |
| `expense_ratio` | `REAL` | `CHECK(0.0 <= ratio <= 10.0)` | Annual management fee percentage |

### 7. `gold_holdings`
Specialization table extending user `investments` (1-to-1).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `investment_id` | `INTEGER` | `PRIMARY KEY REFERENCES investments(...) ON DELETE CASCADE` | Shared PK/FK |
| `gold_form` | `TEXT` | `NOT NULL CHECK(...)` | `PHYSICAL_COIN`, `BAR`, `DIGITAL_GOLD` |
| `purity_karat` | `REAL` | `NOT NULL CHECK(0.0 < purity <= 24.0)` | Gold purity (e.g. 24.0, 22.0) |
| `storage_location`| `TEXT` | — | Vault or depository identifier |

### 8. `fixed_deposits`
Specialization table extending user `investments` (1-to-1).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `investment_id` | `INTEGER` | `PRIMARY KEY REFERENCES investments(...) ON DELETE CASCADE` | Shared PK/FK |
| `institution_name` | `TEXT` | `NOT NULL` | Issuing bank or financial institution |
| `account_number` | `TEXT` | — | Bank FD certificate/account number |
| `principal_amount`| `INTEGER` | `NOT NULL CHECK(principal_amount > 0)` | Deposit principal in cents |
| `annual_interest_rate`| `REAL` | `NOT NULL CHECK(...)` | Stated annual interest rate % |
| `compounding_frequency`| `TEXT` | `NOT NULL CHECK(...)` | `MONTHLY`, `QUARTERLY`, `ANNUALLY` |
| `payout_type` | `TEXT` | `NOT NULL CHECK(...)` | `CUMULATIVE`, `PERIODIC_MONTHLY` |
| `start_date` | `TEXT` | `NOT NULL` | Deposit inception date (YYYY-MM-DD) |
| `maturity_date` | `TEXT` | `NOT NULL CHECK(maturity_date > start_date)`| Payout date |
| `estimated_maturity_amount`| `INTEGER`| `NOT NULL CHECK(...)` | Estimated maturity value in cents |
| `status` | `TEXT` | `NOT NULL DEFAULT 'ACTIVE' CHECK(...)` | `ACTIVE`, `MATURED`, `PREMATURE_WITHDRAWN` |

### 9. `bonds`
Specialization table extending user `investments` (1-to-1).
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `investment_id` | `INTEGER` | `PRIMARY KEY REFERENCES investments(...) ON DELETE CASCADE` | Shared PK/FK |
| `issuer_name` | `TEXT` | `NOT NULL` | Sovereign or corporate issuer |
| `bond_type` | `TEXT` | `NOT NULL CHECK(...)` | `GOVERNMENT`, `CORPORATE`, `MUNICIPAL` |
| `face_value` | `INTEGER` | `NOT NULL CHECK(face_value > 0)` | Par value in cents |
| `coupon_rate` | `REAL` | `NOT NULL CHECK(...)` | Annual coupon yield percentage |
| `coupon_frequency`| `TEXT` | `NOT NULL CHECK(...)` | `ANNUALLY`, `SEMI_ANNUALLY`, `QUARTERLY` |
| `issue_date` | `TEXT` | `NOT NULL` | Original debt issuance date |
| `maturity_date` | `TEXT` | `NOT NULL CHECK(maturity_date > issue_date)` | Principal repayment date |
| `credit_rating` | `TEXT` | — | Credit grade (AAA, AA+, etc.) |
| `status` | `TEXT` | `NOT NULL DEFAULT 'ACTIVE' CHECK(...)` | `ACTIVE`, `REDEEMED`, `DEFAULTED` |

### 10. `transactions`
Unified transactional ledger recording all capital movements.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `transaction_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique transaction ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | User executing trade |
| `investment_id` | `INTEGER` | `REFERENCES investments(...) ON DELETE RESTRICT` | Associated holding |
| `transaction_type`| `TEXT` | `NOT NULL CHECK(...)` | `BUY`, `SELL`, `DIVIDEND`, `INTEREST` |
| `transaction_date`| `TEXT` | `NOT NULL` | Execution timestamp |
| `quantity` | `REAL` | `CHECK(quantity > 0.0 OR NULL)` | Shares, units, or grams transacted |
| `unit_price` | `INTEGER` | `CHECK(unit_price >= 0 OR NULL)` | Unit execution price in cents |
| `gross_amount` | `INTEGER` | `NOT NULL CHECK(gross_amount >= 0)` | Quantity × Unit Price (in cents) |
| `fees` | `INTEGER` | `NOT NULL DEFAULT 0 CHECK(fees >= 0)` | Brokerage & exchange commissions |
| `taxes` | `INTEGER` | `NOT NULL DEFAULT 0 CHECK(taxes >= 0)` | Regulatory transaction taxes |
| `net_amount` | `INTEGER` | `NOT NULL CHECK(net_amount >= 0)` | Total capital debited or credited |
| `notes` | `TEXT` | — | Trade memos and trade confirmations |
| `created_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Ledger recording timestamp |

### 11. `market_prices`
Historical spot and closing price observations.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `price_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique price observation ID |
| `instrument_id` | `INTEGER` | `NOT NULL REFERENCES instruments(...) ON DELETE CASCADE` | Target instrument |
| `price` | `INTEGER` | `NOT NULL CHECK(price >= 0)` | Price per unit in cents |
| `observation_timestamp`| `TEXT` | `NOT NULL` | Quote capture timestamp |
| `source` | `TEXT` | `NOT NULL DEFAULT 'MANUAL'` | Provider tag (`MANUAL`, `API`, `SAMPLE`) |
| `currency` | `TEXT` | `NOT NULL DEFAULT 'USD'` | Pricing currency |
* **Unique Composite Key:** `UNIQUE (instrument_id, observation_timestamp)` prevents duplicate timestamped observations.

### 12. `mutual_fund_nav`
Historical daily Net Asset Value recordings for mutual funds.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `nav_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique NAV record ID |
| `instrument_id` | `INTEGER` | `NOT NULL REFERENCES instruments(...) ON DELETE CASCADE` | Target mutual fund scheme |
| `nav_value` | `REAL` | `NOT NULL CHECK(nav_value > 0.0)` | Price per unit (e.g. 515.80) |
| `observation_date`| `TEXT` | `NOT NULL` | Business date of NAV |
| `source` | `TEXT` | `NOT NULL DEFAULT 'AMFI'` | Data authority tag |
* **Unique Composite Key:** `UNIQUE (instrument_id, observation_date)` guarantees only one official NAV per scheme per date.

### 13. `sip_plans`
Systematic Investment Plan mathematical modeling and savings goals.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `sip_plan_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique SIP plan ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | Plan owner |
| `instrument_id` | `INTEGER` | `REFERENCES instruments(...) ON DELETE SET NULL` | Target scheme |
| `plan_name` | `TEXT` | `NOT NULL` | Goal title (e.g. "House Down Payment") |
| `monthly_amount`| `INTEGER` | `NOT NULL CHECK(monthly_amount > 0)` | Monthly installment in cents |
| `expected_annual_return`| `REAL` | `NOT NULL CHECK(...)` | Assumed compounding return % |
| `duration_months`| `INTEGER` | `NOT NULL CHECK(duration_months > 0)` | Investment horizon in months |
| `start_date` | `TEXT` | `NOT NULL` | Inception date |
| `end_date` | `TEXT` | `NOT NULL CHECK(end_date >= start_date)` | Maturity horizon |
| `contribution_day_of_month`| `INTEGER`| `NOT NULL DEFAULT 1 CHECK(1 <= day <= 31)` | Day of debit |
| `projected_invested_capital`| `INTEGER`| `NOT NULL CHECK(...)` | Total principal invested (cents) |
| `projected_future_value`| `INTEGER` | `NOT NULL CHECK(...)` | Compounded future wealth (cents) |
| `status` | `TEXT` | `NOT NULL DEFAULT 'ACTIVE' CHECK(...)` | `ACTIVE`, `PAUSED`, `COMPLETED` |

### 14. `swp_plans`
Systematic Withdrawal Plan longevity and cash flow modeling.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `swp_plan_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique SWP plan ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | Plan owner |
| `instrument_id` | `INTEGER` | `REFERENCES instruments(...) ON DELETE SET NULL` | Source fund corpus |
| `plan_name` | `TEXT` | `NOT NULL` | Plan title (e.g. "Early Retirement") |
| `initial_corpus` | `INTEGER` | `NOT NULL CHECK(initial_corpus > 0)` | Starting principal in cents |
| `monthly_withdrawal`| `INTEGER` | `NOT NULL CHECK(monthly_withdrawal > 0)`| Monthly cash flow requirement (cents) |
| `expected_annual_return`| `REAL` | `NOT NULL CHECK(...)` | Assumed corpus growth rate % |
| `duration_months`| `INTEGER` | `NOT NULL CHECK(duration_months > 0)` | Withdrawal duration in months |
| `start_date` | `TEXT` | `NOT NULL` | First withdrawal date |
| `withdrawal_day_of_month`| `INTEGER`| `NOT NULL DEFAULT 1 CHECK(1 <= day <= 31)` | Day of withdrawal |
| `projected_final_balance`| `INTEGER`| `NOT NULL CHECK(...)` | Estimated residual principal (cents) |
| `status` | `TEXT` | `NOT NULL DEFAULT 'ACTIVE' CHECK(...)` | `ACTIVE`, `PAUSED`, `COMPLETED` |

### 15. `portfolio_snapshots`
Periodic portfolio net worth records for trend charting.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `snapshot_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique snapshot ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | User ID |
| `snapshot_timestamp`| `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Valuation snapshot timestamp |
| `total_invested` | `INTEGER` | `NOT NULL CHECK(total_invested >= 0)` | Total capital deployed (cents) |
| `estimated_portfolio_value`| `INTEGER`| `NOT NULL CHECK(...)` | Current market value (cents) |
| `realized_profit_loss`| `INTEGER` | `NOT NULL` | Cumulative closed gains (cents) |
| `unrealized_profit_loss`| `INTEGER`| `NOT NULL` | Cumulative paper gains (cents) |
| `notes` | `TEXT` | — | Milestone description |

### 16. `financial_events`
Chronological event calendar for maturities, coupons, and debits.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `event_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique event ID |
| `user_id` | `INTEGER` | `NOT NULL REFERENCES users(...) ON DELETE CASCADE` | Event subscriber |
| `investment_id` | `INTEGER` | `REFERENCES investments(...) ON DELETE CASCADE` | Linked asset holding |
| `event_type` | `TEXT` | `NOT NULL CHECK(...)` | `FD_MATURITY`, `BOND_COUPON`, `SIP_INSTALLMENT` |
| `event_date` | `TEXT` | `NOT NULL` | Scheduled date (YYYY-MM-DD) |
| `amount` | `INTEGER` | `CHECK(amount >= 0 OR NULL)` | Anticipated cash amount in cents |
| `status` | `TEXT` | `NOT NULL DEFAULT 'PENDING' CHECK(...)` | `PENDING`, `PROCESSED`, `CANCELLED` |
| `description` | `TEXT` | `NOT NULL` | Calendar reminder summary |
* **Unique Composite Key:** `UNIQUE (user_id, investment_id, event_type, event_date)` guarantees events are not duplicated.

### 17. `audit_logs`
Immutable compliance and security audit log.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `audit_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Unique audit record ID |
| `user_id` | `INTEGER` | `REFERENCES users(...) ON DELETE SET NULL` | Performing user (nullable for system) |
| `action_type` | `TEXT` | `NOT NULL CHECK(...)` | `INSERT`, `UPDATE`, `DELETE`, `LOGIN` |
| `entity_type` | `TEXT` | `NOT NULL` | Affected domain entity |
| `entity_id` | `INTEGER` | — | Record primary key |
| `action_timestamp`| `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Execution timestamp |
| `description` | `TEXT` | `NOT NULL` | Descriptive log message |

### 18. `schema_migrations`
Tracks database schema versions and upgrade history.
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `migration_id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Auto-incrementing migration step ID |
| `version` | `INTEGER` | `NOT NULL UNIQUE` | Sequential schema version number |
| `description` | `TEXT` | `NOT NULL` | Migration summary note |
| `applied_at` | `TEXT` | `NOT NULL DEFAULT (datetime('now'))` | Timestamp of migration execution |

---

## 4. Referential Integrity & Deletion Policies

InvestSphere applies deliberate, domain-specific referential actions:
1. **`ON DELETE CASCADE` for Owned Subtypes and User Private Data:**
   - Deleting a `user` cascades and cleans up that user's private `investments`, `transactions`, `sip_plans`, `swp_plans`, and `financial_events`.
   - Deleting an `instrument` cascades to its subtype record (`stocks`, `mutual_funds`) and price history (`market_prices`, `mutual_fund_nav`).
2. **`ON DELETE RESTRICT` for Shared Market Data & Active Holdings:**
   - An `investment_type` cannot be deleted if instruments are currently assigned to it (`ON DELETE RESTRICT`).
   - An `instrument` cannot be dropped if any user holds an active investment position in it (`ON DELETE RESTRICT`).
   - An `investment` holding cannot be dropped if historical `transactions` exist referencing it (`ON DELETE RESTRICT`), ensuring financial ledger integrity.
3. **`ON DELETE SET NULL` for Audit and Planning References:**
   - Deleting a user does not delete historical compliance records in `audit_logs`; instead, `user_id` is set to `NULL` to preserve legal audit continuity.
   - Deleting an instrument linked to a generic `sip_plan` sets `instrument_id` to `NULL` while retaining the user's mathematical calculation goals.

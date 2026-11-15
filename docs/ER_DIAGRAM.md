# InvestSphere — Entity-Relationship (ER) Documentation

## 1. Overview & Architectural Blueprint

The **InvestSphere** relational database is designed according to **Third Normal Form (3NF)** principles to ensure data integrity, eliminate update anomalies, and cleanly isolate shared market data from user-specific holdings.

A core architectural distinction in this schema is the separation between:
1. **Financial Instruments (`instruments`):** Market-level entities (e.g. AAPL, MSFT, Vanguard 500) containing symbol, company description, exchange, and market classification. Multiple users can invest in the same instrument without duplicating ticker metadata.
2. **User Investments (`investments`):** User-level holdings linking a specific `user_id` to an `instrument_id`, tracking portfolio tags, user notes, and holding lifecycle states.

---

## 2. Core ER Diagram (Simplified Model)

The following diagram highlights the central transactional and asset relationship spine:

```mermaid
erDiagram
    USERS ||--o{ INVESTMENTS : "owns"
    USERS ||--o{ TRANSACTIONS : "executes"
    INVESTMENT_TYPES ||--o{ INSTRUMENTS : "categorizes"
    INSTRUMENTS ||--o{ INVESTMENTS : "referenced_by"
    INVESTMENTS ||--o{ TRANSACTIONS : "accumulates"
    INSTRUMENTS ||--o{ MARKET_PRICES : "tracks"
    INSTRUMENTS ||--o{ MUTUAL_FUND_NAV : "tracks"

    USERS {
        int user_id PK
        string full_name
        string email UK
        string account_status
    }

    INVESTMENT_TYPES {
        int investment_type_id PK
        string type_code UK
        string type_name
    }

    INSTRUMENTS {
        int instrument_id PK
        int investment_type_id FK
        string symbol UK
        string instrument_name
        string currency
    }

    INVESTMENTS {
        int investment_id PK
        int user_id FK
        int instrument_id FK
        string portfolio_tag
        string status
    }

    TRANSACTIONS {
        int transaction_id PK
        int user_id FK
        int investment_id FK
        string transaction_type
        string transaction_date
        float quantity
        int net_amount
    }

    MARKET_PRICES {
        int price_id PK
        int instrument_id FK
        int price
        string observation_timestamp
    }

    MUTUAL_FUND_NAV {
        int nav_id PK
        int instrument_id FK
        float nav_value
        string observation_date
    }
```

---

## 3. Complete Relational ER Diagram

```mermaid
erDiagram
    USERS ||--o{ INVESTMENTS : "owns"
    USERS ||--o{ TRANSACTIONS : "executes"
    USERS ||--o{ SIP_PLANS : "plans"
    USERS ||--o{ SWP_PLANS : "plans"
    USERS ||--o{ PORTFOLIO_SNAPSHOTS : "records"
    USERS ||--o{ FINANCIAL_EVENTS : "tracks"
    USERS ||--o{ AUDIT_LOGS : "triggers"

    INVESTMENT_TYPES ||--o{ INSTRUMENTS : "categorizes"

    INSTRUMENTS ||--o{ INVESTMENTS : "referenced_by"
    INSTRUMENTS ||--|| STOCKS : "specializes"
    INSTRUMENTS ||--|| MUTUAL_FUNDS : "specializes"
    INSTRUMENTS ||--o{ MARKET_PRICES : "has_history"
    INSTRUMENTS ||--o{ MUTUAL_FUND_NAV : "has_nav"
    INSTRUMENTS ||--o{ SIP_PLANS : "targeted_by"
    INSTRUMENTS ||--o{ SWP_PLANS : "targeted_by"

    INVESTMENTS ||--|| GOLD_HOLDINGS : "specializes"
    INVESTMENTS ||--|| FIXED_DEPOSITS : "specializes"
    INVESTMENTS ||--|| BONDS : "specializes"
    INVESTMENTS ||--o{ TRANSACTIONS : "accumulates"
    INVESTMENTS ||--o{ FINANCIAL_EVENTS : "schedules"

    USERS {
        int user_id PK
        string full_name
        string email UK
        string password_hash
        string account_status
        string created_at
        string updated_at
    }

    INVESTMENT_TYPES {
        int investment_type_id PK
        string type_code UK
        string type_name UK
        string description
        int is_active
    }

    INSTRUMENTS {
        int instrument_id PK
        int investment_type_id FK
        string symbol UK
        string instrument_name
        string currency
        string exchange
        string description
        int is_active
        string created_at
    }

    INVESTMENTS {
        int investment_id PK
        int user_id FK
        int instrument_id FK
        string portfolio_tag
        string notes
        string status
        string created_at
        string updated_at
    }

    STOCKS {
        int instrument_id PK,FK
        string isin UK
        string sector
        string industry
        string market_cap_category
    }

    MUTUAL_FUNDS {
        int instrument_id PK,FK
        string amc_name
        string fund_category
        string plan_type
        string benchmark_index
        float expense_ratio
    }

    GOLD_HOLDINGS {
        int investment_id PK,FK
        string gold_form
        float purity_karat
        string storage_location
    }

    FIXED_DEPOSITS {
        int investment_id PK,FK
        string institution_name
        string account_number
        int principal_amount
        float annual_interest_rate
        string compounding_frequency
        string payout_type
        string start_date
        string maturity_date
        int estimated_maturity_amount
        string status
    }

    BONDS {
        int investment_id PK,FK
        string issuer_name
        string bond_type
        int face_value
        float coupon_rate
        string coupon_frequency
        string issue_date
        string maturity_date
        string credit_rating
        string status
    }

    TRANSACTIONS {
        int transaction_id PK
        int user_id FK
        int investment_id FK
        string transaction_type
        string transaction_date
        float quantity
        int unit_price
        int gross_amount
        int fees
        int taxes
        int net_amount
        string notes
        string created_at
    }

    MARKET_PRICES {
        int price_id PK
        int instrument_id FK
        int price
        string observation_timestamp
        string source
        string currency
    }

    MUTUAL_FUND_NAV {
        int nav_id PK
        int instrument_id FK
        float nav_value
        string observation_date
        string source
    }

    SIP_PLANS {
        int sip_plan_id PK
        int user_id FK
        int instrument_id FK
        string plan_name
        int monthly_amount
        float expected_annual_return
        int duration_months
        string start_date
        string end_date
        int contribution_day_of_month
        int projected_invested_capital
        int projected_future_value
        string status
        string created_at
    }

    SWP_PLANS {
        int swp_plan_id PK
        int user_id FK
        int instrument_id FK
        string plan_name
        int initial_corpus
        int monthly_withdrawal
        float expected_annual_return
        int duration_months
        string start_date
        int withdrawal_day_of_month
        int projected_final_balance
        string status
        string created_at
    }

    PORTFOLIO_SNAPSHOTS {
        int snapshot_id PK
        int user_id FK
        string snapshot_timestamp
        int total_invested
        int estimated_portfolio_value
        int realized_profit_loss
        int unrealized_profit_loss
        string notes
    }

    FINANCIAL_EVENTS {
        int event_id PK
        int user_id FK
        int investment_id FK
        string event_type
        string event_date
        int amount
        string status
        string description
        string created_at
    }

    AUDIT_LOGS {
        int audit_id PK
        int user_id FK
        string action_type
        string entity_type
        int entity_id
        string action_timestamp
        string description
    }

    SCHEMA_MIGRATIONS {
        int migration_id PK
        int version UK
        string description
        string applied_at
    }
```

---

## 4. Entity Cardinality & Relationship Catalog

| Parent Entity | Child Entity | Cardinality | Relationship Description | Foreign Key Constraint |
| :--- | :--- | :---: | :--- | :--- |
| `users` | `investments` | `1 : N` | One user owns zero or more investment holdings | `ON DELETE CASCADE` |
| `users` | `transactions` | `1 : N` | One user records zero or more financial transactions | `ON DELETE CASCADE` |
| `users` | `sip_plans` | `1 : N` | One user saves multiple goal-oriented SIP calculations | `ON DELETE CASCADE` |
| `users` | `swp_plans` | `1 : N` | One user saves multiple retirement SWP calculations | `ON DELETE CASCADE` |
| `users` | `portfolio_snapshots` | `1 : N` | One user records periodic portfolio value snapshots | `ON DELETE CASCADE` |
| `users` | `financial_events` | `1 : N` | One user tracks scheduled financial calendar events | `ON DELETE CASCADE` |
| `users` | `audit_logs` | `1 : N` | One user triggers system and financial audit entries | `ON DELETE SET NULL` |
| `investment_types` | `instruments` | `1 : N` | One category (e.g. Stock) groups many market instruments | `ON DELETE RESTRICT` |
| `instruments` | `investments` | `1 : N` | One instrument can be held across multiple user portfolios | `ON DELETE RESTRICT` |
| `instruments` | `stocks` | `1 : 1` | Stock-specific metadata extends instrument (ISIN, sector) | `ON DELETE CASCADE` |
| `instruments` | `mutual_funds` | `1 : 1` | Fund-specific metadata extends instrument (AMC, category) | `ON DELETE CASCADE` |
| `instruments` | `market_prices` | `1 : N` | One instrument has historical timestamped spot quotes | `ON DELETE CASCADE` |
| `instruments` | `mutual_fund_nav`| `1 : N` | One mutual fund has historical daily NAV observations | `ON DELETE CASCADE` |
| `investments` | `gold_holdings` | `1 : 1` | User gold holding specifies purity and storage location | `ON DELETE CASCADE` |
| `investments` | `fixed_deposits` | `1 : 1` | User FD specifies bank, rate, tenure, and payout rules | `ON DELETE CASCADE` |
| `investments` | `bonds` | `1 : 1` | User bond specifies issuer, coupon rate, and maturity | `ON DELETE CASCADE` |
| `investments` | `transactions` | `1 : N` | One user holding accumulates buy, sell, and income trades | `ON DELETE RESTRICT` |
| `investments` | `financial_events`| `1 : N` | One investment holding generates maturity & coupon events | `ON DELETE CASCADE` |

---

## 5. Subtype Specialization Pattern (Table Inheritance)

To model diverse investment instruments without creating an un-normalized "god table" with dozens of NULL columns, InvestSphere implements the **Class Table Inheritance (Subtype Specialization)** pattern:

1. **Market Instruments (`instruments`):**
   - Base attributes common to all traded instruments: `symbol`, `instrument_name`, `currency`, `exchange`.
   - Specialization table `stocks` shares `instrument_id` as both Primary Key and Foreign Key.
   - Specialization table `mutual_funds` shares `instrument_id` as both Primary Key and Foreign Key.
2. **User Contracts (`investments`):**
   - User holdings common to all portfolio entries: `user_id`, `instrument_id`, `portfolio_tag`, `status`.
   - Specialization table `fixed_deposits` stores bank deposit terms (`principal_amount`, `compounding_frequency`).
   - Specialization table `bonds` stores debt parameters (`face_value`, `coupon_rate`, `credit_rating`).
   - Specialization table `gold_holdings` stores physical asset attributes (`purity_karat`, `gold_form`).

This design maintains strict data integrity, guarantees Third Normal Form (3NF), and enables polymorphic reporting.

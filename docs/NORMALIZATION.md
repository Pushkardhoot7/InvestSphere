# InvestSphere — Database Normalization & Dependency Analysis

## 1. Academic Normalization Overview

In relational database theory, normalization is the systematic technique of organizing schemas to minimize data redundancy and eliminate anomalies during data modification (Insert, Update, and Delete anomalies).

For the **InvestSphere** project, the schema was engineered to strictly satisfy **Third Normal Form (3NF)** while balancing query performance for analytical reporting.

---

## 2. Normal Forms Explained with InvestSphere Examples

### First Normal Form (1NF)
**Definition:** A relation is in 1NF if and only if all domain values are atomic (indivisible), every row has a unique identifier, and there are no repeating groups or multi-valued arrays.

#### Violation Scenario in Portfolio Systems:
Consider a naive investment spreadsheet:
| User | Stocks Held | Purchase Dates | Quantities |
| :--- | :--- | :--- | :--- |
| Aditya Sharma | AAPL, MSFT | 2026-02-10, 2026-03-01 | 50, 30 |

* **Why it violates 1NF:** The attributes `Stocks Held`, `Purchase Dates`, and `Quantities` contain non-atomic, comma-delimited lists. It is impossible to index individual trades, aggregate partial sales, or enforce referential foreign keys.

#### 1NF Solution in InvestSphere:
* Each individual trade is broken down into an atomic row in the `transactions` table.
* Attributes like `quantity`, `unit_price`, and `transaction_date` contain single, indivisible values.
* Surrogate primary key `transaction_id` uniquely identifies each record.

---

### Second Normal Form (2NF)
**Definition:** A relation is in 2NF if and only if it is in 1NF and **every non-prime attribute is fully functionally dependent on the entire primary key** (i.e. no partial functional dependencies on composite candidate keys).

#### Violation Scenario in Multi-Asset Portfolios:
Suppose we attempted to store holdings using a composite primary key `(user_id, symbol)`:
$$\text{UserHoldings}(\underline{\text{user\_id}}, \underline{\text{symbol}}, \text{company\_name}, \text{exchange}, \text{quantity}, \text{portfolio\_tag})$$

Functional Dependencies:
1. $\{\text{user\_id}, \text{symbol}\} \rightarrow \text{quantity}, \text{portfolio\_tag}$ (Full dependency)
2. $\text{symbol} \rightarrow \text{company\_name}, \text{exchange}$ (**Partial dependency!**)

* **Why it violates 2NF:** `company_name` and `exchange` depend *only* on `symbol`, not on the full composite key $\{\text{user\_id}, \text{symbol}\}$.
* **Resulting Anomalies:**
  - **Insertion Anomaly:** We cannot catalog a new stock (e.g., NVIDIA) until at least one user buys shares in it.
  - **Deletion Anomaly:** If the only user holding Apple sells all shares and deletes their row, the application loses the knowledge that Apple is listed on NASDAQ.
  - **Update Anomaly:** If Microsoft changes its corporate entity name, every single user row holding MSFT must be updated; failing to update all rows introduces inconsistent data.

#### 2NF Solution in InvestSphere:
* Separate the relation into two distinct tables:
  1. `instruments`: Primary key `instrument_id`. Holds `symbol`, `instrument_name`, `exchange`.
  2. `investments`: Primary key `investment_id`. Foreign keys `user_id` and `instrument_id`.
* Every attribute in each table now depends on the complete primary key of that table.

---

### Third Normal Form (3NF)
**Definition:** A relation is in 3NF if and only if it is in 2NF and **no non-prime attribute is transitively dependent on the primary key** (i.e., $X \rightarrow Y$ and $Y \rightarrow Z$, meaning non-key attributes must depend *only* on the primary key, directly).

#### Violation Scenario in Mutual Fund Schemas:
Suppose we stored fund investments in a single table:
$$\text{FundHoldings}(\underline{\text{investment\_id}}, \text{user\_id}, \text{scheme\_name}, \text{category\_code}, \text{category\_description}, \text{units})$$

Functional Dependencies:
1. $\text{investment\_id} \rightarrow \text{category\_code}$
2. $\text{category\_code} \rightarrow \text{category\_description}$
3. Therefore: $\text{investment\_id} \rightarrow \text{category\_description}$ (**Transitive dependency!**)

* **Why it violates 3NF:** `category_description` depends on `category_code`, which in turn depends on `investment_id`.
* **Resulting Anomalies:**
  - **Redundancy:** If 500 users hold different equity mutual funds, the string description of "Equity Schemes" is duplicated 500 times.
  - **Update Anomaly:** Modifying the regulatory definition of an ELSS fund requires scanning and mutating every single fund holding in the system.

#### 3NF Solution in InvestSphere:
* Decompose into lookup entity `investment_types`:
  $$\text{investment\_types}(\underline{\text{investment\_type\_id}}, \text{type\_code}, \text{type\_name}, \text{description})$$
  $$\text{instruments}(\underline{\text{instrument\_id}}, \text{investment\_type\_id}, \text{symbol}, \text{instrument\_name})$$
  $$\text{investments}(\underline{\text{investment\_id}}, \text{user\_id}, \text{instrument\_id}, \text{portfolio\_tag})$$
* Zero transitive dependencies remain: non-key attributes depend strictly and directly on the primary key.

---

## 3. Formal Functional Dependencies (FD) Catalog

Below is the formal functional dependency mapping across all primary tables in the InvestSphere schema:

| Table | Candidate Key(s) | Functional Dependencies (Determinants $\rightarrow$ Dependents) | Normal Form |
| :--- | :--- | :--- | :---: |
| `users` | `user_id`, `email` | `user_id` $\rightarrow$ `full_name`, `email`, `password_hash`, `account_status`, `created_at`<br>`email` $\rightarrow$ `user_id`, `full_name`, `password_hash` | **3NF** |
| `investment_types` | `investment_type_id`, `type_code`, `type_name` | `investment_type_id` $\rightarrow$ `type_code`, `type_name`, `description`, `is_active`<br>`type_code` $\rightarrow$ `investment_type_id`, `type_name` | **3NF** |
| `instruments` | `instrument_id`, `symbol` | `instrument_id` $\rightarrow$ `investment_type_id`, `symbol`, `instrument_name`, `currency`, `exchange`<br>`symbol` $\rightarrow$ `instrument_id`, `instrument_name` | **3NF** |
| `investments` | `investment_id`, `(user_id, instrument_id, portfolio_tag)` | `investment_id` $\rightarrow$ `user_id`, `instrument_id`, `portfolio_tag`, `notes`, `status`<br>`(user_id, instrument_id, portfolio_tag)` $\rightarrow$ `investment_id` | **3NF** |
| `stocks` | `instrument_id`, `isin` | `instrument_id` $\rightarrow$ `isin`, `sector`, `industry`, `market_cap_category`<br>`isin` $\rightarrow$ `instrument_id`, `sector` | **3NF** |
| `mutual_funds` | `instrument_id` | `instrument_id` $\rightarrow$ `amc_name`, `fund_category`, `plan_type`, `benchmark_index`, `expense_ratio` | **3NF** |
| `fixed_deposits` | `investment_id` | `investment_id` $\rightarrow$ `institution_name`, `principal_amount`, `annual_interest_rate`, `compounding_frequency`, `payout_type`, `start_date`, `maturity_date` | **3NF** |
| `bonds` | `investment_id` | `investment_id` $\rightarrow$ `issuer_name`, `bond_type`, `face_value`, `coupon_rate`, `coupon_frequency`, `issue_date`, `maturity_date` | **3NF** |
| `transactions` | `transaction_id` | `transaction_id` $\rightarrow$ `user_id`, `investment_id`, `transaction_type`, `transaction_date`, `quantity`, `unit_price`, `gross_amount`, `fees`, `taxes`, `net_amount` | **3NF** |
| `market_prices`| `price_id`, `(instrument_id, observation_timestamp)` | `price_id` $\rightarrow$ `instrument_id`, `price`, `observation_timestamp`, `source`, `currency`<br>`(instrument_id, observation_timestamp)` $\rightarrow$ `price_id`, `price` | **3NF** |
| `mutual_fund_nav`| `nav_id`, `(instrument_id, observation_date)` | `nav_id` $\rightarrow$ `instrument_id`, `nav_value`, `observation_date`, `source`<br>`(instrument_id, observation_date)` $\rightarrow$ `nav_id`, `nav_value` | **3NF** |

---

## 4. Calculated & Derived Values Policy

A frequent dilemma in database normalization is whether to store derived attributes:
* **Total Portfolio Value**
* **Net Remaining Quantity Held**
* **Realized Profit / Loss**

### The InvestSphere Architectural Decision
1. **No Stored Derived Quantities:**
   - Remaining share balance is **NOT** stored as a mutable integer column in `investments`.
   - Instead, remaining quantity is dynamically calculated from the immutable transaction ledger:
     $$\text{Remaining Quantity} = \sum \text{BUY} - \sum \text{SELL}$$
   - This eliminates the classic concurrency hazard where an interrupted transaction updates a balance column without recording a ledger row.
2. **Dynamic Views for Fast Retrieval:**
   - The view `vw_active_holdings` performs this aggregation via SQL `SUM()` and `GROUP BY`, leveraging indexes on `(investment_id, transaction_type)` for sub-millisecond execution.
3. **Periodic Snapshots (`portfolio_snapshots`):**
   - For historical performance charting (e.g. "What was my portfolio worth on July 1st?"), historical valuation cannot be recomputed if market price history is incomplete.
   - Hence, `portfolio_snapshots` stores timestamped point-in-time valuation aggregates, explicitly documented as cached historical observations rather than operational transactional state.

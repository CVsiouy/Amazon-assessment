# C++: Return Routing Engine

A C++ return routing engine for an E-commerce platform that evaluates return requests by checking category eligibility, computing risk scores from linked-account history, and mapping scores to routing decision.

Your task is to fix the broken routing engine so it correctly validates return policies, aggregates risk signals, and assigns the right decision and reason for each request.

# Input Format

Return requests file: `data/returns.jsonl` One JSON object per line. Each return request includes:

Each return request includes:

- `request_id` - unique identifier
- `account_id` - customer account ID
- `category` - product category
- `days_since_purchase` - days elapsed since purchase
- `order_value_usd` - order value in USD (optional)
- `account_age_days` - account age in days (optional)

Example:

```
{"request_id":"r101","account_id":"u101","category":"ELECTRONICS","days_since_purchase":12,"order_value_usd":250,"}
{"request_id":"r102","account_id":"u220","category":"GROCERY","days_since_purchase":5,"order_value_usd":30,"accoun}
```

# Output Format

Results file: `data/results.jsonl`

One JSON object per request, in the same order as the input. Each result includes:

- `request_id` - copied from input
- `risk_score` - integer 0-100 if passed policy checks, `null` otherwise
- `decision` - one of `AUTO_APPROVE`, `MANUAL_REVIEW`, `REJECT`
- `reason` - required for `MANUAL_REVIEW` and `REJECT`; omitted for `AUTO_APPROVE`

Example:

```
{"request_id":"r101","risk_score":35,"decision":"AUTO_APPROVE"}
{"request_id":"r102","risk_score":null,"decision":"REJECT", "reason":"CATEGORY_NON_RETURNABLE"}
{"request_id":"r103","risk_score":80,"decision":"REJECT":"HIGH_RISK_SCORE"}
```

# Decision Rules

## Step 1: Category Checks

**Category eligibility:** A product is returnable only if its category exists in `data/category_rules.json` and is marked `returnable=true`. If the category is missing or marked non-returnable, reject immediately:

- `decision` : `REJECT`
- `reason` : `CATEGORY_NON_RETURNABLE`
- `risk_score` : `null`

**Return window:** Compare `days_since_purchase` against the category's `allowed_return_window_days`. If `days_since_purchase` is strictly greater than the allowed window, reject immediately:

- `decision` : `REJECT`
- `reason` : `RETURN_WINDOW_EXPIRED`
- `risk_score` : `null`

## Step 2: Risk Score

1. Aggregate return history across linked accounts

For the request's `account_id`, compute total return history by summing:

- The account's own `return_history_count` from `data/account_profiles.jsonl`
- All linked accounts' `return_history_count` from `data/account_links.jsonl`

Rules:

- Missing profile → treat `return_history_count` as O
- No linked accounts → use only the customer's own profile

2. Calculate risk score

Using rules from `data/scoring_rules.json` , for each rule:

    1. Read the attribute specified in `"rule"` (e.g., `total_return_history`, `account_age_days`, `order_valye_usd`)
    2. Find the matching value range
    3. Add the corresponding `points` to the total

Sum all rule points. Clamp the total to 0-100. The result is the final `risk_score`

## Step 3: Decision + Reason

Using bands from `data/decision_bands.json`, map the risk score to a decision:

- 0-49 → `AUTO_APPROVE`
- 50-74 → `MANUAL_REVIEW` with reason `MEDIUN_RISK_SCORE`
- 75-100 → `REJECT` with reason `HIGH_RISK_SCORE`

Omit the `reason` field when the decision is `AUTO_APPROVE`.

# Project Structure

```
|—— include/
|      |— returns/
|            |— models.hpp      # Data models (CategoryRule, ScoringRule, ReturnRequest, etc.)
|            |— parser.hpp      # JSON parsing utilities
|            |— engine.hpp      # Core return routing engine
|            |— file_io.hpp     # File I/0 utilities
|
|—— src/
|     |— returns/
|            |— parser.cpp
|            |— engine.cpp      # Engine implementation
|            |— file_io.cpp
|            |— main.cpp        # CLI entry point
|
|—— tests/
|     |— engine_test.cpp
|
|—— data/
|     |— category_rules.json    # Category return policies
|     |— scoring_rules.json     # Risk scoring rules
|     |— decision_bands.json    # Score-to-decision mapping
|     |— account_profiles.jsonl # Account return histories
|     |— account_links.jsonl    # Account Linkage records
|     |— returns.jsonl          # Return requests
|
|—— run.sh                      # Setup and run script
|—— tests.sh                    # Test script (writes report.xml)
|—— CMakeLists.txt              # Project configuration
|—— README.md

```

# Examples

## **Example 1: Auto-approved low-risk return**

input:

```
{"request_id":"r101","account_id":"u184","category":"ELECTRONICS","days_since_purchase":10,"order_value_usd":150,"}
```

**Output:**

```
{
    "request_id": "r101",
    "risk_score": 3,
    "decision": "AUTO_APPROVE"
}
```

**Explanation:**

- u104 has 0 returns and no linked accounts, so `total_return_history` = 0 → 0 pts
- Account age 365 days → 0 pts (180+ range)
- Order valye $150 → 3 pts (50-199 range)
- Total risk score = 3, which falls in AUTO_APPROVE band (0-49)


## **Example 2: Manual review with linked accounts**

input:

```
{"request_id":"r103","account_id":"u101","category":"ELECTRONICS","days_since_purchase":12,"order_value_usd":250,"}
```

**Output:**

```
{
    "request_id": "r103",
    "risk_score": 60,
    "decision": "MANUAL_REVIEW",
    "reason": "MEDIUM_RISK_SCORE"
}
```

**Explanation:**

- u101 is linked to u102 and u103
- Total return history: u101(2) + u102(8) + u103(4) = 14 → 50 pts (11+ range)
- Account age 400 days → 0 pts (180+ range)
- Order value $250 → 10 pts (200-499 range)
- Total risk score = 60, which falls in MANUAL_REVIEW band (50-74)


## **Example 3: High-risk rejection with linked group**

input:

```
{"request_id":"r104","account_id":"u107","category":"FURNITURE","days_since_purchase":20,"order_value_usd":800,"}
```

**Output:**

```
{
    "request_id": "r104",
    "risk_score": 95,
    "decision": "REJECT",
    "reason": "HIGH_RISK_SCORE"
}
```

**Explanation:**

- u107 is linked to u108 and u109
- Total return history: u107(6) + u108(3) + u109(15) = 24 → 50 pts (11+ range)
- Account age 60 days → 20 pts (30-179+ range)
- Order value $800 → 25 pts (500+ range)
- Total risk score = 95, which falls in REJECT band (75-100)


## **Example 4: Return Window Expired**

input:

```
{"request_id":"r105","account_id":"u104","category":"ELECTRONICS","days_since_purchase":35,"order_value_usd":150,"}
```

**Output:**

```
{
    "request_id": "r105",
    "risk_score": null,
    "decision": "REJECT",
    "reason": "RETURN_WINDOW_EXPIRED"
}
```

**Explanation:**

- ELECTRONICS allows a 30-day return window
- The request at 35 days exceeds this limit
- Rejected immediately without computing a risk score




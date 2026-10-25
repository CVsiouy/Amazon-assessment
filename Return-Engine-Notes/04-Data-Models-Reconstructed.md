---
tags: [amazon-assessment, cpp, headers, reconstruction]
---

# The Missing Pieces: Data Models, Reconstructed

`include/returns/models.hpp`, `engine.hpp`, `parser.hpp`, and `file_io.hpp` are all currently empty (0 bytes) in your project. That's not one of "the 8 bugs" — it's transcription that never happened, most likely photos that either don't exist yet or haven't been typed up. Nothing below is copied from your photos; it's rebuilt by carefully reading how every field and function is *used* across `engine.cpp`, `file_io.cpp`, `main.cpp`, and `parser.cpp`, then working backward to the simplest struct/declaration that would make every one of those usages valid.

> [!warning] Verify this against your photos
> This reconstruction is high-confidence but not gospel — I don't have ground truth for these four files. Before you trust anything here for a real fix, go back to your photos (or re-derive it yourself using the method below) and confirm field names, types, and whether fields like `order_value_usd` are `int` or `std::optional<int>`. Treat this chapter as a very strong first draft.

## The technique: reconstructing a type from its usage

This is a genuinely useful, transferable skill, so here's how it's done, not just the result.

For any struct, ask: "every place in the `.cpp` files that writes to or reads a field of this struct — what name and type must that field have for the surrounding code to make sense?"

Take `ReturnRequest`. Searching `parser.cpp`'s `parse_return_request_object()`, I find:
```cpp
r.request_id = parse_string();   // parse_string() returns std::string → request_id is std::string
r.account_id = parse_string();   // same → std::string
r.category = parse_string();     // same → std::string
r.days_since_purchase = parse_int();   // parse_int() returns int → int
r.order_value_usd = parse_int();       // int
// account_age_days is read from JSON but (due to a bug) discarded — still, the key existing
// in the input and being read as a number tells us the field is meant to be an int
```
And in `engine.cpp`, `req.account_age_days` is used directly as an `int` in arithmetic-style comparisons (`attribute_value = req.account_age_days;` where `attribute_value` is declared `int`) — confirming `int`, not `std::optional<int>`, is the simplest type consistent with everything I can see (the README calling these fields "optional" most likely means "the key may be absent from the input JSON," which this parser handles by just leaving the field at its default value rather than by using `std::optional`).

Do this for every struct, and you get the full header. That's the entire method — no guessing, just "what does the surrounding code require to be true."

## `include/returns/models.hpp` — reconstructed

```cpp
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace returns {

// One entry from data/category_rules.json
struct CategoryRule {
    std::string category;
    bool returnable = false;
    int allowed_return_window_days = 0;
};

// One [min, max] -> points bracket, nested inside a ScoringRule
struct ScoringRange {
    int min = 0;
    int max = 0;
    int points = 0;
};

// One entry from data/scoring_rules.json
struct ScoringRule {
    std::string rule;                    // e.g. "total_return_history"
    std::vector<ScoringRange> ranges;
};

// One entry from data/decision_bands.json
struct DecisionBand {
    int min_score = 0;
    int max_score = 0;
    std::string decision;                // e.g. "AUTO_APPROVE"
};

// One entry from data/account_profiles.jsonl
struct AccountProfile {
    std::string account_id;
    int return_history_count = 0;
};

// One entry from data/account_links.jsonl
struct AccountLink {
    std::string primary_account_id;
    std::string related_account_id;
    std::string link_type;
};

// One entry from data/returns.jsonl — a single return request to evaluate
struct ReturnRequest {
    std::string request_id;
    std::string account_id;
    std::string category;
    int days_since_purchase = 0;
    int order_value_usd = 0;             // README calls this "optional"; absent -> stays 0
    int account_age_days = 0;            // README calls this "optional"; absent -> stays 0
};

// One line of output written to data/results.jsonl
struct RoutingResult {
    std::string request_id;
    std::optional<int> risk_score;       // nullopt when rejected at the category-check stage
    std::string decision;
    std::string reason;                  // empty string means "omit this field on output"
};

// String constants for the "decision" field, spelled once so nothing typos it
namespace decision {
    inline constexpr const char* AUTO_APPROVE  = "AUTO_APPROVE";
    inline constexpr const char* MANUAL_REVIEW = "MANUAL_REVIEW";
    inline constexpr const char* REJECT        = "REJECT";
}

// String constants for the "reason" field
namespace reason {
    inline constexpr const char* CATEGORY_NON_RETURNABLE = "CATEGORY_NON_RETURNABLE";
    inline constexpr const char* RETURN_WINDOW_EXPIRED   = "RETURN_WINDOW_EXPIRED";
    inline constexpr const char* MEDIUM_RISK_SCORE       = "MEDIUM_RISK_SCORE";
    inline constexpr const char* HIGH_RISK_SCORE         = "HIGH_RISK_SCORE";
}

// String constants matching the "rule" field inside scoring_rules.json
namespace rule_attr {
    inline constexpr const char* TOTAL_RETURN_HISTORY = "total_return_history";
    inline constexpr const char* ACCOUNT_AGE_DAYS      = "account_age_days";
    inline constexpr const char* ORDER_VALUE_USD       = "order_value_usd";
}

} // namespace returns
```

Why these specific defaults and types:
- `RoutingResult::risk_score` must be able to hold "nothing at all" (`std::nullopt`) because `engine.cpp` explicitly does `out.risk_score = std::nullopt;` for rejected requests, and `file_io.cpp`'s `write_results` calls `.has_value()` on it before deciding whether to print a number or `null`. Only `std::optional<int>` fits both facts at once.
- `RoutingResult::reason` is a plain `std::string`, not optional, because `write_results` checks `if (!result.reason.empty())` — that's how you test "is this string empty," not how you'd test an `std::optional`.
- `bool CategoryRule::returnable` — used directly in `if (!it->second.returnable)`, so it must be a `bool`.

## `include/returns/engine.hpp` — reconstructed

```cpp
#pragma once

#include "returns/models.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace returns {

class Engine {
public:
    static Engine build(
        const std::vector<CategoryRule> &category_rules,
        const std::vector<ScoringRule> &scoring_rules,
        const std::vector<DecisionBand> &decision_bands,
        const std::vector<AccountProfile> &profiles,
        const std::vector<AccountLink> &links);

    std::vector<RoutingResult> evaluate(const std::vector<ReturnRequest> &requests) const;

private:
    Engine(
        std::unordered_map<std::string, CategoryRule> category_map,
        std::vector<ScoringRule> scoring_rules,
        std::vector<DecisionBand> decision_bands,
        std::unordered_map<std::string, int> profile_map,
        std::unordered_map<std::string, std::unordered_set<std::string>> account_groups);

    std::pair<std::string, std::string> check_category(const ReturnRequest &req) const;
    int aggregate_return_history(const std::string &account_id) const;
    int calculate_risk_score(const ReturnRequest &req, int total_return_history) const;
    std::pair<std::string, std::string> map_decision(int risk_score) const;

    std::unordered_map<std::string, CategoryRule> category_map_;
    std::vector<ScoringRule> scoring_rules_;
    std::vector<DecisionBand> decision_bands_;
    std::unordered_map<std::string, int> profile_map_;
    std::unordered_map<std::string, std::unordered_set<std::string>> account_groups_;
};

} // namespace returns
```

Why `build` is `static` and the constructor is `private`: `main.cpp` calls `returns::Engine::build(...)` without ever having an `Engine` to call it on, which only works for a `static` method. Making the raw constructor `private` (with `build` as the only way in) is a common, deliberate pattern — it forces every `Engine` to go through the one function that does the transitive-closure setup work, so nobody can accidentally construct a "half-built" `Engine` by calling the constructor directly.

## `include/returns/parser.hpp` — reconstructed

```cpp
#pragma once

#include "returns/models.hpp"
#include <string>
#include <vector>

namespace returns {

std::vector<CategoryRule> parse_category_rules_json(const std::string &json_doc);
std::vector<ScoringRule> parse_scoring_rules_json(const std::string &json_doc);
std::vector<DecisionBand> parse_decision_bands_json(const std::string &json_doc);

AccountProfile parse_account_profile_line(const std::string &line);
AccountLink parse_account_link_line(const std::string &line);
ReturnRequest parse_return_request_line(const std::string &line);

} // namespace returns
```

Every one of these names is called, with this exact argument shape, from `file_io.cpp` — this header is nothing more than writing down promises that `parser.cpp` already keeps.

## `include/returns/file_io.hpp` — reconstructed

```cpp
#pragma once

#include "returns/models.hpp"
#include <string>
#include <vector>

namespace returns {

std::vector<CategoryRule> read_category_rules(const std::string &path);
std::vector<ScoringRule> read_scoring_rules(const std::string &path);
std::vector<DecisionBand> read_decision_bands(const std::string &path);
std::vector<AccountProfile> read_account_profiles(const std::string &path);
std::vector<AccountLink> read_account_links(const std::string &path);
std::vector<ReturnRequest> read_return_requests(const std::string &path);

void write_results(const std::string &path, const std::vector<RoutingResult> &results);

} // namespace returns
```

Same idea — every signature here is dictated exactly by how `main.cpp` calls these functions.

## What to actually do with this

1. Paste (or re-derive) these into the four empty header files.
2. Check them against your photos of the header files, if you have any, and correct anything that differs — especially exact field types for `order_value_usd`/`account_age_days` (plain `int` vs `std::optional<int>`), since that's the one place I had to make a judgment call rather than a pure deduction.
3. Once the headers exist and the ghost-text/typo cleanup from [[12-Pre-Compile-Cleanup-Checklist]] is done, the project should actually compile.

Next: [[05-Walkthrough-Parser]]

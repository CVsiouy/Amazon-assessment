---
tags: [amazon-assessment, cpp, walkthrough, engine]
---

# Walkthrough: `engine.cpp`

This is the file that actually implements the three-step pipeline from [[01-Big-Picture-The-Business-Problem]]. Everything in `parser.cpp` and `file_io.cpp` exists only to feed this file clean data and carry its answers back out. If you only have time to deeply understand one file in this whole project, make it this one.

## The `Engine` class, at a glance

Looking at [[04-Data-Models-Reconstructed]], `Engine` holds five pieces of pre-processed data as private member variables:

| Member | Type | What it's for |
|---|---|---|
| `category_map_` | `unordered_map<string, CategoryRule>` | category name → its rule, for O(1) lookup |
| `scoring_rules_` | `vector<ScoringRule>` | the three scoring rules, in the order read from JSON |
| `decision_bands_` | `vector<DecisionBand>` | the three score bands |
| `profile_map_` | `unordered_map<string, int>` | account id → its own return_history_count |
| `account_groups_` | `unordered_map<string, unordered_set<string>>` | account id → every *other* account id linked to it (after transitive closure) |

The whole point of `Engine::build(...)` is to turn the flat lists (`vector<CategoryRule>`, `vector<AccountProfile>`, `vector<AccountLink>`) that came straight out of the JSON files into these five lookup-friendly shapes, *once*, up front — so that `evaluate()` can answer questions like "is ELECTRONICS returnable?" or "who is u001 linked to?" instantly instead of re-scanning a list for every single request.

## `Engine::build(...)` — three straightforward conversions, then one hard part

The first three loops are simple "flat list → map" conversions:

```cpp
std::unordered_map<std::string, CategoryRule> category_map;
for (const auto &rule : category_rules) {
    category_map[rule.category] = rule;
}

std::unordered_map<std::string, int> profile_map;
for (const auto &profile : profiles) {
    profile_map[profile.account_id] = profile.return_history_count;
}

std::unordered_map<std::string, std::unordered_set<std::string>> account_groups;
for (const auto &link : links) {
    account_groups[link.primary_account_id].insert(link.related_account_id);
}
```

Each one reads as: *"for every item in the flat list, use one of its fields as the map key, and store the rest (or a derived value) as the map's value."* Note that `account_groups[link.primary_account_id]` uses `operator[]` (see [[02-Cpp-Concepts-You-Need]] section 5) — if `link.primary_account_id` hasn't been seen before, this *creates* an empty `unordered_set` for it on the spot, and then `.insert(...)` adds the related account into that fresh set. After this loop, `account_groups` only has **direct, one-hop** links: if the data says A→B and B→C, at this point `account_groups["A"] == {"B"}` and `account_groups["B"] == {"C"}` — `A` doesn't know about `C` yet.

> [!bug] A typo in the very first loop
> Your typed file has `category_map[rule, category] = rule;` — a comma where a dot belongs. `rule, category` is read by C++ as the comma operator (evaluate `rule`, throw it away, then try to use a variable named `category` — which doesn't exist here, so it wouldn't even compile). The real line is `category_map[rule.category] = rule;`. This is a plain transcription slip (a `.` misread as a `,`), not a logic bug — see [[12-Pre-Compile-Cleanup-Checklist]].

## The hard part: transitive closure

"A is linked to B" and "B is linked to C" should mean A, B, and C are all treated as **one connected cluster** — A's risk calculation should see C's return history too, even though nothing directly says "A is linked to C." Turning "who's directly linked to whom" into "who's in the same cluster as whom, no matter how many hops away" is called computing the **transitive closure** of the relationship.

The algorithm the code uses is the classic brute-force approach: *keep sweeping through every group, and for every account already in a group, pull in everything **that** account is linked to as well — repeat until an entire sweep makes zero changes, meaning nothing new to discover.*

Cleaned of ghost text, here's what it's supposed to do:

```cpp
bool changed = true;
while (changed) {
    changed = false;
    for (auto &[id, group] : account_groups) {
        std::unordered_set<std::string> expanded = group;
        for (const auto &member : group) {
            auto it = account_groups.find(member);
            if (it != account_groups.end()) {
                for (const auto &transitive : it->second) {
                    if (transitive != id && expanded.find(transitive) == expanded.end()) {
                        expanded.insert(transitive);
                        changed = true;   // <-- we grew a group; there might be more to discover, sweep again
                    }
                }
            }
        }
        group = expanded;
    }
}
```

Walk through the intent line by line: for every `(id, group)` pair already known, start `expanded` as a copy of the current group. For every `member` already in that group, check whether *that member* is itself a key in `account_groups` (i.e. does it have outgoing links of its own?) — if so, pull every one of *its* linked accounts (`transitive`) into `expanded` too, as long as it isn't `id` itself and isn't already present. After trying every member, replace the group with the expanded version. Crucially: **if any group grew during this sweep, `changed` must become `true`**, so the outer `while` loop runs another full sweep — because that newly-added account might unlock a further, third-hop account next time.

> [!bug] Your typed file sets `changed = false` at the exact spot it should set `changed = true`
> This is Bug 3 in [[10-The-8-Bugs]] — go there for the full worked-example proof. In short: because the flag that's supposed to say "we found something new, sweep again" is hard-coded to `false` at the one place it should be `true`, the `while (changed)` loop's body only ever runs once, no matter how long a chain of linked accounts exists. A 4-hop chain like `u001 → u002 → u003 → u004 → u005` in your own `data/account_links.jsonl` never fully connects — accounts several hops away silently never join the group.

## `check_category(req)` — Step 1 of the pipeline

```cpp
std::pair<std::string, std::string> Engine::check_category(const ReturnRequest &req) const {
    auto it = category_map_.find(req.category);
    if (it == category_map_.end()) {
        return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};
    }
    if (!it->second.returnable) {
        return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};
    }
    return {"", ""};
}
```

Read the return convention carefully, because `evaluate()` depends on it: an **empty pair `{"", ""}`** means "no objection, keep going" — a non-empty `decision` means "stop right here, this is the final answer." `req.category` not existing in the map, or existing but `returnable == false`, both correctly map to `REJECT` / `CATEGORY_NON_RETURNABLE`, matching the spec.

> [!bug] The one thing missing is right there in the function's own doc comment
> The comment above this function says `/// Check category eligibility: returnable flag and return window.` — but the body never once reads `req.days_since_purchase` or `it->second.allowed_return_window_days`. The entire second half of Step 1 (README: *"Compare `days_since_purchase` against the category's `allowed_return_window_days`... reject immediately"*) is simply absent. This is Bug 4 — see [[10-The-8-Bugs]] for a concrete request from your own data that proves it. Whenever a doc comment promises something the function body doesn't do, that gap is worth treating as a bug candidate on sight.

> [!tip] The name typo
> Your file defines this function as `heck_category` (missing the `c`) but `evaluate()` below calls it as `check_category`. Whichever one is "correct," they need to match — most likely the intended name is `check_category` everywhere (matching the doc comment and the header declaration in [[04-Data-Models-Reconstructed]]). This is a naming/transcription issue, not a logic bug.

## `aggregate_return_history(account_id)` — Step 2a of the pipeline

```cpp
int Engine::aggregate_return_history(const std::string &account_id) const {
    auto profile_it = profile_map_.find(account_id);
    int total = (profile_it != profile_map_.end()) ? profile_it->second : 0;

    auto group_it = account_groups_.find(account_id);
    if (group_it != account_groups_.end()) {
        for (const auto &linked_id : group_it->second) {
            auto linked_profile = profile_map_.find(linked_id);
            if (linked_profile != profile_map_.end()) {
                total += linked_profile->second;
            }
        }
    }

    return total;
}
```

This one is logically sound as written: start `total` from the account's own history (or `0` if it has no profile at all — matching the spec's "missing profile → treat as 0"), then, if this account has a linked group, add every linked account's own history too (again, `0` if a linked account has no profile). The `?:` ternary just means "if this condition is true, use the first value, otherwise the second" — a compact `if`/`else` for a single value. This function's correctness is entirely dependent on `account_groups_` already being fully, correctly closed by `build()` — which is exactly why Bug 3 upstream (the `changed` flag) can make an otherwise-correct function produce wrong totals: garbage in, garbage out.

## `calculate_risk_score(req, total_return_history)` — Step 2b/2c

```cpp
int Engine::calculate_risk_score(const ReturnRequest &req, int total_return_history) const {
    int score = 0;

    for (const auto &rule : scoring_rules_) {
        int attribute_value = 0;

        if (rule.rule == rule_attr::TOTAL_RETURN_HISTORY) {
            attribute_value = total_return_history;
        } else if (rule.rule == rule_attr::ACCOUNT_AGE_DAYS) {
            attribute_value = req.account_age_days;
        } else if (rule.rule == rule_attr::ORDER_VALUE_USD) {
            attribute_value = req.order_value_usd;
        }

        int points = 0;
        for (const auto &range : rule.ranges) {
            if (attribute_value >= range.min && attribute_value <= range.max) {
                points = range.points;
            }
        }
        score += points;
    }

    return score;
}
```

Outer loop: for each of the three scoring rules loaded from `scoring_rules.json`, figure out which attribute of *this request* it's asking about (`total_return_history`, `account_age_days`, or `order_value_usd`) by comparing the rule's `rule` string against the three known constants. Inner loop: scan that rule's list of `[min, max] → points` brackets and take the `points` of whichever bracket the value falls into (there's no `break` once found, but since the ranges shouldn't overlap in the data, the last/only match wins either way). Add each rule's points to the running `score`.

> [!bug] A typo, and a missing requirement
> Your typed file assigns to `attribute_valye` (note the swapped letters) instead of the `attribute_value` declared two lines above, for the `order_value_usd` branch specifically — a plain transcription typo, fix the spelling. Separately, and much more importantly: **the function returns `score` with no clamping at all.** The README explicitly says *"Clamp the total to 0-100."* Add up the maximum possible points from `data/scoring_rules.json` (`50 + 45 + 25 = 120`) and you'll see scores above 100 are absolutely reachable — this is Bug 5, detailed with a concrete example in [[10-The-8-Bugs]].

## `map_decision(risk_score)` — Step 3 of the pipeline

```cpp
std::pair<std::string, std::string> Engine::map_decision(int risk_score) const {
    for (const auto &band : decision_bands_) {
        if (risk_score >= band.min_score && risk_score <= band.max_score) {
            return {band.decision, ""};
        }
    }
    return {decision::REJECT, ""};
}
```

Scan the three bands loaded from `decision_bands.json`, and return whichever one's `[min_score, max_score]` contains this `risk_score`. If somehow none match (which, per Bug 5 above, becomes possible once scores aren't clamped — a raw score of `120` matches none of `0-49`/`50-74`/`75-100`), fall back to `REJECT` by default.

> [!bug] The reason is hard-coded empty, always
> Both `return` statements above hand back `""` as the reason, no matter which band matched. But the spec is explicit: `MANUAL_REVIEW` needs reason `MEDIUM_RISK_SCORE`, and a score-based `REJECT` needs reason `HIGH_RISK_SCORE`. Since `decision_bands.json` never stores a reason string (only `min_score`/`max_score`/`decision`), that mapping has to happen *here, in code* — and right now, it doesn't happen at all. This is Bug 6, and it's independently, numerically provable using your own `results.jsonl` — see [[10-The-8-Bugs]].

## `evaluate(requests)` — the driver that ties all four together

```cpp
std::vector<RoutingResult> Engine::evaluate(const std::vector<ReturnRequest> &requests) const {
    std::vector<RoutingResult> results;
    results.reserve(requests.size());

    for (const auto &req : requests) {
        RoutingResult out;
        out.request_id = req.request_id;

        auto [cat_decision, cat_reason] = check_category(req);
        if (!cat_decision.empty()) {
            out.risk_score = std::nullopt;
            out.decision = cat_decision;
            out.reason = cat_reason;
            results.push_back(std::move(out));
            continue;                      // skip straight to the next request
        }

        int total_history = aggregate_return_history(req.account_id);
        int risk_score = calculate_risk_score(req, total_history);

        auto [decision, reason] = map_decision(risk_score);
        out.risk_score = risk_score;
        out.decision = decision;
        out.reason = reason;
        results.push_back(std::move(out));
    }

    return results;
}
```

This is a direct translation of the pipeline diagram from [[01-Big-Picture-The-Business-Problem]] into code: for every request, first ask `check_category` for a verdict. If it returned a non-empty decision, that's a hard stop — record `risk_score` as absent (`std::nullopt`), record the decision/reason, and `continue` (skip the rest of the loop body, move to the next request). Otherwise, fall through to compute the aggregated history, the risk score, and the final decision/reason, and record those instead.

`std::move(out)` (see [[02-Cpp-Concepts-You-Need]]) transfers `out`'s internals into the vector instead of copying them — a minor efficiency detail, not something that changes behavior here.

> [!tip] Naming shadow, not a bug
> The line `auto [decision, reason] = map_decision(risk_score);` creates local variables named `decision` and `reason`, which are also the names of the two namespaces you saw earlier (`decision::REJECT`, `reason::CATEGORY_NON_RETURNABLE`). This is legal — the local variable simply "shadows" the namespace name within this scope — but it's worth noticing so it doesn't confuse you into thinking something strange is happening.

## What to take away from this file

- `build()` does setup work once; `evaluate()` does per-request work many times. Keep that separation in mind — a bug in `build()` (like Bug 3) silently corrupts *every* request that touches a multi-hop linked group, not just one.
- Every one of `check_category`, `calculate_risk_score`, and `map_decision` has exactly one thing quietly missing from an otherwise-correct function: a check, a clamp, and an assignment, respectively. None of them are "wrong algorithms" — they're each one small, specific omission.
- `evaluate()` itself is clean and correctly wired — it's a good example of "the orchestration logic is fine, the bugs live in the pieces it calls," which is a common shape for planted bugs in an assessment like this.

Next: [[08-Walkthrough-Main]]

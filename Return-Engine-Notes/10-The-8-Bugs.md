---
tags: [amazon-assessment, cpp, bugs, answer-key]
---

# The 8 Bugs

Your research after the assessment turned up that there are exactly 8 bugs in this codebase. Below are 8 genuine logic/spec bugs, found by comparing every function against the README's requirements and, wherever possible, proven numerically against your own `results.jsonl` and `data/` files — not guessed. They're ordered by where they sit in the pipeline (parsing → category check → scoring → decision → output), which is also the order you should fix them in (see the "bugs can hide other bugs" section of [[09-End-to-End-Trace]]).

> [!warning] One honest caveat
> Four header files (`models.hpp`, `engine.hpp`, `parser.hpp`, `file_io.hpp`) are currently empty in your project, so this analysis is built on the four `.cpp` files plus a high-confidence reconstruction of the headers (see [[04-Data-Models-Reconstructed]]). Everything below is independently corroborated by either the README's exact wording or your own `results.jsonl` data (usually both), which is why confidence is high despite the missing headers — but if your photos of the real headers surface something structurally different, re-check bugs 1, 2, and 8 first, since those are the ones most sensitive to exact field types.

This list deliberately excludes the ghost-text and hand-copy typos (`heck_category`, `attribute_valye`, `!-`, `lkey`, `sscoring_rules_path`, `getLine`, `json_espace`, `stds::vector`, `parse_direction_band_object`, the `category_map[rule, category]` comma, the missing `=` in `main.cpp`, etc.) — those are transcription artifacts, not the assessment's planted bugs. The full mechanical fix-list for those is in [[12-Pre-Compile-Cleanup-Checklist]].

---

## Bug 1 — Return history is silently discarded while parsing

**Where:** `src/returns/parser.cpp`, inside `parse_account_profile_object()` (see [[05-Walkthrough-Parser]])

**Current code:**
```cpp
else if (key == "return_history_count") {
    skip_value();
}
```

**What it should be:**
```cpp
else if (key == "return_history_count") {
    p.return_history_count = parse_int();
}
```

**Why this is a bug:** `skip_value()` correctly advances the parser's cursor past the value, but never captures it into anything — every `AccountProfile`'s `return_history_count` ends up at its default value (`0`), regardless of what `data/account_profiles.jsonl` actually says.

**Proof from real data:** Account `u053` has `return_history_count: 6` in `data/account_profiles.jsonl`. Request `r115` uses account `u053` and has no linked accounts complicating the picture beyond its own profile. If `6` were read correctly, it alone lands in the "6-10" bracket → 30 points toward the risk score. `results.jsonl` shows `r115` scoring `70` = `45 (age, see Bug 2) + 25 (order_value_usd=800 → 500+ bracket)` — with **zero** contribution from history. The only way `u053`'s own `6` return history contributes `0` points is if it was never read in the first place. Same proof pattern holds for `u049` (own count `5`, request `r113`) and `u012` (own count `2`, request `r107`). Full derivation in [[09-End-to-End-Trace]].

**Impact:** every request's risk score is missing its entire `total_return_history` contribution — potentially 0, 15, 30, or 50 points, depending on the account — which is often the single largest scoring factor in the whole system.

---

## Bug 2 — Account age is silently discarded while parsing

**Where:** `src/returns/parser.cpp`, inside `parse_return_request_object()` (see [[05-Walkthrough-Parser]])

**Current code:**
```cpp
else if (key == "account_age_days") {
    skip_value();
}
```

**What it should be:**
```cpp
else if (key == "account_age_days") {
    r.account_age_days = parse_int();
}
```

**Why this is a bug:** identical mistake to Bug 1, different struct. `req.account_age_days` always reads as `0`, no matter what the input JSON says.

**Proof from real data:** Request `r101` states `"account_age_days": 400`, which should fall in the "180+" bracket → **0** points. `results.jsonl` shows `r101` scoring `55`. Working backward (full derivation in [[09-End-to-End-Trace]], Trace 1): `55 = 0 (history, Bug 1) + 45 (age) + 10 (order_value_usd=250 → 200-499 bracket)`. The only bracket worth 45 points is "0-29" — meaning the code is behaving as though `account_age_days` is `0`, not `400`. This holds across every single non-rejected row in `results.jsonl` (36 rows checked) — every one of them is consistent with age contributing exactly 45 points, i.e. being read as `0`, with only two exceptions (`r120`, `r139`) where the account's *real* age genuinely is under 30 days, so 45 points is actually correct there too, coincidentally.

**Impact:** every request is scored as though it came from a brand-new, maximally-suspicious account — the single worst account-age bracket — regardless of its true age. Combined with Bug 1, this is why nearly every row in your `results.jsonl` clusters suspiciously close to `45 + (0, 3, 10, or 25)`.

---

## Bug 3 — The linked-account grouping loop never signals that it changed

**Where:** `src/returns/engine.cpp`, inside `Engine::build(...)` (see [[07-Walkthrough-Engine]])

**Current code:**
```cpp
if (transitive != id && expanded.find(transitive) == expanded.end()) {
    expanded.insert(transitive);
    changed = false;   // <-- wrong
}
```

**What it should be:**
```cpp
if (transitive != id && expanded.find(transitive) == expanded.end()) {
    expanded.insert(transitive);
    changed = true;    // <-- we grew a group, so sweep again — there might be more
}
```

**Why this is a bug:** the surrounding `while (changed) { changed = false; ... }` loop is a standard "keep sweeping until nothing changes" fixed-point algorithm (see [[07-Walkthrough-Engine]] for the full walkthrough of *why* this pattern is needed at all). For it to work, discovering something new during a sweep must set `changed` back to `true` so another sweep happens. Because it's hard-coded to `false` at the exact moment something new is found, the loop body runs its single sweep and then immediately exits — multi-hop chains never fully connect.

**Proof from real data:** `data/account_links.jsonl` contains the 4-hop chain `u001 → u002 → u003 → u004 → u005`. A single sweep can only ever pull in accounts that are one additional hop away from what's already known — accounts 3+ hops out are structurally unreachable in one pass, no matter which account is processed first.

**Why you can't see this one directly in `results.jsonl` right now:** Bug 1 already zeroes every account's `return_history_count` to `0`, so whether the *grouping* is right or wrong is currently invisible in the output — `0 + 0 + 0 + ...` is `0` either way. This bug only becomes observable once Bug 1 is fixed. See "bugs can hide other bugs" in [[09-End-to-End-Trace]].

**Impact:** once Bug 1 is fixed, any account more than 2 hops deep in a linked cluster will still under-report its true combined return history, potentially pushing a request into a lower-risk band than it should be in.

---

## Bug 4 — The return-window check doesn't exist

**Where:** `src/returns/engine.cpp`, inside `check_category(...)` (defined as `heck_category` in your file — see [[07-Walkthrough-Engine]])

**Current code:**
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

**What it should be:**
```cpp
std::pair<std::string, std::string> Engine::check_category(const ReturnRequest &req) const {
    auto it = category_map_.find(req.category);
    if (it == category_map_.end()) {
        return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};
    }
    if (!it->second.returnable) {
        return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};
    }
    if (req.days_since_purchase > it->second.allowed_return_window_days) {
        return {decision::REJECT, reason::RETURN_WINDOW_EXPIRED};
    }
    return {"", ""};
}
```

**Why this is a bug:** the README is explicit: *"Compare `days_since_purchase` against the category's `allowed_return_window_days`. If `days_since_purchase` is strictly greater than the allowed window, reject immediately."* The current function's own doc comment even says `/// Check category eligibility: returnable flag and return window.` — promising a check that the body never performs.

**Proof from real data:** Your `data/returns.jsonl` contains exactly one window-violating request for every one of the six returnable categories (`r105` ELECTRONICS, `r114` CLOTHING, `r121` BOOKS, `r125` JEWELRY, `r135` TOYS, `r137` FURNITURE). All six show a computed score and a non-`REJECT` decision in `results.jsonl` instead of `null`/`REJECT`/`RETURN_WINDOW_EXPIRED`. Full derivation in [[09-End-to-End-Trace]], Trace 2.

**Impact:** requests that should be rejected outright — no risk scoring needed — instead flow all the way through the scoring pipeline and can be auto-approved or sent for manual review, which is a correctness failure at the very first gate the system is supposed to enforce.

---

## Bug 5 — The risk score is never clamped to 0-100

**Where:** `src/returns/engine.cpp`, inside `calculate_risk_score(...)` (see [[07-Walkthrough-Engine]])

**Current code:**
```cpp
int Engine::calculate_risk_score(const ReturnRequest &req, int total_return_history) const {
    int score = 0;
    for (const auto &rule : scoring_rules_) {
        // ... adds up points ...
        score += points;
    }
    return score;   // <-- no clamp
}
```

**What it should be:**
```cpp
int Engine::calculate_risk_score(const ReturnRequest &req, int total_return_history) const {
    int score = 0;
    for (const auto &rule : scoring_rules_) {
        // ... adds up points ...
        score += points;
    }
    if (score < 0) score = 0;
    if (score > 100) score = 100;
    return score;
    // equivalently: return std::clamp(score, 0, 100);
}
```

**Why this is a bug:** the README states plainly, *"Sum all rule points. Clamp the total to 0-100."* The maximum possible points from `data/scoring_rules.json` is `50 (total_return_history) + 45 (account_age_days) + 25 (order_value_usd) = 120`, which is mathematically greater than 100 — so this isn't a theoretical edge case, it's guaranteed to be reachable.

**Proof from real data:** Account `u100` (request `r139`) has its own `return_history_count = 19` and no linked accounts. Correctly computed: `19 → 50 points` (history) `+ 25 (age=25, genuinely under 30) + 25 (order_value_usd=5000, 500+ bracket) = 120`. That must be reported as `100`, not `120`. Full derivation in [[09-End-to-End-Trace]], Trace 3.

**Why you can't see the exact number 120 in your current `results.jsonl`:** Bugs 1 and 2 currently cap the *maximum possible* score at `45 + 25 = 70` (since history always contributes 0, and age always contributes the flat 45), so scores over 100 can't appear until those two are fixed. This is another instance of "bugs can hide other bugs" — see [[09-End-to-End-Trace]].

**Impact:** once Bugs 1 and 2 are fixed, high-history + new-account + high-value requests will report a `risk_score` outside the documented `0-100` range, which breaks the output contract even in cases where the final decision (REJECT, via `map_decision`'s fallback) happens to still come out right.

---

## Bug 6 — The decision reason is never set for score-based bands

**Where:** `src/returns/engine.cpp`, inside `map_decision(...)` (see [[07-Walkthrough-Engine]])

**Current code:**
```cpp
std::pair<std::string, std::string> Engine::map_decision(int risk_score) const {
    for (const auto &band : decision_bands_) {
        if (risk_score >= band.min_score && risk_score <= band.max_score) {
            return {band.decision, ""};      // <-- always empty
        }
    }
    return {decision::REJECT, ""};           // <-- always empty here too
}
```

**What it should be:**
```cpp
std::pair<std::string, std::string> Engine::map_decision(int risk_score) const {
    for (const auto &band : decision_bands_) {
        if (risk_score >= band.min_score && risk_score <= band.max_score) {
            std::string reason = "";
            if (band.decision == decision::MANUAL_REVIEW) reason = reason::MEDIUM_RISK_SCORE;
            else if (band.decision == decision::REJECT) reason = reason::HIGH_RISK_SCORE;
            return {band.decision, reason};
        }
    }
    return {decision::REJECT, reason::HIGH_RISK_SCORE};
}
```

**Why this is a bug:** `data/decision_bands.json` only stores `min_score`, `max_score`, and `decision` — there is no `reason` field in the data at all, so the mapping from decision → reason text has to happen in code. The README requires it: `MANUAL_REVIEW` needs reason `MEDIUM_RISK_SCORE`; a score-based `REJECT` needs reason `HIGH_RISK_SCORE`. The current code hard-codes an empty string on every path, unconditionally.

**Proof from real data:** every `MANUAL_REVIEW` row across your entire `results.jsonl` (roughly 20 of them) is missing its `"reason"` field — not sometimes, every single time, with zero exceptions. That kind of universal, exception-free absence is exactly the signature of "this branch always returns the same hard-coded empty value," as opposed to a data problem that would show up inconsistently.

**Impact:** the `reason` field — which the spec calls *"required for `MANUAL_REVIEW` and `REJECT`"* — is silently missing from output whenever the rejection came from the scoring stage rather than the category-check stage (category-stage rejections are unaffected, since those set their reason directly in `check_category`, not here).

---

## Bug 7 — JSON string escaping writes the wrong character for 6 of 7 cases

**Where:** `src/returns/file_io.cpp`, inside `json_escape(...)` (see [[06-Walkthrough-FileIO]])

**Current code:**
```cpp
switch (c) {
    case '"':  out << "\\\""; break;   // correct, by coincidence
    case '\\': out << "\\\""; break;   // wrong — should be \\
    case '\b': out << "\\\""; break;   // wrong — should be \b
    case '\f': out << "\\\""; break;   // wrong — should be \f
    case '\n': out << "\\\""; break;   // wrong — should be \n
    case '\r': out << "\\\""; break;   // wrong — should be \r
    case '\t': out << "\\\""; break;   // wrong — should be \t
    ...
}
```

**What it should be:**
```cpp
switch (c) {
    case '"':  out << "\\\""; break;
    case '\\': out << "\\\\"; break;
    case '\b': out << "\\b";  break;
    case '\f': out << "\\f";  break;
    case '\n': out << "\\n";  break;
    case '\r': out << "\\r";  break;
    case '\t': out << "\\t";  break;
    ...
}
```

**Why this is a bug:** every `case` should emit *that character's own* escape sequence. Instead, six of the seven cases emit the quote-escape sequence (`\"`) regardless of which character actually triggered them — the classic signature of copy-pasting one working line six times and never updating the payload on five of the copies.

**Why this won't show up if you just run the program on the sample data and eyeball the output:** none of `request_id`, `decision`, or `reason` in this project's data ever contain a backslash, tab, newline, or other control character — they're all plain enum-like words (`AUTO_APPROVE`, `r101`, and so on). This bug is invisible on the given sample data no matter how many times you run it, and would only surface with different input (e.g. an account ID or reason string containing a literal backslash or tab). This is exactly why reading code carefully matters even when a test run "looks fine" — some bugs are only reachable through inputs the sample data never exercises.

**Impact:** any string value written to output that happens to contain a backslash, backspace, form feed, newline, carriage return, or tab would be serialized as `\"` in its place, producing corrupted, likely unparseable JSON.

---

## Bug 8 — `write_results` writes the risk score into the `request_id` field, and can crash

**Where:** `src/returns/file_io.cpp`, inside `write_results(...)` (see [[06-Walkthrough-FileIO]])

**Current code:**
```cpp
for (const auto& result : results) {
    std::ostringstream line;
    line << '{';
    line << "\"request_id\":" << result.risk_score.value() << ',';   // <-- wrong field, wrong data, unchecked .value()
    ...
```

**What it should be:**
```cpp
for (const auto& result : results) {
    std::ostringstream line;
    line << '{';
    line << "\"request_id\":" << json_escape(result.request_id) << ',';
    ...
```

**Why this is a bug, in two separate ways:**
1. **Wrong field entirely.** The `"request_id"` JSON key is populated with `result.risk_score`'s value instead of `result.request_id` — so the output's request identifiers would be numbers (or missing) instead of strings like `"r101"`, and `result.request_id` is never written anywhere in the file at all.
2. **Unchecked `.value()` on an optional that is allowed to be empty.** `risk_score` is `std::nullopt` for every request rejected in Step 1 (see [[02-Cpp-Concepts-You-Need]] section 9 and [[04-Data-Models-Reconstructed]]). Calling `.value()` on an empty `std::optional` throws `std::bad_optional_access`. Since `main.cpp` wraps the whole pipeline in a single `try`/`catch` (see [[08-Walkthrough-Main]]), the very first category-rejected request in the input would throw here, and the program would abort mid-write instead of finishing the output file.

**Why you don't see evidence of this in `results.jsonl`:** this bug corrupts the raw bytes `write_results` produces, but `results.jsonl` in your project looks like a clean, correctly-labeled table (`request_id` really does show `"r101"` etc.) — which is consistent with `results.jsonl` being a copy of a results *display* (e.g. an assessment UI showing the buggy program's computed values in a table, matched back up to input order) rather than the literal raw file bytes `write_results` wrote. Either way, this bug is unambiguous from reading the code alone — no output evidence is needed to be confident in it.

**Impact:** this is the most severe bug on the list. It doesn't just produce wrong numbers — for any input containing even one category-rejected request (and your sample data has several), it can crash the entire program before it finishes writing output, and even in a version that avoided crashing, every single line's `request_id` would still be wrong.

---

## Summary table

| # | File | Function | One-line description |
|---|---|---|---|
| 1 | `parser.cpp` | `parse_account_profile_object` | `return_history_count` discarded via `skip_value()` instead of parsed |
| 2 | `parser.cpp` | `parse_return_request_object` | `account_age_days` discarded via `skip_value()` instead of parsed |
| 3 | `engine.cpp` | `Engine::build` | transitive closure sets `changed = false` instead of `true`, so it only sweeps once |
| 4 | `engine.cpp` | `check_category` | return-window comparison (`days_since_purchase` vs `allowed_return_window_days`) doesn't exist |
| 5 | `engine.cpp` | `calculate_risk_score` | final score is never clamped to the documented 0-100 range |
| 6 | `engine.cpp` | `map_decision` | reason is hard-coded to `""` for every band, instead of `MEDIUM_RISK_SCORE`/`HIGH_RISK_SCORE` |
| 7 | `file_io.cpp` | `json_escape` | 6 of 7 special-character cases emit the wrong escape sequence (all emit `\"`) |
| 8 | `file_io.cpp` | `write_results` | writes `risk_score` into the `request_id` key; unchecked `.value()` on a possibly-empty optional |

Next: [[11-Debugging-With-AI-Playbook]]

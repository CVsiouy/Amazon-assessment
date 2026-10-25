---
tags: [amazon-assessment, cpp, debugging, trace]
---

# Tracing Requests By Hand, End to End

This is the single most valuable habit for this kind of assessment: pick a real input, compute the *correct* answer yourself using only the spec (never the code), then compute what the *actual code* would produce, and compare. Every gap between the two numbers is a clue, and often points straight at a specific line. This note does that, twice, using your own real data files, and then shows how the technique generalizes.

You have an extra advantage most debugging sessions don't: your project root contains `results.jsonl`, which — based on the analysis below — is almost certainly a hand-copy of the *actual output the buggy program produced* during your assessment (it has the same kind of copy-typos as your code, like `AUTU_APPROVE` and `MANTAL_REVIEW`, which are clearly meant to be `AUTO_APPROVE`/`MANUAL_REVIEW`). That makes it a genuine oracle you can check your reasoning against.

## Trace 1 — `r101`, a fully-correct computation, and the fully-buggy one

Request: `{"request_id":"r101","account_id":"u001","category":"ELECTRONICS","days_since_purchase":12,"order_value_usd":250,"account_age_days":400}`

### What the spec says the answer should be

- **Step 1:** ELECTRONICS is returnable, window 30 days. `12 ≤ 30` → passes.
- **Step 2a:** `u001` is linked (via `data/account_links.jsonl`) in a chain: `u001 → u002 → u003 → u004 → u005`. Correctly closed, that's one group of 5 accounts. Their `return_history_count` (from `data/account_profiles.jsonl`): `u001=2, u002=0, u003=0, u004=2, u005=1`. Total = **5**.
- **Step 2b:** `total_return_history=5` → bracket "3-5" → **15 points**. `account_age_days=400` → bracket "180+" → **0 points**. `order_value_usd=250` → bracket "200-499" → **10 points**.
- **Step 2c:** `15 + 0 + 10 = 25`. Already inside 0-100.
- **Step 3:** `25` → **AUTO_APPROVE**, no reason.

**Correct answer: `risk_score: 25, decision: AUTO_APPROVE`.**

### What your `results.jsonl` actually shows for `r101`

```json
{ "request_id":"r101", "risk_score":55, "decision":"MANUAL_REVIEW"}
```

**55, MANUAL_REVIEW** — not 25, not AUTO_APPROVE. That's not a rounding difference or a coincidence; it's a 30-point gap that flipped the decision band entirely. Let's find out exactly why, one bug at a time.

### Isolating exactly which wrong assumption produces 55

Start from the buggy `parse_account_profile_object()` (see [[05-Walkthrough-Parser]] and Bug 1 in [[10-The-8-Bugs]]): it calls `skip_value()` instead of `parse_int()` for `return_history_count`, so **every** account's `return_history_count` — including `u001`'s own — is silently discarded and left at its default, `0`. That's true regardless of whether the linked-account grouping is even correct, because it affects the profile lookup itself, not the grouping. So under the actual buggy code:

- `total_return_history` = 0 (own) + 0 (all linked, also zeroed) = **0** → bracket "0-2" → **0 points** (correct code would have given 15 points here — a 15-point gap).

Next, the buggy `parse_return_request_object()` (Bug 2 in [[10-The-8-Bugs]]) calls `skip_value()` instead of `parse_int()` for `account_age_days`, so `req.account_age_days` is also silently `0` for every request, no matter what the input JSON said (`400`, in this case):

- `account_age_days` is read as `0`, not `400` → bracket "0-29" → **45 points** (correct code would have given 0 points here — a 45-point gap, in the opposite direction).

`order_value_usd` has no bug — it's parsed correctly — so it still contributes **10 points**, same as the correct calculation.

Add up the buggy contributions: `0 (history, bugged) + 45 (age, bugged) + 10 (order, correct) = 55`. That's exactly the `55` in `results.jsonl`. The two bugs partially offset each other in this particular case (one subtracts 15, the other adds 45, net +30), which is exactly why you can't spot a bug just by "the number looks kind of plausible" — you have to isolate one variable at a time, the way we just did, to see that *both* the history and the age computation are wrong, even though the final total happens to still look like a normal integer.

### Why the reason field is missing too

`results.jsonl`'s `r101` line has no `"reason"` key at all, despite landing in `MANUAL_REVIEW`, which the spec requires a reason for (`MEDIUM_RISK_SCORE`). Every `MANUAL_REVIEW` and score-based `REJECT` row in your entire `results.jsonl` is missing its reason — that consistent, universal absence (never appearing even once across ~20 such rows) is much stronger evidence than a single missing field would be. It points directly at `map_decision` (Bug 6 in [[10-The-8-Bugs]]), which currently returns `""` unconditionally.

## Trace 2 — `r105`, proving the missing return-window check

Request: `{"request_id":"r105","account_id":"u001","category":"ELECTRONICS","days_since_purchase":35,"order_value_usd":250,"account_age_days":400}`

Notice this is the *same account* as `r101`, just filed later. `days_since_purchase` is `35`. ELECTRONICS allows a 30-day window. `35 > 30`.

### What the spec says

Per the README: *"If `days_since_purchase` is strictly greater than the allowed window, reject immediately: decision REJECT, reason RETURN_WINDOW_EXPIRED, risk_score null."* There is no ambiguity here — this should stop dead at Step 1.

**Correct answer: `risk_score: null, decision: REJECT, reason: RETURN_WINDOW_EXPIRED`.**

### What `results.jsonl` actually shows

```json
{ "request_id":"r105","risk_score":55, "decision":"MANUAL_REVIEW"}
```

A real number, not `null`. `MANUAL_REVIEW`, not `REJECT`. The request sailed straight past a check the spec says must reject it immediately — because, as covered in [[07-Walkthrough-Engine]], `check_category`'s body never once reads `days_since_purchase` or `allowed_return_window_days`, despite its own doc comment claiming it does. (The `55` itself is just `r101`'s same buggy math again — same account, same order value, same bugged history/age — which is a nice side confirmation that the window check truly never runs at all, rather than running and failing in some other way.)

This isn't a one-off. Your `data/returns.jsonl` contains **six** requests that exceed their category's window — one for every single returnable category in the config (`r105` ELECTRONICS, `r114` CLOTHING, `r121` BOOKS, `r125` JEWELRY, `r135` TOYS, `r137` FURNITURE) — and all six show a real score and a non-REJECT decision in `results.jsonl` instead of `null`/`REJECT`/`RETURN_WINDOW_EXPIRED`. That the sample data deliberately includes one violation per category is a strong signal this exact bug is one of the ones the assessment is designed to test for.

## Trace 3 — `r139`, proving the score needs clamping

Request: `{"request_id":"r139","account_id":"u100","category":"ELECTRONICS","days_since_purchase":5,"order_value_usd":5000,"account_age_days":25}`

This account is a good stress test: `u100` never appears in `data/account_links.jsonl` at all (no linked accounts to worry about), so the aggregation step is as simple as it gets — and its own `return_history_count` (from `data/account_profiles.jsonl`) is `19`.

### What the spec says

- **Step 1:** ELECTRONICS, `5 ≤ 30` → passes.
- **Step 2a:** No links, so `total_return_history` = just `u100`'s own count = **19**.
- **Step 2b:** `19` → bracket "11+" → **50 points**. `account_age_days=25` → bracket "0-29" → **45 points** (this is a genuinely young account, not a bug artifact — 25 really is under 30). `order_value_usd=5000` → bracket "500+" → **25 points**.
- **Step 2c:** `50 + 45 + 25 = 120`. The spec says: *"Clamp the total to 0-100."* Clamped, that's **100**.
- **Step 3:** `100` → **REJECT**, reason `HIGH_RISK_SCORE`.

**Correct answer: `risk_score: 100, decision: REJECT, reason: HIGH_RISK_SCORE`.**

This one real example proves the clamp isn't a theoretical nicety — `50 + 45 + 25` legitimately exceeds 100 for a real account in your real data, and nothing in `calculate_risk_score` (see [[07-Walkthrough-Engine]]) currently caps it. Notice also that even *without* an explicit clamp, `map_decision`'s loop wouldn't find any band covering `120` (bands only go up to `100`), so it would fall through to its hard-coded `REJECT` default — meaning for this particular request the *decision* might accidentally still come out as REJECT, but the *reported `risk_score` number* would be the out-of-spec value `120` instead of `100`, which still directly violates the documented contract ("risk_score - integer 0-100"). Don't let "the decision happened to still be right" convince you the bug doesn't matter — check the actual field values, not just the final bucket.

(Your current buggy `results.jsonl` shows `r139` as `70, MANUAL_REVIEW` — that's the history-bug and age-bug from Trace 1 acting on this request too: `0 (history, bugged) + 45 (age — 25 really is 0-29, so this one happens to be correct either way) + 25 (order) = 70`. This is a good example of a case where fixing the parsing bugs alone would already push this specific request from 70 up toward 120/100/REJECT — showing how fixing bugs in the wrong order can make *other* symptoms appear or disappear, which is exactly the next point.)

## The technique, generalized: bugs can hide other bugs

Notice something important across these three traces: the return-history bug (Bug 1) and the account-age bug (Bug 2) are so aggressive — they zero out an entire attribute for *every single request* — that they completely mask two other bugs:

- The **transitive-closure bug** (Bug 3, the `changed` flag) can only possibly matter if `return_history_count` is being read correctly in the first place. Right now, every profile reads as `0` regardless of grouping, so whether the grouping itself is correct or not is currently *unobservable* from the output. You would only start seeing evidence of Bug 3 after fixing Bug 1.
- The **missing clamp** (Bug 5) can only trigger once real point values are flowing through — with history and age both zeroed/flattened, the current maximum reachable score is `45 + 25 = 70`, which never needs clamping. You'd only start seeing scores above 100 after fixing Bugs 1 and 2.

This is a genuinely important lesson for time-boxed debugging: **fix bugs in the order the pipeline runs them**, roughly parsing → category checks → scoring → decision → output. Fixing a downstream bug first can look like it "did nothing," not because your fix was wrong, but because an upstream bug is still hiding its effect. If you fix something and the output doesn't visibly change, don't assume you made a mistake — check whether something earlier in the pipeline is still swallowing the signal.

## The technique, as a reusable checklist

1. Pick one concrete request from the real input data — not a hypothetical one.
2. Compute the correct answer **by hand, from the spec only**, showing every intermediate number (don't skip to the final decision).
3. Compute (or look up, if you have real output) what the code actually produces.
4. If they match, move to a different, more adversarial example (boundary values, linked accounts, missing optional fields, categories that don't exist).
5. If they don't match, isolate *one* input variable at a time (as in Trace 1) until you can say precisely which computed sub-value is wrong, not just that "the final number is wrong."
6. Once you know which sub-value is wrong, go find the *one function* responsible for that sub-value and read it line by line against the spec sentence describing it.

This is exactly the manual version of what you should now automate with AI — covered next in [[11-Debugging-With-AI-Playbook]].

Next: [[10-The-8-Bugs]]

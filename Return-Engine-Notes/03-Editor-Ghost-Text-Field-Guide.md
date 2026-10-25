---
tags: [amazon-assessment, cpp, inlay-hints, transcription]
---

# The Editor Ghost-Text Field Guide

This note exists so you can look at *any* weird-looking line in your typed files and immediately know: "that's ghost text, here's the pattern, here's the real line." Once this clicks, you'll be able to clean up the rest of this codebase (and read other people's IDE screenshots) far faster.

## Why this happened, mechanically

Editors like CLion, Visual Studio, and VS Code's C++ extension run a background type-checker while you look at code. To save you from having to hover over every variable to ask "what type is this," they render small annotations directly in the editor view — usually in a light grey, usually in a smaller font — showing things like:

- the type a variable was deduced to have when you wrote `auto`
- the name of a function parameter you're passing a value to, so `foo(true)` shows as `foo(enabled: true)` even though the file only contains `foo(true)`

These are called **inlay hints**. They are a rendering feature of the editor. They are never written into the file on disk. If you opened the same file in Notepad, or ran `cat` on it in a terminal, none of that grey text would be there — only the real code.

A camera photographing the screen cannot tell the difference between real, black, syntax-highlighted code and grey editor-generated hint text — both are just pixels. So the photos captured both. And when you later read the photos and typed the code back out by hand, you (reasonably!) typed everything you could see, including the grey parts, because nothing marked them as "not real."

> [!tip] For your next photographed assessment
> If you ever have to photograph code off a screen again: before taking the photo, open the editor's settings and turn off inlay hints (in most JetBrains IDEs: `Settings → Editor → Inlay Hints`, untick everything; in VS Code: `Editor: Inlay Hints` setting, set to `off`). It costs 20 seconds and will save you hours. If turning them off isn't possible, mentally note where the grey text starts and stops in the photo *before* you start transcribing — anything grey, skip it.

## The four repeating patterns

Every single ghost-text line in your codebase is one of these four shapes. Learn these four, and you can clean up anything.

### Pattern A — type hint after a range-for loop variable

**What you typed:**
```cpp
for (const auto &rule : ScoringRule const & : scoring_rules_)
```
**What it actually is:**
```cpp
for (const auto &rule : scoring_rules_)
```
The editor was showing you that `rule`'s deduced type is `ScoringRule const &`. Delete everything between the *first* `:` and the *second* `:`, and delete the now-extra colon, leaving exactly one `:` before the container name.

### Pattern B — type hint after an `auto` variable name

**What you typed:**
```cpp
auto it : const_iterator = category_map_.find(req.category);
```
**What it actually is:**
```cpp
auto it = category_map_.find(req.category);
```
The editor was showing that `it`'s deduced type is (some long iterator type, abbreviated to `const_iterator`). Same fix: delete the `: TYPE` before the `=`.

This pattern also appears with the type written out in full, e.g. `auto engine: Engine = returns::Engine::build(...)` → `auto engine = returns::Engine::build(...)`, and even more corrupted where the `=` itself got dropped entirely during hand-copying, e.g. `auto results: std::vector<RoutingResult> engine.evaluate(requests);` → `auto results = engine.evaluate(requests);` (note: putting the `=` back is a separate, ordinary transcription slip stacked on top of the ghost-text — see the note at the bottom of this file).

### Pattern C — parameter name hint before an argument

**What you typed:**
```cpp
account_groups[link.primary_account_id].insert(x : link.related_account_id);
```
**What it actually is:**
```cpp
account_groups[link.primary_account_id].insert(link.related_account_id);
```
The editor shows the *name of the parameter* the function declares, followed by a colon, before every argument you pass — much like how some languages let you call `insert(value: x)` for readability. C++ doesn't have that syntax at the call site at all; this is 100% a rendering hint. Strip `NAME :` immediately before every argument.

This is the single most common pattern in your files — it happens on nearly every function call with an argument, across all four `.cpp` files: `find(x : ...)`, `substr(pos: start, n: end - start)`, `setfill(c: '0')`, `setw(n: 4)`, `reserve(n : ...)`, and so on.

### Pattern D — type hints inside a structured binding

**What you typed:**
```cpp
auto [cat_decision : basic_string<char>, cat_reason : basic_string<char>] = check_category(req);
```
**What it actually is:**
```cpp
auto [cat_decision, cat_reason] = check_category(req);
```
Same idea as Pattern B, just inside the `[...]` unpacking syntax, and applied to both names at once. `basic_string<char>` is the real, unaliased name of the type that `std::string` is a shorthand for — the editor sometimes prefers showing you the "real" template name instead of the friendly alias.

## Full catalogue: every ghost-text instance found, by file

Use this as a direct find-and-replace guide. Anything not listed here in a file is either already clean or is a genuine typo, not ghost text (see the note at the end, and the full mechanical list in [[12-Pre-Compile-Cleanup-Checklist]]).

### `src/returns/engine.cpp`

| Line (as you typed it) | Pattern | Real line |
|---|---|---|
| `for (const auto &rule : CategoryRule const & : category_rules)` | A | `for (const auto &rule : category_rules)` |
| `for (const auto &profile : AccountProfile const & : profiles)` | A | `for (const auto &profile : profiles)` |
| `for (const auto &link : AccountLink const & : links)` | A | `for (const auto &link : links)` |
| `account_groups[link.primary_account_id].insert(x : link.related_account_id);` | C | `account_groups[link.primary_account_id].insert(link.related_account_id);` |
| `for (auto &[id : const basic_string<char>, group] : account_groups)` | D | `for (auto &[id, group] : account_groups)` |
| `for (const auto &member : const basic_string<char> & : group)` | A | `for (const auto &member : group)` |
| `auto it : iterator = account_groups.find(x : member);` | B + C | `auto it = account_groups.find(member);` |
| `for (const auto &transitive : const basic_string<char> & : it->second)` | A | `for (const auto &transitive : it->second)` |
| `expanded.find(x : transitive)` | C | `expanded.find(transitive)` |
| `expanded.insert(x : transitive);` | C | `expanded.insert(transitive);` |
| `return {x : decision::REJECT, y : reason::CATEGORY_NON_RETURNABLE};` (x2, similar) | C-like | `return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};` |
| `auto it : const_iterator = category_map_.find(x : req.category);` | B + C | `auto it = category_map_.find(req.category);` |
| `auto profile_it : const_iterator = profile_map_.find(x : account_id);` | B + C | `auto profile_it = profile_map_.find(account_id);` |
| `auto group_it : const_iterator = account_groups_.find(x : account_id);` | B + C | `auto group_it = account_groups_.find(account_id);` |
| `for (const auto &linked_id : const basic_string<char> & : group_it->second)` | A | `for (const auto &linked_id : group_it->second)` |
| `auto linked_profile : const_iterator = profile_map_.find(x : linked_id);` | B + C | `auto linked_profile = profile_map_.find(linked_id);` |
| `for (const auto &rule : ScoringRule const & : scoring_rules_)` | A | `for (const auto &rule : scoring_rules_)` |
| `for (const auto &range : ScoringRange count & : rule.ranges)` | A | `for (const auto &range : rule.ranges)` |
| `for (const auto &band : DecisionBand const & : decision_bands_)` | A | `for (const auto &band : decision_bands_)` |
| `return {x : band.decision, y : ""};` | C-like | `return {band.decision, ""};` |
| `return {x : decision::REJECT, y : ""};` | C-like | `return {decision::REJECT, ""};` |
| `for (const auto &req : ReturnRequest const & : requests)` | A | `for (const auto &req : requests)` |
| `results.reserve(n : requests.size());` | C | `results.reserve(requests.size());` |
| `auto [cat_decision : basic_string<char>, cat_reason : basic_string<char>] = check_category(req);` | D | `auto [cat_decision, cat_reason] = check_category(req);` |
| `aggregate_return_history(account_id : req.acount_id)` | C (+ typo, see below) | `aggregate_return_history(req.account_id)` |
| `calculate_risk_score(req, total_return_history : total_history)` | C | `calculate_risk_score(req, total_history)` |
| `auto [decision : basic_string<char>, reason : basic_string<char>] = map_decision(risk_score);` | D | `auto [decision, reason] = map_decision(risk_score);` |
| `results.push_back(x : std::move(out));` (x2) | C | `results.push_back(std::move(out));` |
| return type declarations written as `Engine(category_map : std::move(category_map), ...)` in the `build()` return statement | C | `Engine(std::move(category_map), scoring_rules, decision_bands, std::move(profile_map), std::move(account_groups))` |

### `src/returns/file_io.cpp`

| Line (as you typed it) | Pattern | Real line |
|---|---|---|
| `value.substr(pos: start, n: end - start);` | C | `value.substr(start, end - start);` |
| `std::setfill(c: '0')` | C | `std::setfill('0')` |
| `std::setw(n: 4)` | C | `std::setw(4)` |
| `std::filesystem::path p(source: path);` | C | `std::filesystem::path p(path);` |
| `std::filesystem::create_directories(p: parent, &:ec);` | C | `std::filesystem::create_directories(parent, ec);` |
| `throw std::runtime_error(arg: "...")` (appears many times) | C | `throw std::runtime_error("...")` |
| `std::ifstream file(s: path);` (appears 3x with different var names) | C | `std::ifstream file(path);` |
| `std::getLine(&is: file, &str: line)` / `std::getline(&is : file, &str: line)` | C (+ one typo, see below) | `std::getline(file, line)` |
| `profiles.push_back(x: parse_account_profile_line(line: stripped));` | C | `profiles.push_back(parse_account_profile_line(stripped));` |
| `links.push_back(x: parse_account_link_line(line: stripped));` | C | `links.push_back(parse_account_link_line(stripped));` |
| `requests.push_back(x: parse_return_request_line(line: stripped));` | C | `requests.push_back(parse_return_request_line(stripped));` |
| `std::ofstream file(s:path);` | C | `std::ofstream file(path);` |
| `for (const auto& result: RoutingResult const & : results)` | A | `for (const auto& result : results)` |
| `json_espace(value: result.decision)` (+ typo, see below) | C | `json_escape(result.decision)` |
| `json_escape(value: result.reason)` | C | `json_escape(result.reason)` |
| `read_category_rules(const std::string& path)` calling `parse_category_rules_json(json_doc: content)` | C | `parse_category_rules_json(content)` |
| (same pattern for `parse_scoring_rules_json`, `parse_decision_bands_json`) | C | drop the `json_doc:` label |

### `src/returns/main.cpp`

| Line (as you typed it) | Pattern | Real line |
|---|---|---|
| `auto category_rules: std::vector<CategoryRule> = returns::read_category_rules(path: category_rules_path);` | B + C | `auto category_rules = returns::read_category_rules(category_rules_path);` |
| `auto scoring_rules: std::vector<ScoringRule> = returns::read_scoring_rules(path: scoring_rules_path);` | B + C | `auto scoring_rules = returns::read_scoring_rules(scoring_rules_path);` |
| `auto decision_bands: std::vector<DecisionBand> = returns::read_decision_bands(path: decision_bands_path);` | B + C | `auto decision_bands = returns::read_decision_bands(decision_bands_path);` |
| `auto profiles: std::vector<AccountProfile> = returns::read_account_profiles(path: profiles_path);` | B + C | `auto profiles = returns::read_account_profiles(profiles_path);` |
| `auto links: std::vector<AccountLink> = returns::read_account_links(path: links_path);` | B + C | `auto links = returns::read_account_links(links_path);` |
| `auto requests: std::vector<ReturnRequest> = returns::read_return_requests(path: returns_path);` | B + C | `auto requests = returns::read_return_requests(returns_path);` |
| `auto engine: Engine = returns::Engine::build(category_rules, scoring_rules, decision_bands, profiles, links);` | B | `auto engine = returns::Engine::build(category_rules, scoring_rules, decision_bands, profiles, links);` |
| `auto results: std::vector<RoutingResult> engine.evaluate(requests);` | B (+ missing `=`, see below) | `auto results = engine.evaluate(requests);` |
| `returns::write_results(path: out_path, results);` | C | `returns::write_results(out_path, results);` |

### `src/returns/parser.cpp`

| Line (as you typed it) | Pattern | Real line |
|---|---|---|
| `expect(c: '{')` / `expect(c: ':')` / `expect(c: ',')` / `expect(c: '"')` (appears dozens of times) | C | `expect('{')` / `expect(':')` / etc. |
| `rules.push_back(x: parse_category_rule_object());` | C | `rules.push_back(parse_category_rule_object());` |
| `rules.push_back(x: parse_scoring_rule_object());` | C | `rules.push_back(parse_scoring_rule_object());` |
| `bands.push_back(x: parse_direction_band_object());` | C (+ a real name mismatch, see [[12-Pre-Compile-Cleanup-Checklist]]) | `bands.push_back(parse_decision_band_object());` |
| `ranges.push_back(x: r);` | C | `ranges.push_back(r);` |
| `std::atoi(nptr: json_.substr(pos: start, n: pos_ - start).c_str())` | C | `std::atoi(json_.substr(start, pos_ - start).c_str())` |
| `skip_literal(lit: "true")` / `skip_literal(lit: "false")` | C | `skip_literal("true")` / `skip_literal("false")` |
| `JsonParser parser(json: json_doc);` / `JsonParser parser(json: line);` (appears 6x) | C | `JsonParser parser(json_doc);` / `JsonParser parser(line);` |

## Not ghost text — plain transcription typos

A smaller number of broken lines are *not* editor hints — they're ordinary hand-copying slips: a dropped letter, a swapped letter, a misread symbol. These are just as important to fix (the program won't compile until they are), but they're a different phenomenon, and there's no "pattern" to learn — you just have to fix each one individually. Examples: `heck_category` instead of `check_category`, `attribute_valye` instead of `attribute_value`, `req.acount_id` instead of `req.account_id`, `!-` instead of `!=`, `lkey` instead of `key`, `sscoring_rules_path` instead of `scoring_rules_path`, `getLine` (capital L) instead of `getline`, `json_espace` instead of `json_escape`, `stds::vector` instead of `std::vector`, `parse_direction_band_object` instead of `parse_decision_band_object`, and `category_map[rule, category]` instead of `category_map[rule.category]` (a comma where a dot belonged).

The complete, exhaustive, checkbox-able list of these — organized file by file so you can fix them in one pass — lives in [[12-Pre-Compile-Cleanup-Checklist]]. None of these are among "the 8 bugs" your research turned up; they're artifacts of the photo → handwriting → typing pipeline, and the assessment's actual 8 bugs are logic bugs that exist underneath all this noise, fully covered in [[10-The-8-Bugs]].

Next: [[04-Data-Models-Reconstructed]]

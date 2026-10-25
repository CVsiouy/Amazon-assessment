---
tags: [amazon-assessment, cpp, checklist]
---

# Pre-Compile Cleanup Checklist

A straight-through, top-to-bottom, file-by-file checklist of every mechanical fix needed before this project can even attempt to compile — ghost-text stripping (see [[03-Editor-Ghost-Text-Field-Guide]] for *why* each of these looks the way it does) and plain hand-copy typos, all in one place. None of these are "the 8 bugs" (those are logic bugs, listed separately in [[10-The-8-Bugs]]) — this is purely "make it parse and link."

Work through one file at a time, top to bottom, checking items off as you go. After finishing a file, try building; fix whatever the compiler complains about next (it will likely find a few more things not listed here, and that's fine — this list is thorough but your compiler is the real authority).

## 0. Headers first

- [ ] Create the contents of `include/returns/models.hpp`, `engine.hpp`, `parser.hpp`, and `file_io.hpp` — currently all four are empty. Use the reconstruction in [[04-Data-Models-Reconstructed]] as a starting point, and correct anything that differs from your photos if you have them.

## 1. `src/returns/engine.cpp`

- [ ] `for (const auto &rule : CategoryRule const & : category_rules)` → `for (const auto &rule : category_rules)`
- [ ] `category_map[rule, category] = rule;` → `category_map[rule.category] = rule;` (comma should be a dot)
- [ ] `for (const auto &profile : AccountProfile const & : profiles)` → `for (const auto &profile : profiles)`
- [ ] `for (const auto &link : AccountLink const & : links)` → `for (const auto &link : links)`
- [ ] `account_groups[link.primary_account_id].insert(x : link.related_account_id);` → `.insert(link.related_account_id);`
- [ ] `for (auto &[id : const basic_string<char>, group] : account_groups)` → `for (auto &[id, group] : account_groups)`
- [ ] `for (const auto &member : const basic_string<char> & : group)` → `for (const auto &member : group)`
- [ ] `auto it : iterator = account_groups.find(x : member);` → `auto it = account_groups.find(member);`
- [ ] `for (const auto &transitive : const basic_string<char> & : it->second)` → `for (const auto &transitive : it->second)`
- [ ] `expanded.find(x : transitive)` → `expanded.find(transitive)`
- [ ] `expanded.insert(x : transitive);` → `expanded.insert(transitive);`
- [ ] the function name `heck_category` → `check_category` (must match the header declaration and the call site below)
- [ ] `auto it : const_iterator = category_map_.find(x : req.category);` → `auto it = category_map_.find(req.category);`
- [ ] both `return {x : decision::REJECT, y : reason::CATEGORY_NON_RETURNABLE};` → `return {decision::REJECT, reason::CATEGORY_NON_RETURNABLE};`
- [ ] `return {x : "", y : ""};` → `return {"", ""};`
- [ ] `auto profile_it : const_iterator = profile_map_.find(x : account_id);` → `auto profile_it = profile_map_.find(account_id);`
- [ ] `auto group_it : const_iterator = account_groups_.find(x : account_id);` → `auto group_it = account_groups_.find(account_id);`
- [ ] `for (const auto &linked_id : const basic_string<char> & : group_it->second)` → `for (const auto &linked_id : group_it->second)`
- [ ] `auto linked_profile : const_iterator = profile_map_.find(x : linked_id);` → `auto linked_profile = profile_map_.find(linked_id);`
- [ ] `for (const auto &rule : ScoringRule const & : scoring_rules_)` → `for (const auto &rule : scoring_rules_)`
- [ ] `attribute_valye = req.order_value_usd;` → `attribute_value = req.order_value_usd;`
- [ ] `for (const auto &range : ScoringRange count & : rule.ranges)` → `for (const auto &range : rule.ranges)`
- [ ] `for (const auto &band : DecisionBand const & : decision_bands_)` → `for (const auto &band : decision_bands_)`
- [ ] `return {x : band.decision, y : ""};` → `return {band.decision, ""};`
- [ ] `return {x : decision::REJECT, y : ""};` → `return {decision::REJECT, ""};`
- [ ] `for (const auto &req : ReturnRequest const & : requests)` → `for (const auto &req : requests)`
- [ ] `results.reserve(n : requests.size());` → `results.reserve(requests.size());`
- [ ] `auto [cat_decision : basic_string<char>, cat_reason : basic_string<char>] = check_category(req);` → `auto [cat_decision, cat_reason] = check_category(req);`
- [ ] both `results.push_back(x : std::move(out));` → `results.push_back(std::move(out));`
- [ ] `aggregate_return_history(account_id : req.acount_id)` → `aggregate_return_history(req.account_id)` (note: `acount_id` → `account_id` too)
- [ ] `calculate_risk_score(req, total_return_history : total_history)` → `calculate_risk_score(req, total_history)`
- [ ] `auto [decision : basic_string<char>, reason : basic_string<char>] = map_decision(risk_score);` → `auto [decision, reason] = map_decision(risk_score);`
- [ ] the `Engine(...)` construction at the end of `build()` — strip every `name :` label (`category_map :`, `profile_map :`, `account_groups :`) before its value

Then apply the actual logic fixes for Bugs 3, 4, 5, 6 from [[10-The-8-Bugs]] in this same file.

## 2. `src/returns/file_io.cpp`

- [ ] `value.substr(pos: start, n: end - start);` → `value.substr(start, end - start);`
- [ ] all six wrong `case` bodies in `json_escape` (this is Bug 7 in [[10-The-8-Bugs]] — a logic fix, not just cleanup, do it properly here)
- [ ] `std::setfill(c: '0')` → `std::setfill('0')`
- [ ] `std::setw(n: 4)` → `std::setw(4)`
- [ ] `std::filesystem::path p(source: path);` → `std::filesystem::path p(path);`
- [ ] `std::filesystem::create_directories(p: parent, &:ec);` → `std::filesystem::create_directories(parent, ec);` (you'll need to declare `std::error_code ec;` before this line if it isn't already there)
- [ ] every `throw std::runtime_error(arg: "...")` → `throw std::runtime_error("...")`
- [ ] every `std::ifstream file(s: path);` → `std::ifstream file(path);`
- [ ] `std::getLine(&is: file, &str: line)` in `read_account_profiles` → `std::getline(file, line)` (note capital `L` → lowercase `l`, on top of the ghost text)
- [ ] `std::getline(&is : file, &str: line)` (the other two occurrences) → `std::getline(file, line)`
- [ ] `profiles.push_back(x: parse_account_profile_line(line: stripped));` → `profiles.push_back(parse_account_profile_line(stripped));`
- [ ] `links.push_back(x: parse_account_link_line(line: stripped));` → `links.push_back(parse_account_link_line(stripped));`
- [ ] `requests.push_back(x: parse_return_request_line(line: stripped));` → `requests.push_back(parse_return_request_line(stripped));`
- [ ] `std::ofstream file(s:path);` → `std::ofstream file(path);`
- [ ] `for (const auto& result: RoutingResult const & : results)` → `for (const auto& result : results)`
- [ ] the `request_id`/`risk_score` line — this is Bug 8 in [[10-The-8-Bugs]], a logic fix: rewrite to `line << "\"request_id\":" << json_escape(result.request_id) << ',';` and keep the existing `has_value()`-guarded block for `risk_score` right after it
- [ ] `json_espace(value: result.decision)` → `json_escape(result.decision)` (note the missing `c` in `espace`, on top of the ghost text)
- [ ] `json_escape(value: result.reason)` → `json_escape(result.reason)`
- [ ] every `parse_..._json(json_doc: content)` call → drop the `json_doc:` label, e.g. `parse_category_rules_json(content)`

## 3. `src/returns/main.cpp`

- [ ] `sscoring_rules_path = argv[++i];` → `scoring_rules_path = argv[++i];` (double `s` typo)
- [ ] `auto category_rules: std::vector<CategoryRule> = returns::read_category_rules(path: category_rules_path);` → `auto category_rules = returns::read_category_rules(category_rules_path);`
- [ ] same ghost-text pattern for `scoring_rules`, `decision_bands`, `profiles`, `links`, `requests` — strip `: Type` and the `path:` label on each
- [ ] `auto engine: Engine = returns::Engine::build(...)` → `auto engine = returns::Engine::build(category_rules, scoring_rules, decision_bands, profiles, links);`
- [ ] `auto results: std::vector<RoutingResult> engine.evaluate(requests);` → `auto results = engine.evaluate(requests);` (note: this line is also missing its `=`, not just carrying ghost text)
- [ ] `returns::write_results(path: out_path, results);` → `returns::write_results(out_path, results);`

## 4. `src/returns/parser.cpp`

- [ ] every `expect(c: '{')`, `expect(c: ':')`, `expect(c: ',')`, `expect(c: '"')` (dozens of occurrences throughout the file) → drop the `c:` label, e.g. `expect('{')`
- [ ] `if (lkey == "scoring_rules"){` → `if (key == "scoring_rules"){` (should match the `std::string key = parse_string();` declared just above it)
- [ ] `rules.push_back(x: parse_category_rule_object());` → `rules.push_back(parse_category_rule_object());`
- [ ] `rules.push_back(x: parse_scoring_rule_object());` → `rules.push_back(parse_scoring_rule_object());`
- [ ] `bands.push_back(x: parse_direction_band_object());` → `bands.push_back(parse_decision_band_object());` (note: `direction` → `decision`, not just ghost text — this needs to match the function actually defined later in the file)
- [ ] in `parse_account_profile_object()`: `else if(key == "return_history_count"){ skip_value(); }` → this is Bug 1 in [[10-The-8-Bugs]], a logic fix: change to `p.return_history_count = parse_int();`
- [ ] in `parse_return_request_object()`: `else if(key == "account_age_days"){ skip_value(); }` → this is Bug 2 in [[10-The-8-Bugs]], a logic fix: change to `r.account_age_days = parse_int();`
- [ ] `ranges.push_back(x: r);` → `ranges.push_back(r);`
- [ ] `std::atoi(nptr: json_.substr(pos: start, n: pos_ - start).c_str())` → `std::atoi(json_.substr(start, pos_ - start).c_str())`
- [ ] `skip_literal(lit: "true")` / `skip_literal(lit: "false")` → `skip_literal("true")` / `skip_literal("false")`
- [ ] `while (pos_ < json_.size() && json_[pos_] !- '"')` in `parse_string()` → `json_[pos_] != '"'` (a dash where an equals sign belonged)
- [ ] every `JsonParser parser(json: json_doc);` / `JsonParser parser(json: line);` (six occurrences at the bottom of the file) → drop the `json:` label
- [ ] `stds::vector<CategoryRule> parse_category_rules_json(...)` → `std::vector<CategoryRule> parse_category_rules_json(...)` (extra `s`)

## 5. After all of the above

- [ ] Try to build the project. Fix whatever the compiler reports that isn't already covered above (there may be a small number of things this list missed — that's expected and fine).
- [ ] Once it builds and runs, apply the 8 real fixes from [[10-The-8-Bugs]] if you haven't folded them in already while going through this list.
- [ ] Re-run and compare against the trace examples in [[09-End-to-End-Trace]] to confirm `r101` now produces `25, AUTO_APPROVE`, `r105` now produces `null, REJECT, RETURN_WINDOW_EXPIRED`, and `r139` now produces `100, REJECT, HIGH_RISK_SCORE`.

That's the whole vault. Back to [[00-Start-Here]].

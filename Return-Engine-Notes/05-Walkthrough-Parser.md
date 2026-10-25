---
tags: [amazon-assessment, cpp, walkthrough, parser]
---

# Walkthrough: `parser.cpp`

This file's one job: turn raw JSON text (as a `std::string`) into the C++ structs from [[04-Data-Models-Reconstructed]]. Nothing in this file reads a file from disk — that's [[06-Walkthrough-FileIO]]'s job. This file only ever sees text that's already been loaded into memory.

## Why a hand-written JSON parser at all?

Real projects would pull in a JSON library (like `nlohmann::json` or `rapidjson`). This assessment deliberately writes a tiny one from scratch — probably so the interview round can hide bugs inside "boring plumbing code" rather than only in the interesting business logic. The comment at the top says as much:

```cpp
/// Lightweight JSON parser for known input shapes.
/// Supports the subset needed for configuration JSON and JSONL records.
```

It doesn't need to handle arbitrary JSON — only the exact shapes this project's own data files use.

## The core idea: a cursor crawling through a string

The whole file is built around one class, `JsonParser`, that holds two things:

```cpp
const std::string& json_;   // the text being parsed (a reference, no copy)
std::size_t pos_;           // "we've read up through this index so far"
```

Every parsing function is really just: look at `json_[pos_]` (the character *right now*), decide what to do based on it, and advance `pos_` forward. Think of `pos_` as your finger sliding along the text, left to right, never going backward.

Three tiny helper methods make this bearable:

```cpp
char peek() const {
    return pos_ < json_.size() ? json_[pos_] : '\0';
}
```
"What's the character right where my finger is, without moving it?" Returns `'\0'` (a null character) if we've run off the end, so callers never read out of bounds.

```cpp
void expect(char c) {
    if (peek() != c) {
        throw std::runtime_error("JSON parse error: expected " + std::string(1, c));
    }
    pos_++;
}
```
"The next character had better be exactly `c` — if not, blow up with a clear error. If it matches, move my finger past it." This is how the parser enforces JSON's required punctuation (`{`, `}`, `:`, `,`, `"`) without writing an `if` for each one everywhere.

```cpp
void skip_ws() {
    while (pos_ < json_.size() && std::isspace(static_cast<unsigned char>(json_[pos_]))) {
        pos_++;
    }
}
```
"Slide my finger past any spaces/tabs/newlines sitting here." JSON allows whitespace almost anywhere, so this gets called constantly between tokens.

## The lowest-level building blocks

**`parse_string()`** — reads a quoted JSON string like `"ELECTRONICS"` and returns `ELECTRONICS` (no quotes) as a `std::string`. It walks character by character until it hits an unescaped closing `"`, handling the common escape sequences (`\"`, `\\`, `\n`, `\r`, `\t`) along the way.

> [!bug] Ghost text + a real typo, stacked
> Your typed version has `while (pos_ < json_.size() && json_[pos_] !- '"')`. `!-` isn't a C++ operator — this is a plain misread of `!=` (a straight equals-looking symbol got read as a dash), not an inlay hint. The real line is `json_[pos_] != '"'`. See [[12-Pre-Compile-Cleanup-Checklist]].

**`parse_int()`** — reads an optional leading `-` followed by digits, and converts that substring to an `int` using `std::atoi`. It doesn't validate much (no explicit "must have at least one digit" check), but every place it's called, the caller already knows a number is expected, so that's an acceptable simplification for a "known input shapes" parser.

**`parse_bool()`** — checks if the next character is `t` (assumes `"true"` and skips exactly that many characters) or otherwise assumes `"false"`. Notice it doesn't actually verify the letters after the first one match — it trusts the input. Fine for a controlled assessment data file, not something you'd want in a parser exposed to untrusted input.

**`skip_value()`** — this one matters a lot for the bugs list, so read it carefully:

```cpp
void skip_value() {
    skip_ws();
    if (peek() == '"') { (void)parse_string(); return; }
    if (peek() == '{') { /* walk forward tracking brace depth until balanced */ return; }
    if (peek() == '[') { /* walk forward tracking bracket depth until balanced */ return; }
    // otherwise: a bare number/true/false/null — just advance until , } or ]
    while (pos_ < json_.size()) {
        char c = json_[pos_];
        if (c == ',' || c == '}' || c == ']') break;
        pos_++;
    }
}
```

This function's purpose is legitimate and important: "I found a JSON key I don't care about — move my cursor past its *value*, whatever shape that value is (string, object, array, or bare literal), without extracting it into anything." It's what makes the parser "forward compatible" — if `category_rules.json` gained a new field tomorrow that this code doesn't know about, `skip_value()` cleanly steps over it instead of crashing.

The problem is **this same function is also called for two fields that the caller does need**, which is Bugs 1 and 2 — covered in full in [[10-The-8-Bugs]], but flagged here inline as you read:

```cpp
else if (key == "return_history_count") {
    skip_value();     // <-- Bug 1: this THROWS AWAY the number instead of parsing it
}
```
inside `parse_account_profile_object()`, and
```cpp
else if (key == "account_age_days") {
    skip_value();     // <-- Bug 2: same mistake, different struct
}
```
inside `parse_return_request_object()`. In both cases, the surrounding sibling fields (`account_id`, `order_value_usd`, etc.) correctly call `parse_int()`/`parse_string()` and assign the result — only these two fields call `skip_value()` and assign nothing. That asymmetry, sitting right next to the correct pattern, is the whole bug.

## The "parse one object" functions

There are six of these, and they all follow the exact same shape, so once you understand one, you understand all six: `parse_category_rule_object()`, `parse_scoring_rule_object()`, `parse_decision_band_object()`, `parse_account_profile_object()`, `parse_account_link_object()`, `parse_return_request_object()`.

Take the simplest, `parse_account_profile_object()`, cleaned of ghost text and with Bug 1 fixed:

```cpp
AccountProfile parse_account_profile_object() {
    AccountProfile p;
    expect('{');
    while (true) {
        skip_ws();
        if (peek() == '}') { pos_++; break; }        // end of object, we're done

        std::string key = parse_string();             // read the "key" part
        skip_ws();
        expect(':');
        skip_ws();

        if (key == "account_id") {
            p.account_id = parse_string();
        } else if (key == "return_history_count") {
            p.return_history_count = parse_int();      // fixed: was skip_value()
        } else {
            skip_value();                              // any other/future key: ignore its value
        }

        skip_ws();
        if (peek() == ',') pos_++;                     // more fields follow, loop again
    }
    return p;
}
```

Read this as a recipe: *"Expect an opening brace. Then, repeatedly: if you see a closing brace, stop. Otherwise, read a key, expect a colon, and based on the key's name, either capture the value into the right field or skip over it. If there's a comma, there's more — go again."* Every one of the six object-parsers is a copy of this exact recipe with a different `if/else if` chain matching that struct's field names — which is also exactly why a bug like "one field calls `skip_value()` instead of `parse_int()`" is easy to miss by eye: five nearly-identical blocks look correct, and the sixth field, buried in the middle of one of them, quietly does the wrong thing.

## The "parse a whole document" functions

`parse_category_rules_document()`, `parse_scoring_rules_document()`, and `parse_decision_bands_document()` handle the three `.json` config files, which are each shaped like `{ "some_key": [ {...}, {...}, ... ] }` — a top-level object with exactly one key that matters, holding an array. These three functions are also nearly identical to each other: skip to the one key they care about (`"category_rules"`, `"scoring_rules"`, or `"decision_bands"`), then loop `parse_..._object()` calls across the array until `]`, and use `skip_value()` for every other top-level key (there usually aren't any, but again, it's forward-compatible).

## The public entry points at the bottom of the file

```cpp
std::vector<CategoryRule> parse_category_rules_json(const std::string& json_doc) {
    JsonParser parser(json_doc);
    return parser.parse_category_rules_document();
}
```

...and five siblings just like it. These are the only functions this file exposes to the rest of the program (declared in `parser.hpp`, see [[04-Data-Models-Reconstructed]]) — everything else (`JsonParser` itself, and all its methods) is private to this file via the anonymous `namespace { ... }` wrapping the class (see [[02-Cpp-Concepts-You-Need]] section 13). `file_io.cpp` never touches `JsonParser` directly; it only ever calls these six thin wrapper functions.

## What to take away from this file

- It's a hand-rolled, cursor-based JSON reader: one position counter, a handful of "read one token" primitives, and a repeated "read key, dispatch on its name" loop shape.
- `skip_value()` is *correct and necessary* in general — the bug isn't that it exists, it's that it got called for two fields that should have used `parse_int()` instead.
- Because six functions all share the same shape, the fastest way to sanity-check any one of them is to diff it against its siblings and ask "why does this one field look different from the others?" — that's precisely how Bugs 1 and 2 stand out.

Next: [[06-Walkthrough-FileIO]]

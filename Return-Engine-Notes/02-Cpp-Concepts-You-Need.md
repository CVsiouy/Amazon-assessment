---
tags: [amazon-assessment, cpp, fundamentals]
---

# C++ Concepts You Need For This Codebase

This is not a general C++ course. It only covers the specific handful of features this project actually uses, in the order you'll meet them. Skim the headings, read closely wherever something is unfamiliar.

## 1. Header files (`.hpp`) vs source files (`.cpp`)

C++ splits a program into two kinds of files:

- A **header** (`.hpp`) is a *promise* — "somewhere in this program, a function/class/struct with this exact name and shape exists." It contains declarations only, no actual logic.
- A **source file** (`.cpp`) is the *implementation* — the actual working code that fulfills those promises.

Why bother splitting them? So that other files can `#include` the header and call a function without needing to see (or recompile) its implementation. Your `engine.cpp` starts with:

```cpp
#include "returns/engine.hpp"
```

This means: "I am promising that everything declared in `engine.hpp` exists, and here I'm going to write the actual bodies of those things." If `engine.hpp` declares a class `Engine` with a method `evaluate(...)`, then `engine.cpp` must define `Engine::evaluate(...)` with a matching signature, or the program won't link.

This is exactly why your four `.hpp` files being empty is a real problem, not a stylistic one — right now nothing is *promised*, so the compiler has no idea `Engine`, `ReturnRequest`, etc. are supposed to exist at all when it reads other files that use them. See [[04-Data-Models-Reconstructed]].

## 2. Namespaces

```cpp
namespace returns {
    // everything in here is "returns::something"
}
```

A namespace is just a labeled box to prevent name collisions — if two different libraries both define a function called `parse`, putting one inside `namespace returns { }` means you refer to it as `returns::parse` and there's no clash. Every file in this project wraps its contents in `namespace returns { ... }`.

You'll also see small namespaces used purely to hold string constants, e.g. (reconstructed, see [[04-Data-Models-Reconstructed]]):

```cpp
namespace decision {
    inline constexpr const char* AUTO_APPROVE = "AUTO_APPROVE";
    inline constexpr const char* MANUAL_REVIEW = "MANUAL_REVIEW";
    inline constexpr const char* REJECT = "REJECT";
}
```

So when code says `decision::REJECT`, it just means "the text `\"REJECT\"`, spelled once, here, so nobody typos it differently in five different places." `::` is the **scope resolution operator** — "look inside this namespace/class for the next name."

## 3. `std::string`

C++'s built-in text type. `req.category` is a `std::string`. You can compare them with `==`, concatenate with `+`, and so on, mostly like you'd expect from any language.

## 4. `std::vector<T>`

A resizable array of `T`. `std::vector<ScoringRule>` is "a list of `ScoringRule` objects, that can grow." `.push_back(x)` appends `x` to the end. `.size()` returns the count. `.reserve(n)` pre-allocates space for `n` elements as a performance hint (it doesn't add elements, just avoids repeated reallocation while you push_back).

## 5. `std::unordered_map<K, V>`

A hash map / dictionary: look up a `V` by a `K` key in roughly constant time, with no guaranteed ordering. This codebase uses `std::unordered_map<std::string, CategoryRule>` (category name → its rule), `std::unordered_map<std::string, int>` (account id → its return count), and similar.

Two ways to access it, and the difference matters:

- `map[key]` — if `key` doesn't exist yet, **it is silently created** with a default-constructed value (for `int`, that's `0`). Convenient for *building* a map, dangerous for *reading* one, because a typo'd key won't error, it'll just quietly insert a zero/empty entry.
- `map.find(key)` — returns an **iterator**. If not found, you get a special "end" iterator (`map.end()`), which you must check for before dereferencing. Never silently inserts anything. This is the safe way to *look up* without side effects.

```cpp
auto it = category_map_.find(req.category);
if (it == category_map_.end()) {
    // not found
} else {
    // it->first is the key, it->second is the value
}
```

`it->second` is how you get the value out of an iterator — the arrow because an iterator behaves like a pointer to a key/value pair.

## 6. `std::unordered_set<T>`

Like `std::unordered_map`, but it only stores *presence*, not a key→value pair — "is this string in the set or not." Used here as `std::unordered_set<std::string>` to hold "the set of account IDs linked to this one," where you don't care about order and don't want duplicates. `.insert(x)` adds `x` (no-op if already present). `.find(x) == set.end()` means "not present."

## 7. `std::pair<A, B>`

A simple box holding exactly two values, accessed as `.first` and `.second`. This codebase returns `std::pair<std::string, std::string>` from a couple of functions to bundle "decision" and "reason" together without declaring a whole new struct for it.

```cpp
std::pair<std::string, std::string> Engine::map_decision(int risk_score) const {
    // ...
    return {"REJECT", "HIGH_RISK_SCORE"};   // first="REJECT", second="HIGH_RISK_SCORE"
}
```

## 8. Structured bindings — `auto [a, b] = ...`

A convenient way to unpack a `pair` (or any similarly "tuple-like" type) straight into two named variables in one line:

```cpp
auto [decision, reason] = map_decision(risk_score);
// decision == the pair's .first, reason == the pair's .second
```

> [!warning] This is exactly where most of the "ghost text" damage shows up in your files
> Your typed files show lines like `auto [decision : basic_string<char>, reason : basic_string<char>] = map_decision(risk_score);`. The `: basic_string<char>` parts are inlay type hints (and `basic_string<char>` is literally what `std::string` is a shorthand/alias for under the hood — the editor likes to show you the "real" underlying type). None of that belongs in the code. The real line is just `auto [decision, reason] = map_decision(risk_score);`. Full pattern catalogue in [[03-Editor-Ghost-Text-Field-Guide]].

## 9. `std::optional<T>`

A box that either contains a `T`, or contains nothing (`std::nullopt`). This models "this value might legitimately be absent" without using a magic number like `-1`. This codebase almost certainly uses `std::optional<int>` for `RoutingResult::risk_score`, because a rejected request has *no* risk score at all — not zero, genuinely absent — matching the spec's `"risk_score": null`.

```cpp
std::optional<int> risk_score = std::nullopt;   // empty
if (risk_score.has_value()) { /* ... */ }
int x = risk_score.value();       // gets the int out — THROWS an exception if it was empty!
int y = risk_score.value_or(0);   // gets the int out, or 0 if empty — never throws
```

That difference — `.value()` throws on empty, `.value_or(fallback)` doesn't — is directly responsible for one of the 8 real bugs (Bug 8 in [[10-The-8-Bugs]]), where code calls `.value()` on a `risk_score` that is allowed to be empty, without checking first.

## 10. `struct` vs `class`

In C++ these are almost the same thing; the only default difference is that `struct` members are `public` unless you say otherwise, and `class` members are `private` unless you say otherwise. Convention-wise, this codebase uses `struct` for plain data holders with no real behavior (`ReturnRequest`, `CategoryRule`, `ScoringRange`...) and `class` for the one thing that has actual behavior and hides its internals (`Engine`).

```cpp
struct ScoringRange {
    int min;
    int max;
    int points;
};   // just a labeled bundle of three ints, nothing more
```

## 11. `const` references — `const T&`

Passing a big object (like a `std::vector` with thousands of entries, or a `std::string`) *by value* means C++ copies the whole thing just to hand it to a function — wasteful. Passing `const T&` means "give the function a reference to the original object, but promise the function won't be allowed to modify it." This is why you see function signatures like:

```cpp
Engine Engine::build(
    const std::vector<CategoryRule> &category_rules,
    const std::vector<ScoringRule> &scoring_rules,
    ...
)
```

Read `const std::vector<CategoryRule> &category_rules` as: "a read-only reference to a vector of `CategoryRule`, called `category_rules` — no copy is made, and this function promises not to change it."

## 12. Range-based `for` loops

```cpp
for (const auto &rule : scoring_rules_) {
    // rule refers to each element of scoring_rules_ in turn, one at a time
}
```

Read as "for each thing in this container, call it `rule`, without copying it (`&`), and don't let me accidentally modify it (`const`)." This is by far the most common loop shape in this codebase, and also the single most ghost-text-corrupted line shape in your typed files — see [[03-Editor-Ghost-Text-Field-Guide]] Pattern A.

## 13. Anonymous namespaces — "private to this file"

```cpp
namespace returns {
namespace {
    std::string trim(const std::string& value) { /* ... */ }
} // namespace
} // namespace returns
```

An unnamed `namespace { ... }` inside a `.cpp` file means "everything in here is only visible/usable within this one file." It's how `file_io.cpp` and `parser.cpp` define small helper functions (`trim`, `json_escape`, `read_file_content`) that other files have no business calling directly — a form of encapsulation at the file level, not the class level.

## 14. Exceptions — `throw`, `try`/`catch`

C++ exceptions are how this code reports things going wrong without every function needing an "error code" return value. `throw std::runtime_error("message")` immediately stops normal execution and unwinds up the call stack until something catches it. `main.cpp` wraps the entire program in:

```cpp
try {
    // read files, build the engine, evaluate requests, write results
    return 0;
} catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    return 1;
}
```

So if *anything* throws anywhere in this program — a missing file, a malformed JSON line, or (relevantly) calling `.value()` on an empty `std::optional` — execution jumps straight to that `catch` block, prints the error, and the program exits with status `1` instead of producing `data/results.jsonl`. Keep this in mind for Bug 8: it doesn't produce "slightly wrong output," it can make the *entire program produce no output at all* for the run.

## 15. `static` member functions and `const` methods

```cpp
static Engine build(/* ... */);   // called as Engine::build(...), no existing Engine needed
std::vector<RoutingResult> evaluate(const std::vector<ReturnRequest>& requests) const;   // trailing `const`
```

`static` on a method means it belongs to the class itself, not to any particular instance — that's why `main.cpp` calls it as `returns::Engine::build(...)` rather than needing an `Engine` object first. It's essentially a labeled, namespaced regular function, and it's the conventional way to write a "factory" that constructs and returns an instance.

A trailing `const` on a non-static method (like `evaluate(...) const`) means "calling this method is promised not to modify the object it's called on." It lets you call the method on a `const Engine&` and lets the compiler catch you if you accidentally try to mutate a member field inside it.

## 16. RAII file streams — `std::ifstream` / `std::ofstream`

```cpp
std::ifstream file(path);
if (!file.is_open()) { throw std::runtime_error("failed to open file: " + path); }
```

Opening a file just means constructing one of these objects with a path. You don't need to manually close it — when the `file` variable goes out of scope (the function returns), its destructor closes the file automatically. This pattern (a resource that cleans itself up when its variable dies) is called **RAII** and is one of C++'s signature ideas; you'll see it every time a file is touched in this codebase.

`std::getline(file, line)` reads one line of text from `file` into the string `line`, and returns something falsy once there's nothing left to read — which is why `while (std::getline(file, line)) { ... }` is the standard "read a file line by line" loop shape you'll see three times in `file_io.cpp`.

## What to do with this list

You don't need to memorize any of this in the abstract. Keep this file open as a reference and come back to a section the moment you hit something unfamiliar while reading [[05-Walkthrough-Parser]] through [[08-Walkthrough-Main]]. Understanding *why* a construct exists is far more durable than memorizing its syntax.

Next: [[03-Editor-Ghost-Text-Field-Guide]]

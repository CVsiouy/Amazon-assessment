---
tags: [amazon-assessment, cpp, walkthrough, main]
---

# Walkthrough: `main.cpp`

This is the smallest file in the project and the only one that isn't inside `namespace returns` — it's the entry point, the thing that actually runs when you execute the compiled program. Its whole job is: read command-line arguments, call the pieces from the other three files in the right order, handle errors gracefully.

## Command-line argument parsing

```cpp
int main(int argc, char** argv) {
    std::string category_rules_path;
    std::string scoring_rules_path;
    std::string decision_bands_path;
    std::string profiles_path;
    std::string links_path;
    std::string returns_path;
    std::string out_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--category-rules" && i + 1 < argc) {
            category_rules_path = argv[++i];
        }
        else if (arg == "--scoring-rules" && i + 1 < argc) {
            scoring_rules_path = argv[++i];
        }
        // ...and so on for --decision-bands, --profiles, --links, --returns, --out
    }
    // ...
}
```

`argc` is "how many command-line arguments were given" and `argv` is those arguments as an array of C-strings, with `argv[0]` always being the program's own name — which is why the loop starts at `i = 1`, skipping it. For every argument, check if it matches one of the known flag names (e.g. `--category-rules`) *and* there's at least one more argument after it (`i + 1 < argc`, so we don't read past the end); if so, `++i` first (move to the next argument, which is the flag's *value*), then read `argv[++i]` into the matching path variable. This is a small, manual version of what libraries like `argparse` (Python) or `getopt` (C) do for you — walk the array once, recognize flags, consume the value that follows each one.

After the loop, it verifies every single path was actually provided:

```cpp
if (category_rules_path.empty() || scoring_rules_path.empty() || /* ...all seven... */ ) {
    std::cerr << "All arguments are required: --category-rules --scoring-rules "
                 "--decision-bands --profiles --links --returns --out\n";
    return 2;
}
```

Returning a non-zero value from `main` is how a program tells the operating system (or whatever script launched it, like `run.sh`) "something went wrong" — by convention, `0` means success, and any other number is an error code the caller can check.

> [!bug] A typo that breaks the `--scoring-rules` flag specifically
> Your typed file assigns to `sscoring_rules_path` (double `s`) instead of the `scoring_rules_path` declared at the top of `main`. Since that variable is never declared anywhere, this either fails to compile outright, or — if you "fix" it by declaring a second variable instead of correcting the typo — `scoring_rules_path` would stay permanently empty, and the program would always exit with "All arguments are required," even when you did pass `--scoring-rules`. Fix the spelling; see [[12-Pre-Compile-Cleanup-Checklist]].

## The actual pipeline call

```cpp
try {
    auto category_rules = returns::read_category_rules(category_rules_path);
    auto scoring_rules   = returns::read_scoring_rules(scoring_rules_path);
    auto decision_bands  = returns::read_decision_bands(decision_bands_path);
    auto profiles         = returns::read_account_profiles(profiles_path);
    auto links            = returns::read_account_links(links_path);
    auto requests         = returns::read_return_requests(returns_path);

    auto engine = returns::Engine::build(category_rules, scoring_rules, decision_bands, profiles, links);

    auto results = engine.evaluate(requests);
    returns::write_results(out_path, results);
    return 0;
}
catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    return 1;
}
```

This is genuinely the entire program, at the highest possible zoom level: read six inputs (three configs, two account files, one requests file), build an `Engine` from them, run `evaluate()` once to get all the results, write them out. Every step that's allowed to fail (a missing file, malformed JSON, a thrown exception from anywhere deeper in the call stack) is caught by the single `catch` block, which prints a message to `stderr` (the error output stream, separate from normal program output) and exits with code `1`.

This is also why Bug 8 (from [[06-Walkthrough-FileIO]] and detailed in [[10-The-8-Bugs]]) is so disruptive: `.value()` on an empty `std::optional` throws `std::bad_optional_access`, which *is* a `std::exception`, so it *would* get caught here — but only after `write_results` has already been running mid-loop. Depending on exactly when the throw happens, you can end up with a half-written, truncated output file and a generic error message, rather than either a clean success or a clean, obviously-wrong failure.

> [!bug] Ghost text and a missing `=`
> Nearly every `auto x = ...` line above shows up in your typed file with a `: Type` inserted right after the variable name — pure inlay-hint ghost text, covered in [[03-Editor-Ghost-Text-Field-Guide]] Pattern B. One line additionally lost its `=` sign entirely while being hand-copied: `auto results: std::vector<RoutingResult> engine.evaluate(requests);` needs to become `auto results = engine.evaluate(requests);` — note there are *two* separate problems stacked on that one line (the ghost-text type annotation, and the missing equals sign), which is a good reminder to re-read a "fixed" line fully rather than assuming one pass caught everything.

## What to take away from this file

- `main.cpp` is pure plumbing: parse arguments, call the other three files in sequence, handle errors. There's no business logic here at all — if you're hunting for one of "the 8 bugs" and you're looking inside `main.cpp`, you're very likely in the wrong place (the closest thing to a real issue here is the `--scoring-rules` typo, which is a transcription artifact, not a spec violation).
- The `try`/`catch` wrapping the whole pipeline is why this program fails "loudly but vaguely" — a good exercise once things compile is to run it on purpose with a bad path or a bugged `.value()` call and read exactly what message comes out on `stderr`.

Next: [[09-End-to-End-Trace]]

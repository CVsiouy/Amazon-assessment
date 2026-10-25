---
tags: [amazon-assessment, cpp, walkthrough, file-io]
---

# Walkthrough: `file_io.cpp`

If `parser.cpp` turns "text already in memory" into structs, `file_io.cpp` is the layer above it: it touches the actual disk, reads the raw bytes of each file into a string, hands that string to `parser.cpp`, and — in the other direction — takes the program's final answers and writes them back out to disk as JSON text. This is the only file in the project that opens `std::ifstream`/`std::ofstream` at all.

## The three small private helpers

These live inside an anonymous namespace (private to this file — see [[02-Cpp-Concepts-You-Need]] section 13).

**`trim(value)`** — strips leading and trailing whitespace from a string, the ordinary way: walk a `start` index forward while it's sitting on whitespace, walk an `end` index backward while it's sitting on whitespace, then return the substring between them. Used because a `.jsonl` file might have blank lines or trailing spaces the parser shouldn't choke on.

**`ensure_parent_dir(path)`** — before writing the output file, make sure the folder it lives in actually exists (e.g. if `path` is `data/results.jsonl` and `data/` doesn't exist yet, create it). Uses `std::filesystem::create_directories`, which is a standard-library function that creates a whole chain of missing folders at once, and won't error if the folder already exists.

**`read_file_content(path)`** — opens a file, and reads the *entire* thing into one `std::string` in a single shot:

```cpp
std::ifstream file(path);
if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + path);
}
std::ostringstream ss;
ss << file.rdbuf();       // dump the whole file's buffer into the string stream at once
return ss.str();
```

This is the standard C++ idiom for "give me the whole file as one string" without manually looping line by line. It's used for the three `.json` config files, which are read as one big blob and handed to `parser.cpp`'s document-level functions (they need the *whole* document at once, since it's one JSON object spanning many lines).

## Why the `.jsonl` files are read differently

`.jsonl` ("JSON Lines") files are a different format on purpose: instead of one big JSON document, it's *one independent JSON object per line*. That's why `read_account_profiles`, `read_account_links`, and `read_return_requests` don't use `read_file_content` at all — they open the file directly and loop `std::getline`:

```cpp
std::vector<AccountProfile> read_account_profiles(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open account profiles file: " + path);
    }

    std::vector<AccountProfile> profiles;
    std::string line;
    while (std::getline(file, line)) {
        std::string stripped = trim(line);
        if (stripped.empty()) continue;             // skip blank lines
        profiles.push_back(parse_account_profile_line(stripped));
    }
    return profiles;
}
```

Read the loop as: *"Keep asking for the next line. For each one, trim it; if there's nothing left, skip it (don't try to parse an empty line as JSON); otherwise hand it to the parser and remember the result."* `read_account_links` and `read_return_requests` are the same shape, just calling `parse_account_link_line`/`parse_return_request_line` instead.

> [!bug] A real (if small) bug hiding in plain sight
> Your typed `read_account_profiles` calls `std::getLine` (capital `L`) while the other two nearly-identical functions correctly call `std::getline` (lowercase). C++ is case-sensitive and there is no `std::getLine` — this is either a transcription slip on one of three copy-pasted blocks, or the kind of intentionally-placed "spot the odd one out" typo these assessments like to use. Either way, fix it to lowercase. It's a compile error, not a logic bug, so it doesn't count as one of "the 8," but it's exactly the kind of thing worth training your eye to catch by comparing near-duplicate functions against each other — see [[03-Editor-Ghost-Text-Field-Guide]]'s closing note and the full list in [[12-Pre-Compile-Cleanup-Checklist]].

## The three `read_*_json` config wrappers

```cpp
std::vector<CategoryRule> read_category_rules(const std::string& path) {
    std::string content = read_file_content(path);
    return parse_category_rules_json(content);
}
```
...and two siblings for scoring rules and decision bands. Each is a two-line "glue" function: load the bytes, hand them to the matching parser entry point from [[05-Walkthrough-Parser]]. All the real logic lives on one side or the other; these three functions just connect the pipes.

## `json_escape` — turning a raw string into safe JSON text

JSON strings can't contain certain characters literally — a `"` inside a JSON string would end the string early, a raw newline would break line-based tools, and so on. `json_escape` is supposed to walk every character of a string and, for the handful of characters JSON requires special handling for, emit the correct two-character escape sequence instead of the raw character:

```cpp
switch (c) {
    case '"':  out << "\\\""; break;   // " becomes \"
    case '\\': out << "\\\\"; break;   // \ becomes \\
    case '\b': out << "\\b";  break;   // backspace becomes \b
    case '\f': out << "\\f";  break;   // form feed becomes \f
    case '\n': out << "\\n";  break;   // newline becomes \n
    case '\r': out << "\\r";  break;   // carriage return becomes \r
    case '\t': out << "\\t";  break;   // tab becomes \t
    default:
        if (static_cast<unsigned char>(c) < 0x20) {
            // any other unprintable control character -> \u00XX
            out << "\\u" << std::hex << std::setfill('0') << std::setw(4)
                << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
        } else {
            out << c;   // ordinary printable character, pass through unchanged
        }
}
```

That's the *correct* version. Go look at your typed file, and you'll find all six of the non-`"` cases printing `"\\\""` — i.e. every single one of `\`, backspace, form feed, newline, carriage return, and tab currently gets escaped as if it were a quote character. This is Bug 7 (full detail, and why it's dangerous specifically *because* it won't show up when you just eyeball the sample output, in [[10-The-8-Bugs]]).

The shape of this bug is worth noticing on its own: it has all the fingerprints of someone copy-pasting one working `case` line six times and forgetting to edit the payload on five of the copies. That's an extremely common real-world bug pattern — whenever you see several `switch`/`if` branches that look suspiciously identical, check whether they were *supposed* to differ.

## `write_results` — turning the program's answers back into JSONL

```cpp
void write_results(const std::string& path, const std::vector<RoutingResult>& results) {
    ensure_parent_dir(path);
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open output file: " + path);
    }

    for (const auto& result : results) {
        std::ostringstream line;
        line << '{';
        line << "\"request_id\":" << json_escape(result.request_id) << ',';

        if (result.risk_score.has_value()) {
            line << "\"risk_score\":" << result.risk_score.value() << ',';
        } else {
            line << "\"risk_score\":null,";
        }

        line << "\"decision\":" << json_escape(result.decision);

        if (!result.reason.empty()) {
            line << ",\"reason\":" << json_escape(result.reason);
        }

        line << '}';
        file << line.str() << '\n';
    }
}
```

That's the corrected version — read it as: *"open the output file (creating its folder if needed); for each result, build one line of JSON by hand, field by field, using commas only where another field follows; write that line, then a newline, and move to the next result."* Notice `risk_score` is the one field that's genuinely conditional in two ways — present-as-a-number vs `null` (via `.has_value()`), and `reason` is conditional on non-empty. `request_id` and `decision` are always present, unconditionally.

> [!bug] What your typed file actually does instead
> Line one of the loop body in your file is `line << "\"request_id\":" << result.risk_score.value() << ',';` — it writes the **risk score**, unescaped and unquoted (so `55` instead of `"r101"`), under the `"request_id"` key, and it calls `.value()` directly with no `.has_value()` check. This is Bug 8. It means: (1) the output's `request_id` field is wrong for every single line, and (2) for every request that gets rejected in Step 1 (where `risk_score` is genuinely `std::nullopt`), calling `.value()` on an empty `optional` **throws an exception** (see [[02-Cpp-Concepts-You-Need]] section 9) — which, per `main.cpp`'s `try`/`catch`, would print an error to `stderr` and exit with status 1 **before writing anything at all** for that run. Full detail in [[10-The-8-Bugs]].

## What to take away from this file

- Two different reading strategies for two different file shapes: whole-document JSON gets slurped as one string; JSON Lines gets read one line at a time.
- `json_escape` and `write_results` are both "assemble a JSON string by hand with `<<`" code — no library does the formatting for you, so every character and every comma is something a human (or, in this case, a hand-transcriber) had to get exactly right.
- Both of this file's two real bugs (7 and 8) are "looks fine at a glance, wrong in a way that specific inputs would expose" bugs — neither would necessarily crash on *every* input, which is exactly why reading the code carefully matters more than just running it once on sample data and eyeballing the result.

Next: [[07-Walkthrough-Engine]]

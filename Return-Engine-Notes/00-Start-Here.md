---
tags: [amazon-assessment, cpp, index]
---

# Start Here

This vault explains one codebase — a small C++ "return routing engine" — from the ground up, on the assumption that you don't have a strong coding background yet. It also teaches you the general skill of using an AI assistant to find bugs fast, using this exact assessment as the worked example.

## What actually happened here (read this first)

You did an Amazon OA. One part was a LeetCode-style problem. The other was an "AI round": you were handed a C++ codebase on screen and asked to use AI to find bugs in it — you later found out by searching online that there are **exactly 8 bugs** in it. You didn't have time to understand the codebase in the moment, so you photographed the screen (there are 150+ photos in [photos/](../photos/)) and, afterward, hand-typed everything back out from the photos into this project folder, planning to study it properly later. This vault is that study session.

## The single most important discovery, before you read anything else

> [!important] Your code has "ghost text" baked into it
> Modern C++ editors (CLion, VS Code with the C++ extension, Visual Studio) draw **inlay hints** — small grey annotations showing a variable's inferred type, or a function parameter's name — directly in the editor, overlaid on top of your actual code. They are not part of the file. They exist only on screen, to help the person reading the code.
>
> When you photographed the screen and then typed the photos back out by hand, you couldn't tell the difference between "real code" and "grey helper text the editor drew for you." So you typed both. That is why lines like this exist in your files:
>
> ```cpp
> for (const auto &rule : ScoringRule const & : scoring_rules_)
> ```
>
> That is not valid C++, and it never was. The real line is just:
>
> ```cpp
> for (const auto &rule : scoring_rules_)
> ```
>
> `ScoringRule const &` was grey ghost text the editor was showing you as a courtesy — the deduced type of `rule` — and your hand copied it as if it were code sitting between two colons.
>
> This single realization explains the majority of the "this looks impossibly complex" feeling. It isn't complex. A large fraction of what you're looking at isn't real syntax at all. Once you learn to recognize the pattern (fully covered in [[03-Editor-Ghost-Text-Field-Guide]]), the actual code underneath is short and fairly ordinary.

This is genuinely good news: it means your task isn't "learn to read alien code," it's "learn to spot and strip a handful of repeating patterns," which is a much smaller and more mechanical skill.

## The current state of your project folder

I read every file in your project before writing these notes. Here's the honest status:

| File | Status |
|---|---|
| `src/returns/engine.cpp` | Typed out, full of ghost-text noise, several real logic bugs |
| `src/returns/file_io.cpp` | Typed out, same situation |
| `src/returns/main.cpp` | Typed out, same situation |
| `src/returns/parser.cpp` | Typed out, same situation |
| `include/returns/engine.hpp` | **Empty — 0 bytes** |
| `include/returns/models.hpp` | **Empty — 0 bytes** |
| `include/returns/parser.hpp` | **Empty — 0 bytes** |
| `include/returns/file_io.hpp` | **Empty — 0 bytes** |
| `tests/engine_test.cpp` | **Empty — 0 bytes** |
| `README.md` | Typed out, readable, describes the spec |
| `data/*.json`, `data/*.jsonl` | Typed out, look complete and internally consistent |
| `results.jsonl` (project root) | Typed out — turns out to be extremely useful, see [[10-The-8-Bugs]] |

The four header files (`.hpp`) define every data structure the program uses — `ReturnRequest`, `RoutingResult`, `CategoryRule`, and so on. Right now they're empty, which means **as it stands, this project cannot compile.** That's not a bug to fix in the logic sense — it's unfinished transcription.

The good news: headers in a project like this mostly just *declare* things that the `.cpp` files already *define* and *use*. I was able to reconstruct all four with high confidence purely by reading how every field and function is used across the four source files you did type out. [[04-Data-Models-Reconstructed]] has the full reconstruction, clearly marked as "inferred — verify against your photos," along with the reasoning technique so you can learn to do this yourself.

## How this vault is organized

Read in this order the first time; after that, treat it as a reference you jump around in.

1. [[01-Big-Picture-The-Business-Problem]] — what this program is *for*, in plain English
2. [[02-Cpp-Concepts-You-Need]] — the specific C++ features this codebase uses, explained from zero
3. [[03-Editor-Ghost-Text-Field-Guide]] — the full catalogue of "ghost text" patterns and how to strip them
4. [[04-Data-Models-Reconstructed]] — the missing header files, rebuilt from usage
5. [[05-Walkthrough-Parser]] — `parser.cpp` explained function by function
6. [[06-Walkthrough-FileIO]] — `file_io.cpp` explained function by function
7. [[07-Walkthrough-Engine]] — `engine.cpp`, the actual business logic, explained function by function
8. [[08-Walkthrough-Main]] — `main.cpp`, the entry point, explained
9. [[09-End-to-End-Trace]] — one request, traced by hand through the whole pipeline, with real numbers
10. [[10-The-8-Bugs]] — the 8 genuine logic bugs, each with proof, not guesswork
11. [[11-Debugging-With-AI-Playbook]] — how to make an AI assistant find bugs like these in minutes, with copy-pasteable prompts
12. [[12-Pre-Compile-Cleanup-Checklist]] — the mechanical, non-logic fixes needed just to get this to build

## If you only remember three things

> [!tip] The three-sentence version
> 1. Most of the "weirdness" in your typed files is your editor's on-screen type/parameter hints, hand-copied as if they were code — strip them using the patterns in [[03-Editor-Ghost-Text-Field-Guide]], and the file underneath is short and normal.
> 2. The program is a 3-step pipeline — check the category is returnable and in-window, add up return risk from linked accounts into a score, map the score to a decision — and all 8 real bugs live in one of those three steps or in the parsing/writing code around them.
> 3. The fastest way to find bugs like these with AI is to hand it the spec (the README) and the code *together* and ask it to check each spec requirement against the implementation one line at a time — not to just say "find bugs." [[11-Debugging-With-AI-Playbook]] gives you the exact prompts.

Next: [[01-Big-Picture-The-Business-Problem]]

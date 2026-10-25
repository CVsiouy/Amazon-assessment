---
tags: [amazon-assessment, ai-assisted-debugging, playbook]
---

# How To Use AI To Find Bugs Like These, Fast

This note is the "next time" playbook. Everything below is written so you can reuse it in a live, timed round — copy the prompts nearly verbatim, adjust file names.

## The mistake to avoid: asking "find the bugs"

Just pasting a file and asking an AI assistant "find the bugs in this code" works sometimes, but it's the slowest and least reliable way to do it, for a specific reason: the assistant has no independent way to know what the code is *supposed* to do, so it can only flag things that look locally suspicious (a missing null check, an odd-looking loop) rather than things that are provably wrong against a spec. In an assessment like this one, you were handed a spec (the README) *and* the code together — the fastest path is to make the AI cross-reference them explicitly, the same way you'd grade a student's work against an answer key, rather than just reading the essay and guessing what was assigned.

Everything below builds on that one idea.

## Step 0 (before anything else): get it compiling

You cannot find logic bugs in code that doesn't parse — a compiler error thrown by a stray typo will stop you from ever reaching the interesting bug. In a live round, spend the first few minutes purely on this, and say so explicitly to the AI:

> [!tip] Prompt: get to a compile
> "Here is a C++ project [paste all files, or point at the directory]. It currently fails to compile. Go through it and list every syntax error and undefined-reference error you can find just from reading it — don't look for logic bugs yet, only things that would stop it from compiling or linking. For each one, give me the file, the line, what's wrong, and the one-line fix."

If you have a terminal and can actually run a compiler, this is even faster and more reliable than asking the AI to "mentally compile" — real compiler errors are ground truth:

> [!tip] Prompt: fix from real compiler output
> "Run `cmake --build build` (or whatever this project's build command is) and show me the output. Then fix every error it reports, one at a time, re-running after each fix, until it builds cleanly."

An AI assistant that can actually execute shell commands (like this one, via its terminal/bash tool) should be doing this literally, not guessing — always prefer "run the real compiler and read its real error" over "read the code and imagine what the compiler would say," when the tool supports it.

## Step 1: build a spec-compliance table

This is the single highest-leverage prompt for an assessment shaped like this one — a README full of numbered rules, plus code that's supposed to implement them.

> [!tip] Prompt: spec-vs-code compliance table
> "Here is the spec (README) and here is the implementation (all source files). Go through the spec requirement by requirement — every bullet point, every numbered rule, every example — and for each one, tell me: (1) which function is supposed to implement it, (2) quote the exact lines that do (or should) implement it, (3) whether the code actually satisfies that requirement or not, and (4) if not, exactly what's missing or wrong. Output this as a table. Do not skip any requirement, including ones that seem too simple to get wrong."

That last sentence matters — the easy-looking requirements (like "clamp the total to 0-100") are exactly the ones that get skipped both by careless developers and by an AI assistant that's pattern-matching for "interesting-looking" bugs instead of methodically checking every line of the spec. This one prompt, run against this project, would have surfaced most of the 8 bugs in [[10-The-8-Bugs]] directly, because nearly every one of them is a spec sentence with no corresponding code, or code that only does half of what a spec sentence asked for.

## Step 2: trace real examples by hand, and demand the AI show its arithmetic

If the README includes worked examples (this one has four), or if you have any sample input/output pair (this project's `results.jsonl` turned out to be exactly that), use them as ground truth.

> [!tip] Prompt: trace and diff
> "Take this input request: [paste one line from returns.jsonl]. Compute, step by step, showing every intermediate number, what the *correct* output should be according to the README spec — not by reading the code, purely from the spec's rules. Then separately trace what the *actual code* would compute for the same input, again showing every intermediate value. Tell me exactly where the two traces diverge, and which line of code caused the divergence."

The reason to insist on "step by step, showing every intermediate number" is that it forces the model to commit to concrete values you can sanity-check yourself, rather than jumping straight to a conclusion you have to take on faith. If you have real observed output (like this project's `results.jsonl`), tell it that too:

> [!tip] Prompt: use real output as an oracle
> "Here is the program's actual observed output for these 40 requests [paste results.jsonl] and here is the input data [paste returns.jsonl, account_profiles.jsonl, account_links.jsonl]. For at least 5 requests, compute the spec-correct answer by hand and compare it to the observed one. Find the smallest set of bugs in the code that would explain *all* the discrepancies you see, not just one of them. Show your reasoning for how you isolated each bug — don't just assert a conclusion."

This is precisely the technique used throughout [[09-End-to-End-Trace]] and [[10-The-8-Bugs]] — every bug in this vault was confirmed, not guessed, by exactly this kind of arithmetic. It generalizes to any assessment where you're given (or can produce) even a small amount of real input/output.

## Step 3: ask for specific, named bug *shapes*, not just "bugs"

Certain bug shapes recur constantly in planted-bug exercises like this one, because they're realistic mistakes real developers make. Naming them explicitly gets much more targeted results than a generic "find bugs" prompt:

> [!tip] Prompt: hunt by pattern
> "Look specifically for these patterns across the whole codebase, and tell me every place you find one, even if you're not fully sure it's a bug: (1) a boolean/flag that's supposed to flip to true under some condition but is hard-coded to false, or vice versa; (2) blocks of near-identical code (loops, switch/case branches, parser functions) where one copy differs from its siblings in a way that looks unintentional; (3) a value that's read from input and then thrown away (e.g. parsed and discarded, or assigned to the wrong variable) instead of being stored where it's used later; (4) a doc comment or function name that promises behavior the function body doesn't actually perform; (5) a numeric result that's documented to be bounded (e.g. 'clamp to 0-100') but the code never enforces that bound; (6) two names that are almost identical but not quite (a function defined under one name and called under a slightly different one)."

Every one of the 8 bugs in [[10-The-8-Bugs]] is an instance of one of these six shapes. Memorize this list — it transfers to almost any "find the planted bugs" exercise, not just this one.

## Step 4: don't stop at the business logic — check the plumbing too

It's tempting to focus all your attention on the file with the "interesting" logic (here, `engine.cpp`) and skim the file-reading/writing/parsing code as boring plumbing. Two of the eight bugs in this project (7 and 8) live in `file_io.cpp`, and two more (1 and 2) live in `parser.cpp` — half the bugs are in code that never makes a "business decision" at all.

> [!tip] Prompt: don't skip the boring files
> "For every function in file_io.cpp and parser.cpp, tell me in one sentence what invariant it's supposed to preserve (e.g. 'every field present in the input JSON ends up in the output struct', 'every special character in a string is escaped correctly for JSON'), and then check whether the code actually preserves that invariant for every code path, not just the common one."

## Step 5: make the AI adversarially double-check its own findings

AI assistants can produce confident-sounding false positives, especially on code with as much surface-level noise as your hand-transcribed files had (see [[03-Editor-Ghost-Text-Field-Guide]]) — it's easy for a model to flag a ghost-text artifact as "a bug" when it's actually just corrupted transcription, or to miss a real bug because a typo nearby is distracting. Always ask for a second pass that tries to break the first pass's conclusions:

> [!tip] Prompt: adversarial re-check
> "For each bug you just reported, argue against yourself: is there any way the current code is actually correct, or any assumption you made that might be wrong? Only keep the ones that survive this challenge. For each one that survives, state your confidence and what evidence (spec quote, data example, or observed-output example) backs it up."

This is also exactly why every bug in [[10-The-8-Bugs]] is written with a "proof from real data" section rather than just an assertion — a claim with a concrete, checkable example attached is far more trustworthy (to you, to a grader, and to yourself under time pressure) than "this looks wrong."

## Step 6: verify the fix, not just the diagnosis

Finding a bug is half the job; an assessment grading your fixes cares about the corrected behavior. After applying a fix, re-run the same trace-and-diff technique from Step 2 on the same example to confirm the gap actually closed, and check that you didn't only fix the symptom you first noticed while leaving the underlying cause (or a sibling copy-pasted block with the same mistake) untouched.

## Time-boxing this in a live, timed round

Given what actually happened to you, here's a concrete order of operations for next time, assuming roughly 45-60 minutes and a codebase of this size:

1. **~5 min:** Get it compiling (Step 0). Don't read anything deeply yet.
2. **~10 min:** Run the spec-compliance table prompt (Step 1) against the whole codebase at once. This alone typically finds most of the bugs in an exercise shaped like this one.
3. **~10 min:** If sample input/output is available, run the trace-and-diff prompt (Step 2) to confirm or refine the Step 1 findings with concrete numbers.
4. **~10 min:** Run the pattern-hunt prompt (Step 3) as a sweep for anything Steps 1-2 missed, especially in "boring" files (Step 4).
5. **~10 min:** Adversarially re-check (Step 5), fix each confirmed bug, and re-verify (Step 6).
6. **Remaining time:** write up what you found and why, citing file/line/spec-quote for each — a clear, evidenced writeup is often worth as much as finding one extra bug.

## Why this matters beyond this one assessment

The core skill being tested in an "AI round" like this isn't "can you personally spot a bug by eye" — it's "can you direct an AI assistant efficiently toward provably correct answers, under time pressure, on code you don't already know." The concrete moves that make that fast are: hand over the spec and the code together, insist on arithmetic you can check rather than assertions, name the bug shapes you're hunting for instead of asking generically, don't neglect boring-looking files, and adversarially double-check anything before you report it as a finding. That's a transferable skill independent of this specific return-routing engine.

Next: [[12-Pre-Compile-Cleanup-Checklist]]

# Qutepart highlighting benchmark

How the syntax-highlighting engine was measured before and after optimization, what the
numbers came out to, and how it now stands against KDE's KSyntaxHighlighting — both on
qutepart's grammars and on the upstream ones the official library actually ships.

| | |
|---|---|
| Machine | Intel i5-10210U, 4C/8T, Debian testing |
| Qt | 6.10.2 |
| Baseline | `540e993` |
| Optimized | `47e11db`, `226bb3e` (branch `faster-highlighting`) |
| Reference | KSyntaxHighlighting 6.28.1 |

**Headline:** opening a 944 KB file in codepointer went from **2125 ms to 842 ms** (2.5×).
Best case is Rust at 13.8×. Highlighting output is byte-identical across 247 files in about
120 languages.

---

## 1. What was measured

The engine parses a document one line at a time, carrying state forward. An editor wraps that
in `QSyntaxHighlighter`, which adds its own per-character format bookkeeping and pushes results
into a `QTextDocument`. The application wraps *that* in a file-open path with I/O, decoding and
widget layout. A speedup in the parser is diluted at each layer, so all three were measured
separately:

- **Parser only** — a loop calling `Language::highlightBlock()` over every block.
- **Editor path** — `QSyntaxHighlighter::rehighlight()` over a document with a live widget attached.
- **End to end** — codepointer opening the file, timed inside `qmdiEditor::loadContent()`.

The primary workload is a single 944 KB, 49,806-line C++ file of deliberately awkward code:
preprocessor edge cases, template metaprogramming, macros spanning lines. Fourteen other files
from `code-examples/` cover the language spread; each is concatenated with itself until it
reaches roughly 20,000 lines so per-file fixed costs stay negligible.

---

## 2. Methodology

Timing on a loaded laptop is worthless. Most of the effort went into making the numbers
reproducible rather than into producing them.

### Controlling the machine

Every batch is gated: it does not start until system-wide CPU busy time, sampled from
`/proc/stat` over a three-second window, falls below 8%. Runs are pinned to one logical core
with `taskset`, because this is a 4-core/8-thread part and sibling threads share execution
resources — the same binary measured 2.45× on cpu0 and 2.01× on cpu5 in one session. Core
frequency was confirmed at 3.8 GHz during measurement against a 4.2 GHz maximum, so results are
not clipped by a cold clock.

### Interleaving, not batching

Baseline and optimized binaries alternate within a round rather than running as two blocks.
Machine conditions drift over minutes; interleaving spreads that drift across both sides instead
of loading it onto whichever ran second. Each round is itself the best of three in-process
passes, and each figure is reported as the minimum and median across rounds. **When those two
agree to within a percent or two, the batch is clean** — that is the acceptance criterion used
throughout.

### Proving the output did not change

A dump mode emits, for every block of every file in `code-examples/`, each format range with a
colour-and-weight digest, the per-character text-type map, and the folding level. Running it
against both builds and diffing gives a 247-file, ~120-language regression net. Every result
below was gated on that diff coming back clean.

One wrinkle: block *state* values are hashes of context pointers, so they differ between runs of
the same binary. The dump relabels them by order of first appearance, which keeps the sequence
reproducible while still detecting a genuine change in where states begin and end.

---

## 3. Measurement errors found and corrected

Each of these produced confident, wrong numbers before it was caught. They are listed because
they are easy to repeat.

**The harness never highlighted anything.** A `QTextDocument` with no document layout attached
does not run `QSyntaxHighlighter` at all. The first harness reported that attaching a highlighter
cost nothing — because zero of 49,806 blocks were highlighted. Caught by counting blocks carrying
highlight data afterwards; fixed by attaching a real `QPlainTextEdit`.

**Background load swung results 2×.** The same binary measured 222 ms and 860 ms minutes apart. A
project was loaded in the editor under test and its scanner was saturating the CPU. Fixed by the
idle gate plus interleaving; min and median now agree to ~1%.

**The comparison ran a different language.** `definitionForFileName("or1200_du.v")` resolved to
KSyntaxHighlighting's **V** definition — the V programming language — not Verilog. That row read
0.51×; on the correct grammar it is 1.18×. Fixed by forcing definitions by name and asserting both
engines resolved the same file.

**The two engines shipped different grammars.** Both use Kate syntax files, but from different
snapshots: HTML v5 against v20, Ruby v8 against v21, Lua v4 against v21. Per-language ratios were
partly measuring years of grammar evolution, not engine speed. Fixed with
`Repository::addCustomSearchPath()`, pointing KSyntaxHighlighting at qutepart's own XML files.

**`addCustomSearchPath()` did not do what it looked like it did.** Feeding qutepart's `syntax/`
directory to a KSyntaxHighlighting `Repository` appeared to work, and a check on the definition's
file *basename* said "same grammar". It was not: the repository keeps whichever same-named
definition has the higher `version`, so qutepart's `html.xml` (v5) always lost to the bundled v20,
and the comparison was silently running KDE's own newer grammars on both sides. Caught by printing
the full path — `:/org.kde.syntax-highlighting/syntax/html.xml`. Fixed by copying all 385
definitions with `version="9999"` so they win, and asserting on the full path prefix. The
correction is large and moves in qutepart's favour: HTML went 0.49× → 0.77×, C++ 1.21× → 1.40×,
Lua 3.18× → 3.99×.

**A "1100 ms" that excluded the highlighting.** The application's own load timer stopped just
before `updateFileDetails()`, which is where the highlighter gets attached — so on a first open
the reported figure omitted highlighting entirely. Timer split into read / document+highlight /
language phases and moved past the attach point.

---

## 4. Results: before and after

Same binary, same file, same conditions; only the qutepart source differs.

### Per-language, whole-document parse

> RelWithDebInfo · each file repeated to ~20,000 lines · parser only
> 5 interleaved rounds, best of 3 passes each · CPU pinned, gated idle

| File | Grammar | Before | After | Speedup |
|---|---|---:|---:|---:|
| test.rs | Rust | 1476.4 ms | 107.2 ms | **13.77×** |
| highlight.java | Java | 396.6 ms | 54.9 ms | 7.23× |
| test.py | Python | 432.1 ms | 63.0 ms | 6.86× |
| or1200_du.v | Verilog | 585.4 ms | 88.8 ms | 6.59× |
| highlight.rb | Ruby | 965.3 ms | 164.2 ms | 5.88× |
| highlight.js | JavaScript | 284.0 ms | 64.2 ms | 4.42× |
| highlight.cpp | C++ | 175.8 ms | 70.5 ms | 2.50× |
| highlight.css | CSS | 104.7 ms | 49.3 ms | 2.13× |
| test.htm | HTML | 288.2 ms | 148.0 ms | 1.95× |

HTML gains least: `html.xml` matches every non-space character with `RegExpr(\S)`, which no
first-character filter can skip.

### The primary workload, layer by layer

> benchmark.cpp · 944 KB · 49,806 lines · 7 interleaved rounds
> Reported as median; min and max within 2% of median throughout

| Layer | Build | Before | After | Speedup |
|---|---|---:|---:|---:|
| Parser only | RelWithDebInfo | 437 ms | 178 ms | 2.46× |
| Parser only | Debug (`-g`) | 1867 ms | 615 ms | 3.04× |
| Editor path | RelWithDebInfo | 499 ms | 219 ms | 2.27× |
| Editor path | Debug (`-g`) | 1982 ms | 663 ms | 2.99× |
| File open, end to end | Debug (`-g`) | 2125 ms | 842 ms | **2.52×** |

The last row is codepointer itself, timed inside `loadContent()`, minimum of four opens. It
agrees with the isolated harness to within 1% — the cross-check that the two measurement
approaches are consistent.

> **The Debug rows are not academic.** `codepointer/cbuild` is configured
> `CMAKE_BUILD_TYPE=Debug`, and neither project sets a default build type, so an unspecified
> configure compiles the highlighter with no optimization at all. Debug against RelWithDebInfo is
> worth roughly 3.2× on the parser by itself — more than any single change described here.

---

## 5. Results: against KSyntaxHighlighting

KDE's own implementation of the same Kate syntax definitions, measured two ways:

- **KSH · qutepart grammar** — KSyntaxHighlighting fed qutepart's own XML files (copied with
  `version="9999"` so its repository does not discard them in favour of its newer bundled ones).
  Identical input to both engines, so this column isolates the **engine**.
- **KSH · upstream grammar** — KSyntaxHighlighting exactly as shipped, which is what you would
  get from the official library. This column shows engine **and** grammar together.

Ratios are **KSyntaxHighlighting time ÷ qutepart time**: above 1.00 means qutepart is faster.
`ver` is qutepart's grammar version against upstream's.

### Editor path — both driven by `QSyntaxHighlighter` over an identical `QTextDocument`

> RelWithDebInfo · best of 5 · full-path assertion that KSH loaded the intended grammar

| File | qutepart | KSH · qutepart grammar | | KSH · upstream grammar | | ver |
|---|---:|---:|---:|---:|---:|---|
| highlight.lua | 28.1 ms | 112.1 ms | **3.99×** | 91.9 ms | 3.27× | 4→21 |
| test.py | 83.1 ms | 156.2 ms | 1.88× | 80.5 ms | 0.97× | 6→33 |
| highlight.php | 81.8 ms | 132.7 ms | 1.62× | 99.7 ms | 1.22× | 5→20 |
| highlight.js | 84.6 ms | 133.8 ms | 1.58× | 125.1 ms | 1.48× | 8→26 |
| test.markdown | 54.3 ms | 85.5 ms | 1.57× | 96.0 ms | 1.77× | 3→33 |
| highlight.java | 74.1 ms | 112.8 ms | 1.52× | 69.5 ms | 0.94× | 4→13 |
| highlight.rb | 190.2 ms | 282.8 ms | 1.49× | 108.0 ms | 0.57× | 8→21 |
| **benchmark.cpp** | **227.7 ms** | **322.7 ms** | **1.42×** | **269.9 ms** | **1.19×** | 13→20 |
| highlight.cpp | 92.8 ms | 129.9 ms | 1.40× | 108.3 ms | 1.17× | 13→20 |
| highlight.hs | 114.2 ms | 151.0 ms | 1.32× | 150.5 ms | 1.32× | 10→22 |
| or1200_du.v | 112.0 ms | 132.3 ms | 1.18× | 131.8 ms | 1.18× | 4→8 |
| highlight.css | 68.1 ms | 71.7 ms | 1.05× | 68.2 ms | 1.00× | 7→17 |
| highlight.xml | 51.1 ms | 52.8 ms | 1.03× | 55.3 ms | 1.08× | 7→27 |
| test.rs | 132.5 ms | 124.9 ms | 0.94× | 137.1 ms | 1.03× | 5→16 |
| test.htm | 177.8 ms | 136.1 ms | 0.77× | 87.9 ms | 0.49× | 5→20 |
| **total** | **1572 ms** | **2137 ms** | — | **1680 ms** | — | |
| **geometric mean** | | | **1.41×** | | **1.13×** | |
| **qutepart ahead on** | | | **13 / 15** | | **11 / 15** | |

### Engine only — each engine's own line-at-a-time API

> RelWithDebInfo · best of 5 · same grammar assertion

| File | qutepart | KSH · qutepart grammar | | KSH · upstream grammar | |
|---|---:|---:|---:|---:|---:|
| highlight.lua | 16.4 ms | 69.6 ms | **4.23×** | 52.5 ms | 3.19× |
| test.py | 62.3 ms | 109.3 ms | 1.75× | 38.7 ms | 0.62× |
| highlight.php | 59.2 ms | 88.9 ms | 1.50× | 56.6 ms | 0.96× |
| highlight.js | 63.7 ms | 90.8 ms | 1.43× | 83.9 ms | 1.32× |
| highlight.rb | 161.1 ms | 227.5 ms | 1.41× | 62.9 ms | 0.39× |
| test.markdown | 42.4 ms | 56.4 ms | 1.33× | 63.1 ms | 1.49× |
| highlight.java | 53.9 ms | 69.1 ms | 1.28× | 28.3 ms | 0.53× |
| highlight.cpp | 69.1 ms | 83.7 ms | 1.21× | 63.3 ms | 0.92× |
| benchmark.cpp | 173.1 ms | 205.8 ms | 1.19× | 156.1 ms | 0.90× |
| highlight.hs | 83.9 ms | 98.7 ms | 1.18× | 99.5 ms | 1.19× |
| or1200_du.v | 86.1 ms | 84.6 ms | 0.98× | 82.8 ms | 0.96× |
| test.rs | 104.3 ms | 78.1 ms | 0.75× | 84.3 ms | 0.81× |
| highlight.css | 48.8 ms | 34.6 ms | 0.71× | 27.9 ms | 0.57× |
| test.htm | 144.2 ms | 82.3 ms | 0.57× | 36.8 ms | 0.25× |
| highlight.xml | 35.4 ms | 19.9 ms | 0.56× | 20.7 ms | 0.59× |
| **total** | **1204 ms** | **1399 ms** | — | **957 ms** | — |
| **geometric mean** | | | **1.17×** | | **0.82×** | |
| **qutepart ahead on** | | | **10 / 15** | | **4 / 15** | |

### How much of the gap is the grammar, not the engine?

Comparing KSyntaxHighlighting against *itself* on the two grammar sets isolates the grammars,
since the engine is held constant:

| Scenario | Upstream grammars vs qutepart's, same engine |
|---|---:|
| Editor path | **1.24× faster** |
| Engine only | **1.43× faster** |

Individual languages are far more extreme. On the identical KSyntaxHighlighting engine, upstream's
grammars beat qutepart's bundled copies by **3.6×** on Ruby (227.5 → 62.9 ms), **2.8×** on Python
(109.3 → 38.7 ms), **2.4×** on Java (69.1 → 28.3 ms) and **2.2×** on HTML (82.3 → 36.8 ms).
qutepart's `syntax/` is years behind — Python v6 against v33, Markdown v3 against v33, HTML v5
against v20.

**This is the actionable finding.** Refreshing `syntax/` from upstream is likely worth more on
Python, Ruby, Java and HTML than anything left to squeeze out of the parser. It is also the reason
the two KSH columns disagree so much: the middle column measures the engine, the right column
measures the product.

### Why the two scenarios disagree

The native-API comparison is **not symmetric**. KSyntaxHighlighting's line API walks plain strings
and hands back a compact `State`. qutepart's needs a `QTextBlock`, and allocates a
`TextBlockUserData` per block holding a *per-character* text-type map and language map — the
bookkeeping behind `isComment(block, column)` and per-position language resolution in
mixed-language files. KSyntaxHighlighting has no equivalent, so that scenario charges qutepart for
features the other engine does not provide.

The editor-path comparison reflects how the code is actually used, and qutepart leads there —
while still building those maps. Part of the swing is on the other side of the ledger:
KSyntaxHighlighting's `applyFormat()` constructs a `QTextCharFormat` from its `Format` and `Theme`
on every call, where qutepart hands over a cached format pointer and merges adjacent equal ranges.

**Short version:** on identical grammars and the path an editor actually takes, qutepart's engine
is **1.41×** ahead of KDE's reference implementation. Against the official library as shipped —
newer grammars included — it is still **1.13×** ahead. Before this work it was roughly 2.5× behind.

---

## 6. What changed in the engine

Five changes, in rough order of contribution.

1. **First-character rule prefilter.** Every rule declares the set of characters it can begin a
   match at; contexts bucket rules by that character, so matching walks two or three candidates
   rather than all of them. Regular expressions get a FIRST-set analyzer that returns "unknown" —
   meaning the rule is always tried, exactly as before — for anything it cannot prove. Sets
   propagate through `IncludeRules`, which matters because C++'s top-level context is nothing but
   includes. 94.5% of the 8,537 rules across the bundled languages get a set.
2. **Dynamic regexps no longer recompiled per attempt.** A rule marked dynamic was rebuilding,
   compiling and JIT-compiling its pattern at every column it was tried at. This is most of Rust's
   13.8×.
3. **No per-match copying.** The match result held a style by value — two atomic reference counts
   per matched token — plus a context switcher and a capture list. It now points at the rule's own
   copies, and captures are extracted only when the target context can read them back.
4. **Context switches mutate in place** rather than returning a fresh stack.
5. **Per-block buffers are reused**, and the block text is passed down instead of re-fetched.

A separate fix, unrelated to speed but found along the way: `setHighlighter()` overwrote its
highlighter pointer without destroying the old one, leaving two highlighters attached to the same
document, both re-highlighting on every edit.

---

## 7. Limitations

- **One machine, one microarchitecture.** Everything here is a mobile Skylake-derived i5 with SMT.
  Ratios that depend on cache behaviour may move on other hardware.
- **Synthetic file sizes.** Repeating a file to reach 20,000 lines gives the branch predictor and
  caches a friendlier workload than 20,000 lines of genuinely varied code.
- **The version bump is a benchmarking hack.** Rewriting `version="9999"` in copies of the
  grammars changes only metadata KSyntaxHighlighting uses to pick between duplicates, not matching
  behaviour — but it is a modification, and the copies are not the files qutepart itself loads.
- **Grammar comparisons are not like-for-like in output.** Upstream's newer definitions do not just
  run faster, they highlight differently — more rules, different regions. The grammar columns
  compare cost, not correctness.
- **Correctness is verified, not proven.** The 247-file corpus is broad but finite; it does not
  exercise every rule of every grammar. The FIRST-set analyzer is written to fail towards "always
  try this rule", so an error should cost speed rather than correctness — but that is a design
  property, not a guarantee.
- **Block state values changed.** There are now fewer distinct states, because unused capture
  groups no longer enter them. Strictly better for incremental re-highlighting, but any code
  depending on specific state values would notice.

---

## 8. What is left

1. **Refresh `syntax/` from upstream.** The largest remaining win, and it needs no engine work —
   worth up to 3.6× on individual languages, 1.24–1.43× in aggregate.
2. **HTML and Ruby.** The two languages where qutepart's engine still loses on identical grammars.
   HTML is understood: `html.xml` uses `RegExpr(\S)`, which matches every non-space character, so
   the first-character prefilter can never skip it. Ruby is not yet explained.
3. **Structural, beyond that.** The parser profile is now flat — nothing above about 6% — so
   further micro-optimization is worth perhaps another 10–20%. At roughly 4.7 µs per line,
   highlighting a 50,000-line file is inherently a fifth of a second; highlighting the hundred
   lines actually on screen is half a millisecond. Reaching the latter means highlighting to the
   last visible block and continuing in idle slices — which `QSyntaxHighlighter` cannot do, since
   it insists on a full sequential pass. Even then, `QTextDocument` construction alone is about
   130 ms of the open, and that is Qt's, not ours.

---

*Baseline `540e993`; optimized `47e11db` + `226bb3e` on branch `faster-highlighting`. Comparison
against KSyntaxHighlighting 6.28.1 with qutepart's own `syntax/*.xml` loaded into its `Repository`.
All timings: Intel i5-10210U, single pinned core, system CPU under 8% busy at the start of each
batch.*

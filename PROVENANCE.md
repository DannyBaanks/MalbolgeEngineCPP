# PROVENANCE

Date of writing: 2026-09-03. This file states, truthfully, where each part
of this repository comes from. It is not retroactively editable marketing;
if a subsystem's origin changes, this table changes in the same commit.

Classification legend:

- `PORTED_FROM_REFERENCE` — behavior/logic taken from the reference C
  engine (DannyBaanks/Malbolge-Engine, MIT, same author), re-expressed as
  idiomatic C++.
- `NEW_IMPLEMENTATION` — code that has no counterpart in the reference.
- `GENERATED_ASSISTANCE` — drafted by an AI coding assistant and
  reviewed/tested by the repository owner. (Stated plainly: the entire
  C++ codebase in its current form was produced with AI assistance.)
- `THIRD_PARTY` — external code included or invoked.

| Path | Origin | Notes |
|---|---|---|
| `src/memory.cpp` | PORTED_FROM_REFERENCE | Line-for-line behavioral port of overlay + lazy crazy-chain fill, incl. documented quirks |
| `src/crazy.cpp` | PORTED_FROM_REFERENCE | Same truth table and split-half table construction |
| `src/vm.cpp` (dispatch loop) | PORTED_FROM_REFERENCE | Same instruction semantics and step accounting, restructured into `step()`/`run()` |
| `src/vm.cpp` (StopReason, load_source) | NEW_IMPLEMENTATION | Reference collapses stop causes; C++ exposes them |
| `src/trace.cpp`, `include/malbolge/trace.hpp` | NEW_IMPLEMENTATION | Reference has no tracing |
| `include/malbolge/*` | NEW_IMPLEMENTATION | API design is new |
| `cli/main.cpp` | NEW_IMPLEMENTATION | Reference CLI interface differs (subcommands, `--json`, trace) |
| `tests/*` | NEW_IMPLEMENTATION | Reference has no test suite beyond a smoke Makefile target |
| `fuzz/fuzz.cpp` | NEW_IMPLEMENTATION | Property harness, seeded PRNG, no external framework |
| `bench/bench.cpp` | NEW_IMPLEMENTATION | |
| `bench/reference_bench.c` | NEW_IMPLEMENTATION (driver) around THIRD_PARTY (`vm.c`) | Driver written here; the engine it measures is the MIT reference |
| `tools/differential_parity.py` | NEW_IMPLEMENTATION | |
| `examples/hello.malbolge` | PORTED_FROM_REFERENCE | Canonical 48-step program, MIT, copied verbatim from the reference `examples/` |
| BUILD/DEBUGGING note | — | WinLibs MinGW 16.1 was used; static linking chosen because this host's PATH contains foreign `libstdc++-6.dll`s that crash dynamic binaries |

## Authorship statement

The C++ code was written with AI coding-assistant involvement (OpenCode /
Kimi) under the direction of the repository owner, who designed the audit,
parity and verification plan and reviewed the result. No claim of purely
human authorship is made for any file. The reference C engine was written by
the same owner independently of this project.

## Third-party dependencies

None at runtime or build time beyond a C++20 standard library and CMake.
The *verification tooling* compiles the MIT reference engine as its oracle;
that code is not copied into this repository.

# HACKEREARTH READINESS AUDIT

Date: 2026-09-03. Prepared for an honest self-assessment. **No submission has
been made.** This document exists so nothing is claimed that cannot be shown.

| Field | Value |
|---|---|
| PRIMARY_LANGUAGE | C++ (C++20) |
| REPO_CREATED_AT | 2026-09-03 (created today, locally) |
| REPO_AGE | **days, not months** |
| LOC_SOURCE_ONLY (approx., excludes build/, evidence/, docs JSON) | ~1,400 lines hand-structured C++ + ~700 lines of tests/fuzz/bench/drivers — measured with git, see below |
| ACTIVELY_MAINTAINED | created today; the reference engine has been actively worked on |
| OWNERSHIP | repository owner authored the reference C engine; this repo is its C++ successor |
| CORE_PROVENANCE | VM semantics PORTED from the owner's own MIT reference (documented per-file in PROVENANCE.md); architecture/API/tracing/tests/fuzz/bench/CLI are NEW |
| AI_ASSISTANCE | **Yes — substantial.** The C++ code was produced with an AI coding assistant under owner direction (see PROVENANCE.md). No claim of purely human authorship is made. |
| THIRD_PARTY_DEPENDENCIES | none at runtime/build (STL + CMake only); the MIT reference engine is used *externally* as a verification oracle |
| PRIVATE | yes — created as a PRIVATE repository |
| SYSTEMS_DEPTH | real: interpreter/VM, differential verification harness (70 cases, 0 mismatches), typed tracing + non-interference proof, property fuzzing (5 families × 20k cases), C-vs-C++ benchmarks, CLI + embeddable library |

## Eligibility reasoning

- If a programme requires the repository to have existed for **≥ 1 year**:
  `ELIGIBILITY = NOT_ELIGIBLE`. This repo was created today. No backdating,
  no history fabrication.
- If a programme requires the core to be **fully human-written**: this repo
  does not satisfy that as documented above.
- If a programme evaluates **technical depth** (correctness evidence,
  architecture, testing, tooling): this repo is genuinely strong — parity is
  demonstrated, not asserted.

Do not submit until the programme's exact current requirements are read and
mapped against this table again.

## Decision log

- **2026-09-03** — Owner decision: the repository stays **private and
  pushed** (GitHub `DannyBaanks/MalbolgeEngineCPP`, PRIVATE). No submission
  was made; HackerEarth cohort intake is currently backlogged/closed. When
  submissions reopen, re-run this audit: the `REPO_AGE` field will then read
  in months, and only the age-dependent eligibility line can change — the
  AI-assistance provenance line is permanent and must never be "corrected"
  retroactively.

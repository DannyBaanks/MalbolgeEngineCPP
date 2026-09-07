# VERDICTS — recorded 2026-09-03

All verdicts were produced by actually running the commands on the machine
described in `environment.json`. Commands and raw outputs are in the files
named below and in `hashes.json`.

| Verdict | Value | Evidence |
|---|---|---|
| CPP_BUILD | PASS | clean `cmake -S . -B build && cmake --build build` |
| CPP_TESTS | PASS (6/6 suites, via ctest) | `ctest-output.txt` |
| REFERENCE_PARITY | **DEMONSTRATED** — 70 cases, 0 mismatches (stdout bytes, steps, halt status, exit codes) | `parity.json` |
| TRACE_NON_INTERFERENCE | PASS (unit test `trace_non_interference`) | `ctest-output.txt` |
| FUZZ_PROPERTIES | PASS (F1–F5, 20k iters, seed 0x9e3779b97f4a7c15) | `fuzz.txt` |
| BENCHMARKS | recorded, C faster on both micro-metrics (ratios in file) | `benchmark-summary.json` |
| CLEAN_CHECKOUT | **PASS** — fresh `git clone` to temp, configure, build, 6/6 tests, CLI hello, and a full re-run of the differential parity (70/70) from the clone | recorded 2026-09-03 |
| PRIVATE_REPO_READY | yes | repository visibility at push time |

Non-claims (deliberate):
- No claim of runtime superiority over the C engine.
- No truth-machine/quine parity cases (not available locally) — classed
  as NOT_DEMONSTRATED, not failed.
- No claim of human-only authorship (see PROVENANCE.md).

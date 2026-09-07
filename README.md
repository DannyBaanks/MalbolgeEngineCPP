# MalbolgeEngineCPP

A standalone Classic Malbolge interpreter/VM implemented in modern C++20,
with deterministic execution, CLI and embeddable library APIs, differential
verification against the reference C engine, structured tracing, property
fuzzing, and benchmarks.

It is a behavioral port-and-rewrite of
[DannyBaanks/Malbolge-Engine](https://github.com/DannyBaanks/Malbolge-Engine)
(MIT) — not a rename: the engine is restructured into a RAII, value-typed,
header/documented C++ library with a test suite, fuzz harness, and typed
tracing that the C engine does not have.

## Quick start

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

./build/malbolge run examples/hello.malbolge
# Hello, world.
# steps: 48 | status: HALT
```

## CLI

```
malbolge run    <file.mal> [--input <file>] [--max-steps N] [--json]
malbolge trace  <file.mal> --output <trace.jsonl> [--input <file>] [--max-steps N]
malbolge doctor
malbolge version
```

Exit codes: `0` success (including step-limit timeouts), `1` usage,
`2` file/program error. Machine-readable output only behind `--json` /
`--output`.

## Library

```cpp
#include <malbolge/vm.hpp>

malbolge::VM vm = malbolge::VM::from_source(source);
vm.set_input(input_bytes);
auto result = vm.run(1'000'000);   // or vm.step() for single steps

result.steps;        // cumulative executed instructions
result.reason;       // HALT | INVALID_FETCH | INPUT_EOF | STEP_LIMIT
vm.output();         // captured output bytes
```

The engine never touches the host beyond what you hand it: input is a byte
buffer, output accumulates in memory, and tracing is an observer interface
(`malbolge::Tracer`) that cannot mutate state.

## Semantics

Frozen in [docs/REFERENCE_BEHAVIOR.md](docs/REFERENCE_BEHAVIOR.md).
Notable deliberate deviations from textbook Malbolge, inherited from the
reference engine: input EOF **terminates** the machine; output is capped at
65536 bytes; memory self-fills lazily in a way that is observably affected
by writes above the program length.

## Verification

- **Unit tests**: 6 suites, run via `ctest`.
- **Differential parity**: `py tools/differential_parity.py` builds the
  reference C engine and compares byte-exact output, steps and status across
  a 70-case corpus → `DEMONSTRATED` (see `evidence/parity.json`).
- **Trace non-interference**: traced and untraced runs are observably
  identical (test `trace_non_interference`).
- **Fuzz/properties**: `./build/fuzz_malbolge.exe [seed]` — 5 property
  families over 20k cases per family; deterministic and seed-reproducible.
- **Benchmarks**: `py tools/run_benchmarks.py` measures this engine against
  the reference C engine on the same machine → `bench/results.json`.

## Layout

```
include/malbolge/   public headers (vm, memory, decode, crazy, trace, result)
src/                library implementation
cli/                the malbolge CLI
tests/              unit tests (in-repo mini framework)
fuzz/               property harness
bench/              micro-benchmarks (+ reference C driver)
examples/           hello.malbolge
docs/               REFERENCE_BEHAVIOR.md (the frozen spec)
tools/              parity + benchmark drivers
evidence/           run evidence and verdicts
```

## Requirements

C++20 compiler (developed with MinGW g++ 16.1), CMake ≥ 3.20. No third-party
dependencies; the core library is STL-only.

## License / provenance

MIT (see LICENSE). The reference C engine is MIT by the same author.
Every subsystem is classified in [PROVENANCE.md](PROVENANCE.md).

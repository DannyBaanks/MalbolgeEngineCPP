# Benchmarks

Run everything (builds the reference C engine's benchmark driver too):

```sh
cmake -S . -B build && cmake --build build
py tools/run_benchmarks.py
```

Produces `bench/results.json` and `evidence/benchmark-summary.json`.

## What is measured

- `crazy_ns_per_op` — the table-lookup crz on random word pairs.
- `rotate_ns_per_op` / `decode_ns_per_op` — C++ only (no C counterpart in
  the reference's public interface).
- `vm_hello_ns_per_run` — full hello-world run (48 steps) including VM
  construction/init. This is where the engines differ most; see
  ARCHITECTURE.md for why.

Raw numbers from the latest recorded run are in `results.json`. Do not quote
them without the environment recorded in `evidence/environment.json`.

## Reference driver

`reference_bench.c` is a driver compiled against the *external* reference
checkout (`MALBOLGE_REFERENCE_ROOT` or the sibling default). The reference
code itself is never copied here.

# Architecture

## Layers

```
┌────────────────────────────────────────────┐
│ cli/main.cpp        arg parsing, files, I/O│   ← the only place with argv/stdout
├────────────────────────────────────────────┤
│ malbolge::VM        step()/run(), results  │
├──────────┬──────────┬───────────┬──────────┤
│ Memory   │ decode   │ crazy     │ Tracer   │   ← primitives, each unit-tested
│ overlay+ │ + rotate │ table     │ observer │
│ chain    │ + encrypt│           │          │
└──────────┴──────────┴───────────┴──────────┘
```

The library (`malbolge_core`) never reads argv, never prints, never opens
files. The CLI is a thin adapter: file → `VM::from_source`, run, print.

## Determinism

A `VM` instance is closed-world: all state (registers, memory, input cursor,
output) lives in the object. Same program + same input + same step limit ⇒
identical output, steps, and final registers. No threads, no global mutable
state (the `crazy5` table is an immutable `static const`).

## Memory model

`Memory` replicates the reference engine's sparse-overlay-plus-lazy-chain.
The laziness is load-bearing for parity (writes above the program size
change the fill seed), so it is kept verbatim instead of "cleaned up".
See docs/REFERENCE_BEHAVIOR.md for the exact rules.

## Stop model

The reference engine collapses every termination into one boolean. That
boolean is the parity surface; the C++ engine additionally *classifies*
stops (`Halt`, `InvalidFetch`, `InputEof`, `StepLimit`) without changing
observable behavior.

## Tracing

`Tracer` is a pure-observer interface. The VM calls `on_event` after each
instruction; the tracer holds no references into the VM. Non-interference
is enforced by construction and verified by `trace_non_interference`.

## Error model

- Load time: invalid characters throw `std::invalid_argument` (translated
  to exit code 2 by the CLI).
- Run time: no exceptions, no faults; everything becomes a `StopReason`.

## Bounded everything

Output cap 65536 bytes, overlay cap 200000 entries, explicit step budget —
all inherited from the reference and part of the parity contract.

## Performance notes (measured, see bench/results.json)

The C reference engine wins the hello-world rerun benchmark because its
"reset" is a couple of pointer resets over static arrays, while the C++ VM
constructs fresh vectors per run. `crazy` itself is within noise of the C
engine. No claim of superiority is made; numbers are in evidence/.

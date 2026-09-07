# Security policy

## Threat model

This is an interpreter for an esoteric language. The interesting security
properties are:

- A Malbolge program must never escape the VM: it can read only its input
  buffer, write only its output buffer, and execute at most `max_steps`
  instructions. Memory, output and execution are all explicitly bounded.
- Malformed source is rejected at load time; malformed *runtime state*
  (non-printable fetch) terminates the machine deterministically.

## Reporting

Open a private GitHub advisory on this repository, or contact the owner
directly. Do not open public issues for vulnerabilities.

## Scope

In scope: VM memory-safety bugs, unbounded resource use reachable from a
crafted program, CLI path handling. Out of scope: the reference C engine
(report upstream), the Python harnesses (developer tooling).

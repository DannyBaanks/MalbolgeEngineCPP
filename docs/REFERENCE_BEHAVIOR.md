# Reference behavior — frozen audit of DannyBaanks/Malbolge-Engine

Audited 2026-09-03 against commit
`e7faeb1bb469e8f21dfb37e9c261bb84b2590b01` of
https://github.com/DannyBaanks/Malbolge-Engine (MIT).

This document is the specification the C++ implementation is held to. Where
the C engine deviates from "textbook" Malbolge, **the C engine wins** — this
project's parity claim is against *this* reference, not against folklore.

## Memory

- `MEM_SIZE = 59049` (3^10), valid word values are `[0, 59049)`.
- **Sparse overlay**: program cells and all runtime writes live in an
  overlay array of at most `200000` (`MAX_OVERLAY`) `(index, value)` pairs.
  `set` updates the first matching entry; `get` scans from the front.
  Writes past the cap are **silently dropped**.
- **Lazy chain**: never-written cells with address `>= fill_start`
  (`fill_start = max(ncells, 2)`) are materialized in blocks of `243`
  (`BLOCK_SIZE`) following `chain[i] = crz(mem[i-1], mem[i-2])`.
- Fill seeding quirks (preserved intentionally):
  - The first fill seeds `p1/p2` from the *overlay entries* of
    `fill_start-1` and `fill_start-2`, defaulting to `0` when absent.
  - Exactly at `i == tail_end` and `i == tail_end + 1`
    (`tail_end = (fill_start / 243 + 1) * 243`) the operands are re-read
    through the full read path, so overlay writes influence those cells.
  - Consequently, a program that writes above its own length changes the
    fill — pre-filling memory eagerly is **not** observably equivalent.
- Reading a never-written cell **below** `fill_start` yields `0`.
- Reading addresses `>= 59049` does not happen through the pointer model
  (registers are words modulo wrap), so it is out of scope.

## Registers and execution loop

- Registers `a`, `c`, `d`, all start at 0.
- One step:
  1. `ins = mem[c]`; if `ins < 33 || ins > 126` → terminate
     (`INVALID_FETCH`; still counts as an executed step).
  2. `op = (ins + c) % 94`.
  3. Dispatch:
     - `4`  → `c = mem[d]`
     - `5`  → output byte `a % 256` (output buffer cap `65536` bytes,
       further bytes **silently dropped**)
     - `23` → `a = next input byte`; on EOF → **terminate** (NOT the
       textbook `a = 59048`)
     - `39` → `r = rotate(mem[d]); mem[d] = r; a = r`
     - `40` → `d = mem[d]`
     - `62` → `r = crz(a, mem[d]); mem[d] = r; a = r`
     - `81` → halt
     - else  → NOP
  4. If the instruction terminated the machine, stop.
  5. If `mem[c]` is printable, rewrite it through the ENCRYPT table.
  6. `c` and `d` increment with wrap at `59048 → 0`.
- Steps are counted per fetched instruction, including the terminating one.
- Default step budget: 100,000,000; exhausting it is a timeout, not a fault.

## Tables

- `rotate(n) = 19683 * (n % 3) + n / 3` (one-trit right rotate).
- Crazy trit truth table (rows = `b` trit, columns = `a` trit):

  ```
        a=0  a=1  a=2
  b=0 {  1,   0,   0  }
  b=1 {  1,   0,   2  }
  b=2 {  2,   2,   1  }
  ```

  Computed as `crazy5[a%243][b%243] + 243 * crazy5[a/243][b/243]`.
- `ENCRYPT` (94 entries, for cell values 33..126):

  ```
  5z]&gqtyfr$(we4{WP)H-Zn,[%\3dL+Q;>U!pJS72FhOA1CB6v^=I_0/8|jsb9m<.TVac`uY*MK'X~xDl}REokN:#?G"i@
  ```

## Source loading

- Input is **raw Malbolge text**; each non-whitespace byte is one cell.
- Whitespace skipped: space, tab, CR, LF.
- Any other byte outside `[33,126]` → CLI error "invalid character", exit 2.

## Reference vectors (verified)

| program | steps | status | output |
|---|---|---|---|
| `hello.malbolge` | 48 | halt | `Hello, world.` |
| `Q` | 1 | halt | (empty) |
| `c` | 2 | invalid fetch | one byte `0x00` |
| `u` (no input) | 1 | input EOF | (empty) |

## Reference CLI interface

`malbolge <program> [max_steps]` — program input on stdin; stdout = program
output + trailing newline; stderr = `steps: N | status: HALTED|TIMEOUT`.
Exit codes: 0 (even on timeout), 1 usage, 2 file/character error.

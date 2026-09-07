# Contributing

## Ground rules

1. **Parity is sacred.** Any semantic change requires rerunning
   `py tools/differential_parity.py` and getting `DEMONSTRATED`. If the
   behavior intentionally diverges from the reference, update
   `docs/REFERENCE_BEHAVIOR.md` in the same commit and say why.
2. **No dependency creep.** Core library stays STL-only. New tools depend on
   nothing beyond Python 3 for harnesses.
3. **No LOC farming.** A contribution that mostly adds fixture bulk will be
   rejected on principle.
4. **Provenance honesty.** Update PROVENANCE.md when adding code of a new
   origin class, including AI-assisted code.

## Workflow

```sh
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure
./build/fuzz_malbolge.exe            # property suite
py tools/differential_parity.py      # requires reference checkout
```

Keep commits small and titled imperatively. Raw evidence (parity JSON,
benchmark JSON) goes in `evidence/` with the command that produced it.

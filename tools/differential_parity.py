#!/usr/bin/env python3
"""Differential parity harness: reference C engine vs this C++ engine.

Builds the reference engine (MIT, DannyBaanks/Malbolge-Engine) from source,
then runs a corpus of programs through BOTH command-line interfaces and
compares byte-exact stdout, step counts and halt/timeout status.

Usage:
    py tools/differential_parity.py --reference PATH [--out evidence/parity.json]

The reference repo (Malbolge-Engine) path is required: pass --reference or set
MALBOLGE_REFERENCE_ROOT.
"""
from __future__ import annotations

import argparse
import json
import os
import random
import subprocess
import sys
import tempfile
import hashlib
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

HELLO = "(=<`#9]~6ZY327Uv4-QsqpMn&+Ij\"'E%e{Ab~w=_:]Kw%o44Uqp0/Q?xNvL:`H%c#DD2^WV>gY;dts76qKJImZkj"


def build_reference(ref_root: Path, work: Path) -> Path:
    """Compile the reference CLI. Returns path to the executable."""
    out = work / "malbolge_ref.exe"
    cmd = [
        "gcc", "-O2", "-static", "-std=c11", "-Wall", "-Wextra",
        "-I", str(ref_root / "src"),
        str(ref_root / "src" / "vm.c"),
        str(ref_root / "src" / "malbolge.c"),
        "-o", str(out),
    ]
    subprocess.run(cmd, check=True, capture_output=True)
    return out


def run_reference(exe: Path, program: bytes, max_steps: int, stdin: bytes) -> dict:
    """Run the reference CLI: `malbolge <tmpfile> [max_steps]`, stdin=input.

    stdout = program output + trailing newline.
    stderr = "steps: N | status: HALTED|TIMEOUT".
    """
    with tempfile.NamedTemporaryFile("wb", suffix=".mal", delete=False) as f:
        f.write(program)
        tmp = f.name
    try:
        p = subprocess.run(
            [str(exe), tmp, str(max_steps)],
            input=stdin, capture_output=True, timeout=120,
        )
        # stderr: "steps: 48 | status: HALTED"
        steps, status = -1, "?"
        line = p.stderr.decode("ascii", "replace").strip()
        if line.startswith("steps:"):
            left, _, right = line.partition("|")
            steps = int(left.split(":")[1])
            status = right.split(":")[1].strip()
        if p.returncode == 2:
            return {"exit": 2, "stdout": b"", "steps": None, "terminated": None}
        # strip the trailing newline the reference adds
        out = p.stdout[:-1] if p.stdout.endswith(b"\n") else p.stdout
        return {"exit": 0, "stdout": out, "steps": steps,
                "terminated": status == "HALTED"}
    finally:
        os.unlink(tmp)


def run_cpp(exe: Path, program: bytes, max_steps: int, stdin_bytes: bytes) -> dict:
    with tempfile.NamedTemporaryFile("wb", suffix=".mal", delete=False) as f:
        f.write(program)
        tmp = f.name
    infile = None
    try:
        cmd = [str(exe), "run", tmp, "--max-steps", str(max_steps), "--json"]
        if stdin_bytes:
            with tempfile.NamedTemporaryFile("wb", suffix=".bin", delete=False) as fi:
                fi.write(stdin_bytes)
                infile = fi.name
            cmd += ["--input", infile]
        p = subprocess.run(cmd, capture_output=True, timeout=120)
        if p.returncode == 2:
            return {"exit": 2, "stdout": b"", "steps": None, "terminated": None}
        # --json mode carries the metadata on stdout; the program bytes go
        # un-parsed. Re-run in plain mode to capture raw output bytes.
        cmd2 = [str(exe), "run", tmp, "--max-steps", str(max_steps)]
        if infile:
            cmd2 += ["--input", infile]
        p2 = subprocess.run(cmd2, capture_output=True, timeout=120)
        out = p2.stdout[:-1] if p2.stdout.endswith(b"\n") else p2.stdout
        return {"exit": 0, "stdout": out, "steps": steps_from_json(p),
                "terminated": terminated_from_json(p)}
    finally:
        os.unlink(tmp)
        if infile:
            os.unlink(infile)


def steps_from_json(p) -> int:
    return json.loads(p.stdout.decode())["steps"]


def terminated_from_json(p) -> bool:
    return json.loads(p.stdout.decode())["status"] != "STEP_LIMIT"


def corpus() -> list[dict]:
    cases = [
        {"name": "hello", "program": HELLO.encode(), "input": b"", "steps": 100000},
        {"name": "halt_Q", "program": b"Q", "input": b"", "steps": 100000},
        {"name": "out_zero_byte", "program": b"c", "input": b"", "steps": 100000},
        {"name": "in_eof", "program": b"u", "input": b"", "steps": 100000},
        {"name": "in_a_byte", "program": b"u", "input": b"A", "steps": 100000},
        {"name": "in_two_bytes", "program": b"uu", "input": b"AB", "steps": 100000},
        {"name": "whitespace_heavy", "program": b" \t\r\nQ \n ", "input": b"", "steps": 100000},
        {"name": "malformed_control_char", "program": b"Q\x01x", "input": b"", "steps": 100000},
        {"name": "limited_steps", "program": HELLO.encode(), "input": b"", "steps": 10},
        {"name": "jmp_self_loop_bounded", "program": b"b", "input": b"", "steps": 4000},
    ]
    # Deterministic random printable programs of varying sizes.
    rng = random.Random(0xC0FFEE)
    printable = [bytes([c]) for c in range(33, 127)]
    for i in range(60):
        n = 1 + rng.randrange(0, 400 if i % 6 else 3000)
        prog = b"".join(rng.choice(printable) for _ in range(n))
        inp = bytes(rng.randrange(256) for _ in range(rng.randrange(0, 16)))
        cases.append({"name": f"random_{i:03d}",
                      "program": prog, "input": inp,
                      "steps": 20000 if i % 3 else 200000})
    return cases


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--reference",
                    default=os.environ.get("MALBOLGE_REFERENCE_ROOT"))
    ap.add_argument("--cpp-exe", default=str(REPO / "build" / "malbolge.exe"))
    ap.add_argument("--out", default=str(REPO / "evidence" / "parity.json"))
    args = ap.parse_args()

    if not args.reference:
        print("reference engine path not given: pass --reference or set "
              "MALBOLGE_REFERENCE_ROOT to the Malbolge-Engine repo",
              file=sys.stderr)
        return 2
    ref_root = Path(args.reference)
    if not ref_root.is_dir() or not (ref_root / "src" / "vm.c").exists():
        print(f"reference engine not found at {ref_root}", file=sys.stderr)
        return 2

    with tempfile.TemporaryDirectory() as work:
        workp = Path(work)
        print(f"building reference from {ref_root} ...")
        ref_exe = build_reference(ref_root, workp)
        print(f"reference: {ref_exe}")

        results = []
        mismatches = []
        for case in corpus():
            ref = run_reference(ref_exe, case["program"], case["steps"], case["input"])
            cpp = run_cpp(Path(args.cpp_exe), case["program"], case["steps"], case["input"])

            ok = (ref["exit"] == cpp["exit"]
                  and ref["stdout"] == cpp["stdout"]
                  and ref["steps"] == cpp["steps"]
                  and ref["terminated"] == cpp["terminated"])
            rec = {
                "name": case["name"],
                "program_sha256": hashlib.sha256(case["program"]).hexdigest()[:16],
                "program_len": len(case["program"]),
                "input_len": len(case["input"]),
                "max_steps": case["steps"],
                "ref": {k: (v.hex() if isinstance(v, bytes) else v) for k, v in ref.items()},
                "cpp": {k: (v.hex() if isinstance(v, bytes) else v) for k, v in cpp.items()},
                "match": ok,
            }
            results.append(rec)
            if not ok:
                mismatches.append(rec)
                print(f"MISMATCH {case['name']}: ref={ {k: v for k, v in ref.items() if k != 'stdout'} } "
                      f"cpp={ {k: v for k, v in cpp.items() if k != 'stdout'} }")

    verdict = "DEMONSTRATED" if not mismatches else "FAILED"
    report = {
        "verdict": {"C_CPP_BEHAVIORAL_PARITY": verdict},
        "cases": len(results),
        "mismatches": len(mismatches),
        "comparisons": ["stdout_bytes", "steps", "terminated", "exit_code"],
        "details": results,
    }
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"\n{len(results)} cases, {len(mismatches)} mismatches -> {verdict}")
    print(f"written: {out}")
    return 0 if not mismatches else 1


if __name__ == "__main__":
    sys.exit(main())

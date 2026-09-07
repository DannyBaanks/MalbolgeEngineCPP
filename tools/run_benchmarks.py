#!/usr/bin/env python3
"""Run C++ and reference-C benchmarks; write bench/results.json + evidence."""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent


def main() -> int:
    ref_env = os.environ.get("MALBOLGE_REFERENCE_ROOT")
    if not ref_env:
        print("MALBOLGE_REFERENCE_ROOT not set: point it at the "
              "Malbolge-Engine repository", file=sys.stderr)
        return 2
    ref_root = Path(ref_env)
    iters = "2000000"

    with tempfile.TemporaryDirectory() as work:
        ref_bench = Path(work) / "ref_bench.exe"
        subprocess.run(
            ["gcc", "-O2", "-static", "-std=c11",
             "-I", str(ref_root / "src"),
             str(REPO / "bench" / "reference_bench.c"),
             str(ref_root / "src" / "vm.c"),
             "-o", str(ref_bench)],
            check=True, capture_output=True)

        cpp = subprocess.run([str(REPO / "build" / "bench_malbolge.exe"), iters],
                             capture_output=True, check=True, text=True).stdout
        ref = subprocess.run([str(ref_bench), iters],
                             capture_output=True, check=True, text=True).stdout

    cpp_j = json.loads(cpp)
    ref_j = json.loads(ref)
    results = {
        "iterations": int(iters),
        "cpp": cpp_j,
        "reference_c": ref_j,
        "ratios": {
            "crazy_c_over_cpp": round(ref_j["crazy_ns_per_op"] / cpp_j["crazy_ns_per_op"], 3),
            "vm_hello_c_over_cpp": round(ref_j["vm_hello_ns_per_run"] / cpp_j["vm_hello_ns_per_run"], 3),
        },
        "note": "Same machine, same inputs, back to back. ratio > 1 means C++ faster.",
    }
    for out in (REPO / "bench" / "results.json",
                REPO / "evidence" / "benchmark-summary.json"):
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(json.dumps(results["ratios"], indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())

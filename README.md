# MalbolgeEngineCPP

Interprete/VM standalone de Malbolge Clasico implementado en C++20 moderno,
con ejecucion determinista, APIs de CLI y libreria embebible, verificacion
diferencial contra el motor C de referencia, trazado estructurado, fuzzing
de propiedades, y benchmarks.

Es un port-and-rewrite conductual de
[DannyBaanks/Malbolge-Engine](https://github.com/DannyBaanks/Malbolge-Engine)
(MIT) — no un renombrado: el motor se reestructuro en una libreria C++
RAII, de tipos valor, con documentacion en headers, suite de pruebas, harness de fuzz,
y trazado tipado que el motor C no tiene.

## Inicio rapido

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

Exit codes: `0` exito (incluyendo timeouts por limite de pasos), `1` uso,
`2` error de archivo/programa. Output de maquina-readable solo detras de `--json` /
`--output`.

## Libreria

```cpp
#include <malbolge/vm.hpp>

malbolge::VM vm = malbolge::VM::from_source(source);
vm.set_input(input_bytes);
auto result = vm.run(1'000'000);   // o vm.step() para pasos individuales

result.steps;        // instrucciones ejecutadas acumuladas
result.reason;       // HALT | INVALID_FETCH | INPUT_EOF | STEP_LIMIT
vm.output();         // bytes de output acumulados
```

El motor nunca toca el host mas de lo que le entregas: el input es un buffer
de bytes, el output se acumula en memoria, y el trazado es una interfaz de observador
(`malbolge::Tracer`) que no puede mutar estado.

## Semanticas

Congeladas en [docs/REFERENCE_BEHAVIOR.md](docs/REFERENCE_BEHAVIOR.md).
Desviaciones deliberadas notables del Malbolge de textbook, heredadas del
motor de referencia: el input EOF **termina** la maquina; el output esta limitado a
65536 bytes; la memoria se llena perezosamente de una forma que se ve afectada
por escrituras arriba de la longitud del programa.

## Verificacion

- **Pruebas unitarias**: 6 suites, corren via `ctest`.
- **Paridad diferencial**: `py tools/differential_parity.py` construye el
  motor C de referencia y compara output byte-exacto, pasos y status en
  un corpus de 70 casos → `DEMONSTRATED` (ver `evidence/parity.json`).
- **No-interferencia de trace**: corridas con trace y sin trace son observablamente
  identicas (prueba `trace_non_interference`).
- **Fuzz/propiedades**: `./build/fuzz_malbolge.exe [seed]` — 5 familias de propiedades
  sobre 20k casos por familia; determinista y reproducible con semilla.
- **Benchmarks**: `py tools/run_benchmarks.py` mide este motor contra
  el motor C de referencia en la misma maquina → `bench/results.json`.

## Layout

```
include/malbolge/   headers publicos (vm, memory, decode, crazy, trace, result)
src/                implementacion de la libreria
cli/                el CLI de malbolge
tests/              pruebas unitarias (mini framework del repo)
fuzz/               harness de propiedades
bench/              micro-benchmarks (+ driver C de referencia)
examples/           hello.malbolge
docs/               REFERENCE_BEHAVIOR.md (la spec congelada)
tools/              drivers de paridad + benchmark
evidence/           evidencia de corrida y veredictos
```

## Requisitos

Compilador C++20 (desarrollado con MinGW g++ 16.1), CMake ≥ 3.20. Sin dependencias
de terceros; la libreria central es solo STL.

## Licencia / procedencia

MIT (ver LICENSE). El motor C de referencia es MIT del mismo autor.
Cada subsistema esta clasificado en [PROVENANCE.md](PROVENANCE.md).

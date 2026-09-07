# GUÍA — cómo operar MalbolgeEngineCPP

(Español a propósito: esta guía es para quien usa el repo a diario, no para contribuidores.)

## El comando que viniste a buscar

```powershell
cmake -S . -B build; cmake --build build; ctest --test-dir build
```

## Regla de oro

**Nunca afirmes paridad sin correr `py tools/differential_parity.py`.**
La paridad es un resultado medido contra el motor C compilado en el momento,
no una propiedad del source code.

## Comandos

| Comando | Qué hace | Salida real |
|---|---|---|
| `build\malbolge.exe run examples\hello.malbolge` | corre el programa | `Hello, world.` + `steps: 48 \| status: HALT` (stderr) |
| `... run X --max-steps N` | acota pasos | `status: STEP_LIMIT` si se agota |
| `... run X --input in.bin` | alimenta stdin del programa | idem |
| `... run X --json` | resumen JSON por stdout | `{"steps":48,"status":"HALT",...}` |
| `... trace X --output t.jsonl` | traza tipada por instrucción | 48 líneas JSON para hello |
| `... doctor` | auto-chequeo del entorno | `malbolge-cpp doctor: OK` |
| `ctest --test-dir build` | 6 suites de tests | `100% tests passed` |
| `build\fuzz_malbolge.exe [seed]` | propiedades F1–F5 | `ALL PROPERTIES HELD` |
| `py tools/differential_parity.py` | C vs C++ byte a byte | `70 cases, 0 mismatches -> DEMONSTRATED` |
| `py tools/run_benchmarks.py` | bench C vs C++ | escribe `bench/results.json` |

## Códigos de salida

| exit | significado |
|---|---|
| 0 | todo bien, incluso con `STEP_LIMIT` |
| 1 | error de uso (argumentos) |
| 2 | archivo ilegible o carácter inválido en el programa |

## Trampas (costaron tiempo real, quedan escritas)

1. **Esta máquina tiene varios MinGW en PATH.** Un binario dinámico carga el
   `libstdc++-6.dll` de QEMU y crashea con `0xC0000005` sin mensaje. El repo
   enlaza `-static` por eso. Si un ejecutable crashea en Release y funciona
   en Debug, sospecha esto primero.
2. `run --json` pone el JSON en **stdout**; el programa NO imprime sus bytes
   en ese modo (el arnés de paridad corre dos veces por esto).
3. `status: STEP_LIMIT` no es error: exit code sigue siendo 0.
4. EOF de entrada **termina** la máquina (no carga 59048 como el Malbolge de
   libro). Es intencional: paridad con el motor de referencia.
5. La salida está capada en 65536 bytes; el excedente se descarta en silencio.

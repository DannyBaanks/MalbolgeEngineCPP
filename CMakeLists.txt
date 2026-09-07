cmake_minimum_required(VERSION 3.20)
project(MalbolgeEngineCPP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static runtime: this machine has several unrelated MinGW installations on
# PATH; a dynamically linked executable picks up a foreign libstdc++-6.dll
# and crashes. Static linkage removes that whole failure class.
if(MINGW OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32))
  add_link_options(-static)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# Core library: everything except the CLI. STL-only, no third-party deps.
# ---------------------------------------------------------------------------
add_library(malbolge_core STATIC
  src/crazy.cpp
  src/memory.cpp
  src/vm.cpp
  src/trace.cpp
)
target_include_directories(malbolge_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(malbolge_core PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
add_executable(malbolge cli/main.cpp)
target_link_libraries(malbolge PRIVATE malbolge_core)

# ---------------------------------------------------------------------------
# Tests (tiny in-repo test framework, no external dependency)
# ---------------------------------------------------------------------------
enable_testing()

foreach(t IN ITEMS test_crazy test_decode test_memory test_vm test_trace test_cli_api)
  add_executable(${t} tests/${t}.cpp)
  target_link_libraries(${t} PRIVATE malbolge_core)
  add_test(NAME ${t} COMMAND ${t})
endforeach()

# Copy example programs next to the tests so ctest runs are self-contained.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.malbolge
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/examples)

# ---------------------------------------------------------------------------
# Benchmarks (opt-in target: cmake --build build --target bench_malbolge)
# ---------------------------------------------------------------------------
add_executable(bench_malbolge bench/bench.cpp)
target_link_libraries(bench_malbolge PRIVATE malbolge_core)

# ---------------------------------------------------------------------------
# Fuzz / property harness (deterministic seeds; no libFuzzer required)
# ---------------------------------------------------------------------------
add_executable(fuzz_malbolge fuzz/fuzz.cpp)
target_link_libraries(fuzz_malbolge PRIVATE malbolge_core)

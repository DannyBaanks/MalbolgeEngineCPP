// MalbolgeEngineCPP — the "crazy" (crz) tritwise operation.
#pragma once

#include <cstdint>

namespace malbolge {

// crz(a, b) on ten-trit words (values in [0, 59049)).
//
// Tritwise truth table (rows = b trit, columns = a trit), exactly as
// frozen from the reference C engine:
//
//       a=0  a=1  a=2
// b=0 {  1,   0,   0  }
// b=1 {  1,   0,   2  }
// b=2 {  2,   2,   1  }
//
// Implementation: the 10-trit word is split into two 5-trit halves and
// each half is resolved with a 243x243 table lookup. The table is built
// once at first use (thread-safe static initialization).

std::uint32_t crazy(std::uint32_t a, std::uint32_t b);

// Slow, obviously-correct definition used by tests/fuzz as an oracle.
std::uint32_t crazy_direct(std::uint32_t a, std::uint32_t b);

}  // namespace malbolge

// MalbolgeEngineCPP — crazy (crz) implementation.
#include "malbolge/crazy.hpp"

#include <array>

#include "malbolge/constants.hpp"

namespace malbolge {

namespace {

// Tritwise table, rows indexed by the b trit, columns by the a trit.
// Identical to the reference C engine's CRAZY_TBL.
constexpr std::uint8_t kCrazyTbl[3][3] = {
    {1, 0, 0},
    {1, 0, 2},
    {2, 2, 1},
};

// Two-level table: crazy5[a5][b5] resolves 5 trits at once. 243*243
// entries * 4 bytes = ~236 KB, built once. Thread-safe static init.
const auto& crazy5_table() {
    static const std::array<std::array<std::uint32_t, kHalf>, kHalf> table = [] {
        std::array<std::array<std::uint32_t, kHalf>, kHalf> t{};
        for (std::size_t a5 = 0; a5 < kHalf; ++a5) {
            for (std::size_t b5 = 0; b5 < kHalf; ++b5) {
                std::uint32_t r = 0, p = 1;
                std::size_t aa = a5, bb = b5;
                for (int k = 0; k < 5; ++k) {
                    r += static_cast<std::uint32_t>(kCrazyTbl[bb % 3][aa % 3]) * p;
                    aa /= 3;
                    bb /= 3;
                    p *= 3;
                }
                t[a5][b5] = r;
            }
        }
        return t;
    }();
    return table;
}

}  // namespace

std::uint32_t crazy(std::uint32_t a, std::uint32_t b) {
    const auto& t = crazy5_table();
    return t[a % kHalf][b % kHalf] + kHalf * t[a / kHalf][b / kHalf];
}

std::uint32_t crazy_direct(std::uint32_t a, std::uint32_t b) {
    std::uint32_t r = 0, p = 1;
    for (int k = 0; k < 10; ++k) {
        r += static_cast<std::uint32_t>(kCrazyTbl[b % 3][a % 3]) * p;
        a /= 3;
        b /= 3;
        p *= 3;
    }
    return r;
}

}  // namespace malbolge

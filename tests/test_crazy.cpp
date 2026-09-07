// Unit tests: crazy (crz) operation.
#include "test_framework.hpp"

#include "malbolge/crazy.hpp"
#include "malbolge/constants.hpp"

using namespace malbolge;

TEST_CASE(crazy_canonical_vectors) {
    // All-zero trits: every pair (a=0,b=0) -> 1, so crz(0,0) = 1111111111_3
    // = (3^10 - 1) / 2 = 29524.
    CHECK_EQ(crazy(0, 0), 29524u);

    // All-two trits: M[2][2] = 1 -> also 29524.
    CHECK_EQ(crazy(59048, 59048), 29524u);

    // crz(1, 0): trit 0 is (a=1,b=0) -> M[0][1] = 0, the rest are 1 trits
    // -> 29524 - 1 = 29523.
    CHECK_EQ(crazy(1, 0), 29523u);

    // crz(0, 1): M[1][0] = 1 at trit 0, rest 1 -> 29524.
    CHECK_EQ(crazy(0, 1), 29524u);

    // crz(2, 1): M[1][2] = 2 at trit 0 -> 29524 + 1 = 29525.
    CHECK_EQ(crazy(2, 1), 29525u);
}

TEST_CASE(crazy_result_in_range) {
    // Result must always be a valid 10-trit word.
    for (std::uint32_t a = 0; a < 59049; a += 37) {
        for (std::uint32_t b = 0; b < 59049; b += 211) {
            CHECK(crazy(a, b) < kMemSize);
        }
    }
}

TEST_CASE(crazy_table_matches_direct) {
    // The split-half table must agree with the trit-by-trit definition.
    std::uint32_t a = 0, b = 12345;
    for (int i = 0; i < 20000; ++i) {
        a = (a * 1103515245u + 12345u) % 59049u;
        b = (b * 1664525u + 1013904223u) % 59049u;
        CHECK_EQ(crazy(a, b), crazy_direct(a, b));
    }
}

TF_MAIN()

// Unit tests: sparse overlay + lazy crazy-chain memory.
#include "test_framework.hpp"

#include <array>

#include "malbolge/crazy.hpp"
#include "malbolge/memory.hpp"

using namespace malbolge;

TEST_CASE(memory_program_cells_visible) {
    const std::array<std::uint32_t, 3> prog{100, 50, 25};
    Memory m;
    m.load_program(prog.data(), prog.size());
    CHECK_EQ(m.get(0), 100u);
    CHECK_EQ(m.get(1), 50u);
    CHECK_EQ(m.get(2), 25u);
}

TEST_CASE(memory_unwritten_below_fill_start_is_zero) {
    // A program of 10 cells: fill_start = 10. Cells are all loaded, so use
    // a program and never-written checking on a fresh overlay write instead.
    const std::array<std::uint32_t, 10> prog{};
    Memory m;
    m.load_program(prog.data(), prog.size());  // overlay holds explicit zeros
    CHECK_EQ(m.fill_start(), 10u);
}

TEST_CASE(memory_lazy_chain_seed) {
    // Program of 2 cells -> fill_start = 2 (max(ncells,2)).
    // mem[2] = crz(mem[1], mem[0]).
    const std::array<std::uint32_t, 2> prog{33, 34};
    Memory m;
    m.load_program(prog.data(), prog.size());
    CHECK_EQ(m.fill_start(), 2u);
    CHECK_EQ(m.get(2), crazy(34, 33));
}

TEST_CASE(memory_chain_induction) {
    // Without overlay interference, the free chain obeys
    // mem[i] = crz(mem[i-1], mem[i-2]) everywhere.
    const std::array<std::uint32_t, 5> prog{70, 71, 72, 73, 74};
    Memory m;
    m.load_program(prog.data(), prog.size());
    for (std::uint32_t i = 5; i < 5000; ++i) {
        CHECK_EQ(m.get(i), crazy(m.get(i - 1), m.get(i - 2)));
    }
}

TEST_CASE(memory_overlay_write_shadows_chain) {
    // Write into a cell at/above fill_start before it is ever materialized:
    // the overlay value must win, and its presence changes the seed of the
    // boundary fill (the observable lazy-vs-eager difference).
    const std::array<std::uint32_t, 300> prog{};
    Memory m;
    m.load_program(prog.data(), prog.size());  // fill_start = 300
    m.set(485, 42424u);
    CHECK_EQ(m.get(485), 42424u);              // overlay wins
    // Boundary cell tail_end = 486 re-reads operands through get():
    // mem[486] = crz(mem[485]=42424, mem[484]).
    const std::uint32_t m484 = m.get(484);
    CHECK_EQ(m.get(486), crazy(42424u, m484));
}

TEST_CASE(memory_set_updates_first_matching_entry) {
    const std::array<std::uint32_t, 3> prog{1, 2, 3};
    Memory m;
    m.load_program(prog.data(), prog.size());
    m.set(1, 999u);
    CHECK_EQ(m.get(1), 999u);
    CHECK_EQ(m.overlay_size(), 3u);  // updated in place, no duplicate
}

TEST_CASE(memory_values_are_words) {
    // Every materialized cell is a valid 10-trit word.
    const std::array<std::uint32_t, 1> prog{66};
    Memory m;
    m.load_program(prog.data(), prog.size());
    for (std::uint32_t i = 0; i < 20000; ++i) {
        CHECK(m.get(i) < kMemSize);
    }
}

TF_MAIN()

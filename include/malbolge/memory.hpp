// MalbolgeEngineCPP — sparse overlay + lazy crazy-chain memory.
#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "malbolge/constants.hpp"

namespace malbolge {

// Memory model replicated from the reference C engine (see
// docs/REFERENCE_BEHAVIOR.md):
//
//   * The program occupies cells [0, ncells) in a sparse overlay.
//   * Every write during execution lands in the same overlay.
//     The overlay holds at most kMaxOverlay distinct cells; writes beyond
//     that are silently dropped (reference parity).
//   * Cells >= fill_start (max(ncells, 2)) that have never been written are
//     lazily materialized in blocks of kBlockSize following
//     chain[i] = crazy(mem[i-1], mem[i-2]), with the exact seed-handling
//     quirks of the reference implementation (see memory.cpp comments).
//   * Reading a never-written cell < fill_start yields 0.
//
// The lazy chain matters for correctness, not only performance: a program
// that writes into cells at or above its own length changes the seed of a
// later fill, so pre-filling memory eagerly is NOT observably equivalent.
class Memory {
public:
    Memory() = default;

    // Resets the memory and loads `cells` as the program image.
    void load_program(const std::uint32_t* cells, std::size_t ncells);

    std::uint32_t get(std::uint32_t addr);
    void set(std::uint32_t addr, std::uint32_t value);

    std::size_t overlay_size() const { return overlay_.size(); }
    std::uint32_t fill_start() const { return fill_start_; }

    // Test/observability hook: fully materialize a cell range into the
    // chain buffer without touching the overlay (read-only view helper).
    std::uint32_t peek_materialized(std::uint32_t addr) { return get(addr); }

private:
    void ensure_filled(std::uint32_t x);

    // Overlay: (index, value) pairs, first matching update wins on set,
    // linear scan on get — mirrors the reference exactly.
    std::vector<std::pair<std::uint32_t, std::uint32_t>> overlay_;

    // Lazily filled backing chain. Index i is valid once i < chain_until_.
    std::vector<std::uint32_t> chain_ = std::vector<std::uint32_t>(kMemSize, 0);

    std::uint32_t fill_start_  = 0;
    std::uint32_t chain_until_ = 0;
};

}  // namespace malbolge

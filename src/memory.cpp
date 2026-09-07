// MalbolgeEngineCPP — memory: sparse overlay + lazy crazy-chain fill.
//
// This is a line-for-line behavioral port of the reference C engine's
// mem_get/mem_set/ensure_filled, including its quirks:
//
//   1. The overlay is a linear array with first-match update semantics and
//      a hard capacity of kMaxOverlay entries. Writes past the cap are
//      silently dropped.
//   2. chain[i] = crz(mem[i-1], mem[i-2]) is materialized in blocks of
//      kBlockSize cells starting at fill_start = max(ncells, 2).
//   3. The very first fill seeds p1/p2 from the OVERLAY entries of
//      fill_start-1 and fill_start-2 only (defaulting to 0), NOT from a
//      materialized chain.
//   4. Exactly at the two boundary cells i == tail_end and
//      i == tail_end+1 (tail_end = next multiple of kBlockSize after
//      fill_start), the operands are re-read through the full get() path,
//      so overlay writes influence those two cells. All other cells use
//      the running p1/p2 pair.
//   5. Cells below fill_start that were never written read as 0.
#include "malbolge/memory.hpp"

#include "malbolge/crazy.hpp"

namespace malbolge {

void Memory::load_program(const std::uint32_t* cells, std::size_t ncells) {
    overlay_.clear();
    chain_until_ = 0;
    fill_start_  = static_cast<std::uint32_t>(ncells > 2 ? ncells : 2);
    chain_until_ = fill_start_;
    overlay_.reserve(ncells < kMaxOverlay ? ncells : kMaxOverlay);
    for (std::size_t i = 0; i < ncells; ++i) {
        set(static_cast<std::uint32_t>(i), cells[i]);
    }
}

std::uint32_t Memory::get(std::uint32_t x) {
    for (const auto& [idx, val] : overlay_) {
        if (idx == x) return val;
    }
    if (x < fill_start_) return 0;
    if (x >= chain_until_) ensure_filled(x);
    return chain_[x];
}

void Memory::set(std::uint32_t x, std::uint32_t v) {
    for (auto& [idx, val] : overlay_) {
        if (idx == x) {
            val = v;
            return;
        }
    }
    if (overlay_.size() < kMaxOverlay) {
        overlay_.emplace_back(x, v);
    }
    // else: silently dropped — reference parity.
}

void Memory::ensure_filled(std::uint32_t x) {
    const std::uint32_t block_end = (x / kBlockSize + 1) * kBlockSize;
    const std::uint32_t tail_end  = (fill_start_ / kBlockSize + 1) * kBlockSize;

    std::uint32_t p1, p2;
    std::size_t i;

    if (chain_until_ == fill_start_) {
        // First fill: seeds come from the overlay only (default 0).
        p1 = 0;
        p2 = 0;
        for (const auto& [idx, val] : overlay_) {
            if (idx == fill_start_ - 1) p1 = val;
            if (idx == fill_start_ - 2) p2 = val;
        }
        i = fill_start_;
    } else {
        i  = chain_until_;
        p1 = chain_[i - 1];
        p2 = chain_[i - 2];
    }

    while (i < block_end) {
        std::uint32_t nxt;
        if (i == tail_end) {
            nxt = crazy(get(i - 1), get(i - 2));
        } else if (i == tail_end + 1) {
            nxt = crazy(p1, get(i - 2));
        } else {
            nxt = crazy(p1, p2);
        }
        chain_[i] = nxt;
        p2 = p1;
        p1 = nxt;
        ++i;
    }
    chain_until_ = block_end;
}

}  // namespace malbolge

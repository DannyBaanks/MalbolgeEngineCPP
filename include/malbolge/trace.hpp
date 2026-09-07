// MalbolgeEngineCPP — typed, structured execution tracing.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "malbolge/decode.hpp"

namespace malbolge {

// One typed event per executed instruction. All fields are plain values; a
// consumer can serialize them (the CLI emits JSON lines) or aggregate them.
struct TraceEvent {
    std::uint64_t step = 0;

    // State BEFORE the instruction executes.
    std::uint32_t a_before = 0;
    std::uint32_t c_before = 0;
    std::uint32_t d_before = 0;

    std::uint32_t fetched = 0;     // mem[c] before execution
    Opcode        opcode = Opcode::Nop;

    // State AFTER the instruction executes (pre self-encryption write is
    // reflected in the memory_* fields; registers are post-instruction).
    std::uint32_t a_after = 0;
    std::uint32_t c_after = 0;     // after pointer advance
    std::uint32_t d_after = 0;

    // Optional write performed by the instruction itself (rot/crazy write
    // mem[d]).
    struct MemWrite {
        std::uint32_t address = 0;
        std::uint32_t before  = 0;
        std::uint32_t after   = 0;
    };
    std::optional<MemWrite> mem_write;

    std::optional<std::uint8_t>  input_event;   // byte consumed
    std::optional<std::uint8_t>  output_event;  // byte emitted

    bool halted = false;  // instruction ended the run

    std::string to_json() const;
};

// Sink interface. A tracer NEVER mutates VM state; tracing is off by
// default and costs a single null check per step.
class Tracer {
public:
    virtual ~Tracer() = default;
    virtual void on_event(const TraceEvent& ev) = 0;
};

// Collects every event in memory (bounded by the caller's step budget).
class VectorTracer final : public Tracer {
public:
    void on_event(const TraceEvent& ev) override { events_.push_back(ev); }
    const std::vector<TraceEvent>& events() const { return events_; }
    void clear() { events_.clear(); }

private:
    std::vector<TraceEvent> events_;
};

}  // namespace malbolge

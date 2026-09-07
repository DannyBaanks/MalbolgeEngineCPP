// MalbolgeEngineCPP — the embeddable VM.
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "malbolge/memory.hpp"
#include "malbolge/result.hpp"
#include "malbolge/trace.hpp"

namespace malbolge {

// Errors only exist at LOAD time. Execution itself never throws; it stops
// with a StopReason.
struct LoadError {
    std::size_t   position = 0;   // byte offset in the source text
    unsigned char byte     = 0;   // offending byte
};

// Deterministic Classic Malbolge virtual machine.
//
// Construction validates the program. Execution is side-effect free except
// through the VM object itself: input comes from an in-memory buffer, output
// accumulates in a bounded buffer (kOutCap bytes, extra bytes silently
// dropped — reference parity). Reusable: reset() + load again.
class VM {
public:
    VM() = default;
    explicit VM(std::span<const std::uint32_t> cells) { load(cells); }

    // Parse source text: skips ' ', '\t', '\r', '\n'; rejects any other
    // byte outside [33,126] with a LoadError. Mirrors the reference CLI.
    static VM from_source(std::string_view source) {
        VM vm;
        vm.load_source(source);
        return vm;
    }

    void load(std::span<const std::uint32_t> cells);

    // Throws std::invalid_argument carrying a LoadError description on a
    // bad character. (Exception at load time only; run() never throws.)
    void load_source(std::string_view source);

    // Set the byte stream the In opcode consumes. May be re-set later; the
    // read cursor is reset to the new buffer.
    void set_input(std::span<const std::uint8_t> bytes);

    // Execute exactly one instruction (or stop immediately if already
    // finished). Returns false when the machine cannot run further.
    bool step();

    // Execute up to max_steps instructions.
    RunResult run(std::uint64_t max_steps = kDefaultMaxSteps);

    // Attach a tracer (caller keeps ownership). nullptr disables tracing.
    void set_tracer(Tracer* tracer) { tracer_ = tracer; }

    // ----- observers ----------------------------------------------------
    std::uint32_t a() const { return a_; }
    std::uint32_t c() const { return c_; }
    std::uint32_t d() const { return d_; }
    std::uint64_t executed() const { return executed_; }
    bool          stopped() const { return reason_ != StopReason::Running; }
    StopReason    stop_reason() const { return reason_; }

    const std::vector<std::uint8_t>& output() const { return output_; }
    std::string output_text() const {
        return std::string(output_.begin(), output_.end());
    }

    // Read a memory cell (materializes lazily, like a real fetch).
    std::uint32_t mem(std::uint32_t addr) { return memory_.get(addr); }
    const Memory& memory() const { return memory_; }

    // Number of program cells loaded (0 before load).
    std::size_t program_size() const { return program_size_; }

private:
    Memory memory_;

    std::uint32_t a_ = 0;
    std::uint32_t c_ = 0;
    std::uint32_t d_ = 0;

    std::uint64_t executed_ = 0;
    StopReason    reason_   = StopReason::Running;

    std::vector<std::uint8_t> input_;
    std::size_t               input_pos_ = 0;
    std::vector<std::uint8_t> output_;

    std::size_t program_size_ = 0;

    Tracer* tracer_ = nullptr;
};

}  // namespace malbolge

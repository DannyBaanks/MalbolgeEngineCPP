// MalbolgeEngineCPP — VM execution core.
#include "malbolge/vm.hpp"

#include <stdexcept>

#include "malbolge/crazy.hpp"
#include "malbolge/decode.hpp"

namespace malbolge {

const char* to_string(StopReason r) {
    switch (r) {
        case StopReason::Running:      return "RUNNING";
        case StopReason::Halt:         return "HALT";
        case StopReason::InvalidFetch: return "INVALID_FETCH";
        case StopReason::InputEof:     return "INPUT_EOF";
        case StopReason::StepLimit:    return "STEP_LIMIT";
    }
    return "UNKNOWN";
}

void VM::load(std::span<const std::uint32_t> cells) {
    memory_.load_program(cells.data(), cells.size());
    program_size_ = cells.size();
    a_ = c_ = d_ = 0;
    executed_   = 0;
    reason_     = StopReason::Running;
    input_pos_  = 0;
    output_.clear();
}

void VM::load_source(std::string_view source) {
    std::vector<std::uint32_t> cells;
    cells.reserve(source.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        const auto byte = static_cast<unsigned char>(source[i]);
        if (byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n') {
            continue;
        }
        if (!is_fetchable(byte)) {
            throw std::invalid_argument(
                "malbolge: invalid character 0x" +
                std::string(1, "0123456789abcdef"[byte >> 4]) +
                std::string(1, "0123456789abcdef"[byte & 0x0f]) +
                " in program at offset " + std::to_string(i));
        }
        cells.push_back(byte);
    }
    load(cells);
}

void VM::set_input(std::span<const std::uint8_t> bytes) {
    input_.assign(bytes.begin(), bytes.end());
    input_pos_ = 0;
}

bool VM::step() {
    if (stopped()) return false;

    const std::uint32_t ins = memory_.get(c_);
    if (!is_fetchable(ins)) {
        ++executed_;  // reference counts the terminating fetch as a step
        reason_ = StopReason::InvalidFetch;
        return false;
    }

    TraceEvent ev;
    Tracer* tr = tracer_;
    if (tr) {
        ev.step     = executed_ + 1;
        ev.a_before = a_;
        ev.c_before = c_;
        ev.d_before = d_;
        ev.fetched  = ins;
    }

    const Opcode op = decode(ins, c_);
    if (tr) ev.opcode = op;
    ++executed_;

    switch (op) {
        case Opcode::JmpD:
            c_ = memory_.get(d_);
            break;
        case Opcode::Out: {
            const auto byte = static_cast<std::uint8_t>(a_ % 256);
            if (output_.size() < kOutCap) output_.push_back(byte);
            if (tr) ev.output_event = byte;
            break;
        }
        case Opcode::In:
            if (input_pos_ < input_.size()) {
                a_ = input_[input_pos_++];
                if (tr) ev.input_event = a_;
            } else {
                reason_ = StopReason::InputEof;
            }
            break;
        case Opcode::Rot: {
            const std::uint32_t before = memory_.get(d_);
            const std::uint32_t r      = rotate(before);
            memory_.set(d_, r);
            a_ = r;
            if (tr) ev.mem_write = TraceEvent::MemWrite{d_, before, r};
            break;
        }
        case Opcode::MovD:
            d_ = memory_.get(d_);
            break;
        case Opcode::Crazy: {
            const std::uint32_t before = memory_.get(d_);
            const std::uint32_t r      = crazy(a_, before);
            memory_.set(d_, r);
            a_ = r;
            if (tr) ev.mem_write = TraceEvent::MemWrite{d_, before, r};
            break;
        }
        case Opcode::Halt:
            reason_ = StopReason::Halt;
            break;
        case Opcode::Nop:
            break;
    }

    if (stopped()) {
        // The terminating instruction already counted in executed_ (same
        // accounting as the reference engine, which increments per fetch).
        if (tr) {
            ev.halted = true;
            ev.a_after = a_;
            ev.c_after = c_;
            ev.d_after = d_;
            tr->on_event(ev);
        }
        return false;
    }

    // Self-encryption of the just-executed cell (printable cells only),
    // then advance both pointers with wrap-around.
    const std::uint32_t enc = memory_.get(c_);
    if (is_fetchable(enc)) memory_.set(c_, encrypt(enc));
    c_ = (c_ == kLast) ? 0 : c_ + 1;
    d_ = (d_ == kLast) ? 0 : d_ + 1;

    if (tr) {
        ev.a_after = a_;
        ev.c_after = c_;
        ev.d_after = d_;
        tr->on_event(ev);
    }
    return true;
}

RunResult VM::run(std::uint64_t max_steps) {
    // Hitting the step limit does not kill the machine: a subsequent run()
    // resumes exactly where the previous one stopped. Terminal states
    // (halt / invalid fetch / input EOF) are permanent.
    if (reason_ == StopReason::StepLimit) reason_ = StopReason::Running;
    const std::uint64_t start = executed_;
    while (!terminated(reason_)) {
        if (executed_ - start >= max_steps) {
            reason_ = StopReason::StepLimit;
            break;
        }
        step();
    }
    RunResult r;
    r.steps   = executed_;  // cumulative, like the reference's per-run count
    r.reason  = reason_;
    r.final_a = a_;
    r.final_c = c_;
    r.final_d = d_;
    return r;
}

}  // namespace malbolge

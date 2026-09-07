// MalbolgeEngineCPP — typed run results.
#pragma once

#include <cstdint>
#include <string>

namespace malbolge {

// Why a run stopped. The reference engine folds all of these into a single
// binary "terminated" flag plus a step count; we keep the reason as
// additional observability WITHOUT changing observable output/steps.
enum class StopReason {
    Running,       // only produced by step(), never by run()
    Halt,          // 'v' opcode executed
    InvalidFetch,  // mem[c] outside [33,126]
    InputEof,      // input instruction with the input stream exhausted
    StepLimit      // max_steps consumed
};

// Whether the machine terminated before the step limit (reference parity:
// this is exactly RunOutcome.terminated).
inline bool terminated(StopReason r) { return r != StopReason::StepLimit && r != StopReason::Running; }

struct RunResult {
    std::uint64_t steps = 0;
    StopReason    reason = StopReason::Running;
    std::uint32_t final_a = 0;
    std::uint32_t final_c = 0;
    std::uint32_t final_d = 0;
};

const char* to_string(StopReason r);

}  // namespace malbolge

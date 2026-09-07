// Unit tests: the VM end to end, against frozen reference vectors.
#include "test_framework.hpp"

#include <fstream>
#include <iterator>
#include <string>

#include "malbolge/vm.hpp"

using namespace malbolge;

namespace {

std::string hello_source() {
    // Canonical program, ported from the reference engine's examples/.
    return "(=<`#9]~6ZY327Uv4-QsqpMn&+Ij\"'E%e{Ab~w=_:]Kw%o44Uqp0/Q?xNvL:`H%c"
           "#DD2^WV>gY;dts76qKJImZkj";
}

}  // namespace

TEST_CASE(vm_hello_world_vector) {
    // Frozen against the reference C engine: 48 steps, "Hello, world.",
    // terminated (halt opcode executed).
    VM vm = VM::from_source(hello_source());
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 48u);
    CHECK(r.reason != StopReason::StepLimit);
    CHECK(terminated(r.reason));
    CHECK_EQ(vm.output_text(), std::string("Hello, world."));
}

TEST_CASE(vm_minimal_halt) {
    // 'Q' = 81, (81 + 0) % 94 = 81 -> halt after exactly 1 step.
    VM vm = VM::from_source("Q");
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 1u);
    CHECK(r.reason == StopReason::Halt);
    CHECK(vm.output().empty());
}

TEST_CASE(vm_immediate_invalid_fetch) {
    // First byte decodes printable but... use a program whose first cell is
    // printable yet the SECOND cell (unwritten free chain) is not.
    // Simplest: program "c" (out, a=0 -> emits byte 0), then c=1 reads an
    // unwritten cell below fill_start? No: fill_start = max(1,2)=2, so
    // cell 1 was never written -> reads 0 -> invalid fetch at step 2.
    VM vm = VM::from_source("c");
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 2u);
    CHECK(r.reason == StopReason::InvalidFetch);
    CHECK_EQ(vm.output().size(), 1u);
    CHECK_EQ(vm.output()[0], 0u);  // a was 0 -> 0 % 256
}

TEST_CASE(vm_input_eof_terminates) {
    // 'u' = 117, (117 + 0) % 94 = 23 -> In. Empty input => EOF => terminate.
    VM vm = VM::from_source("u");
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 1u);
    CHECK(r.reason == StopReason::InputEof);
}

TEST_CASE(vm_input_consumed) {
    VM vm = VM::from_source("u");
    const std::uint8_t in[] = {'A'};
    vm.set_input(std::span<const std::uint8_t>(in, 1));
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 2u);                    // In, then invalid fetch at c=1
    CHECK_EQ(r.final_a, static_cast<std::uint32_t>('A'));
    CHECK(r.reason == StopReason::InvalidFetch);
}

TEST_CASE(vm_step_limit) {
    VM vm = VM::from_source(hello_source());
    const RunResult r = vm.run(10);
    CHECK_EQ(r.steps, 10u);
    CHECK(r.reason == StopReason::StepLimit);
    // Resuming must continue, not restart.
    const RunResult r2 = vm.run(100);
    CHECK_EQ(r2.steps, 48u);
    CHECK_EQ(vm.output_text(), std::string("Hello, world."));
}

TEST_CASE(vm_step_api_matches_run) {
    VM a = VM::from_source(hello_source());
    while (a.step()) {
    }
    VM b = VM::from_source(hello_source());
    const RunResult rb = b.run();
    CHECK_EQ(a.executed(), rb.steps);
    CHECK_EQ(a.output_text(), b.output_text());
    CHECK_EQ(a.a(), rb.final_a);
    CHECK_EQ(a.c(), rb.final_c);
    CHECK_EQ(a.d(), rb.final_d);
}

TEST_CASE(vm_source_whitespace_and_reject) {
    // Whitespace is skipped everywhere.
    VM vm = VM::from_source(std::string("Q \n\t\r"));
    CHECK_EQ(vm.program_size(), 1u);
    // A non-printable, non-whitespace byte is a load error.
    bool threw = false;
    try {
        VM bad = VM::from_source(std::string("Q\x01", 2));
        (void)bad;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE(vm_self_encryption_happens) {
    // After the first instruction executes, mem[0] must be re-written with
    // ENCRYPT[old - 33] (before the pointer advance).
    VM vm = VM::from_source("*Q");  // '*' = 42 -> (42)%94 = 42 -> Nop
    (void)vm.step();
    CHECK_EQ(vm.mem(0), static_cast<std::uint32_t>(
                            kEncrypt[static_cast<std::uint32_t>('*') - 33]));
}

TF_MAIN()

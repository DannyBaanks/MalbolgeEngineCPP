// Library API ergonomics tests (the same surface the CLI uses).
#include "test_framework.hpp"

#include <array>
#include <string>

#include "malbolge/vm.hpp"

using namespace malbolge;

TEST_CASE(api_construct_from_cells) {
    const std::array<std::uint32_t, 1> prog{static_cast<std::uint32_t>('Q')};
    VM vm(std::span<const std::uint32_t>(prog.data(), prog.size()));
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 1u);
}

TEST_CASE(api_reload_reuses_vm) {
    VM vm = VM::from_source("Q");
    vm.run();
    CHECK(vm.stopped());
    vm.load_source("c");
    const RunResult r = vm.run();
    CHECK_EQ(r.steps, 2u);
    CHECK_EQ(vm.output().size(), 1u);
}

TEST_CASE(api_input_replaceable) {
    VM vm = VM::from_source("u");
    const std::uint8_t in1[] = {'x'};
    vm.set_input(std::span<const std::uint8_t>(in1, 1));
    vm.run();
    CHECK_EQ(vm.a(), static_cast<std::uint32_t>('x'));

    vm.load_source("u");
    const std::uint8_t in2[] = {'y'};
    vm.set_input(std::span<const std::uint8_t>(in2, 1));
    vm.run();
    CHECK_EQ(vm.a(), static_cast<std::uint32_t>('y'));
}

TEST_CASE(api_run_result_text) {
    CHECK_EQ(std::string(to_string(StopReason::Halt)), "HALT");
    CHECK_EQ(std::string(to_string(StopReason::StepLimit)), "STEP_LIMIT");
    CHECK(terminated(StopReason::Halt));
    CHECK(terminated(StopReason::InvalidFetch));
    CHECK(terminated(StopReason::InputEof));
    CHECK(!terminated(StopReason::StepLimit));
    CHECK(!terminated(StopReason::Running));
}

TF_MAIN()

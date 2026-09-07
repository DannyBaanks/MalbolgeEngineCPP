// Unit tests: tracing correctness and non-interference.
#include "test_framework.hpp"

#include "malbolge/trace.hpp"
#include "malbolge/vm.hpp"

using namespace malbolge;

namespace {
const char* kHello =
    "(=<`#9]~6ZY327Uv4-QsqpMn&+Ij\"'E%e{Ab~w=_:]Kw%o44Uqp0/Q?xNvL:`H%c"
    "#DD2^WV>gY;dts76qKJImZkj";
}

TEST_CASE(trace_event_count_matches_steps) {
    VM vm = VM::from_source(kHello);
    VectorTracer tracer;
    vm.set_tracer(&tracer);
    const RunResult r = vm.run();
    CHECK_EQ(tracer.events().size(), static_cast<std::size_t>(r.steps));
    CHECK(tracer.events().back().halted);
}

TEST_CASE(trace_event_fields) {
    VM vm = VM::from_source("Q");
    VectorTracer tracer;
    vm.set_tracer(&tracer);
    vm.run();
    CHECK_EQ(tracer.events().size(), 1u);
    const TraceEvent& e = tracer.events()[0];
    CHECK_EQ(e.step, 1u);
    CHECK_EQ(e.fetched, static_cast<std::uint32_t>('Q'));
    CHECK(e.opcode == Opcode::Halt);
    CHECK(e.a_before == 0u && e.a_after == 0u);
}

TEST_CASE(trace_records_output_events) {
    VM vm = VM::from_source(kHello);
    VectorTracer tracer;
    vm.set_tracer(&tracer);
    vm.run();
    int outputs = 0;
    for (const auto& e : tracer.events()) {
        if (e.output_event) ++outputs;
    }
    CHECK_EQ(outputs, 13);  // "Hello, world." is 13 bytes
}

TEST_CASE(trace_non_interference) {
    // THE P5 verdict test: identical runs with and without a tracer must
    // produce identical observable behavior.
    VM plain = VM::from_source(kHello);
    const RunResult rp = plain.run();

    VM traced = VM::from_source(kHello);
    VectorTracer tracer;
    traced.set_tracer(&tracer);
    const RunResult rt = traced.run();

    CHECK_EQ(rp.steps, rt.steps);
    CHECK(rp.reason == rt.reason);
    CHECK_EQ(rp.final_a, rt.final_a);
    CHECK_EQ(rp.final_c, rt.final_c);
    CHECK_EQ(rp.final_d, rt.final_d);
    CHECK_EQ(plain.output_text(), traced.output_text());
}

TEST_CASE(trace_json_shape) {
    VM vm = VM::from_source("Q");
    VectorTracer tracer;
    vm.set_tracer(&tracer);
    vm.run();
    const std::string j = tracer.events()[0].to_json();
    CHECK(j.find("\"opcode\":\"halt\"") != std::string::npos);
    CHECK(j.find("\"halted\":true") != std::string::npos);
    CHECK(j.front() == '{' && j.back() == '}');
}

TF_MAIN()

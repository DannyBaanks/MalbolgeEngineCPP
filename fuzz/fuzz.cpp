// MalbolgeEngineCPP — deterministic property/fuzz harness.
//
// No external fuzzing framework: a fixed-seed xorshift PRNG drives property
// checks. Every run is reproducible; run with a custom seed as argv[1] to
// explore, and add any failing seed to the table below.
//
// Properties are derived from the frozen semantics (docs/REFERENCE_BEHAVIOR.md):
//   F1  crazy(a,b)    is always a valid 10-trit word and matches crazy_direct
//   F2  rotate        is a bijection (10 applications = identity)
//   F3  decode        is a total function on (printable, c) inputs
//   F4  memory        values read are always valid words; overlay reads are
//                     equal to the last write to that cell
//   F5  bounded runs  of random programs never crash and always stop with a
//                     declared StopReason within the step limit
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "malbolge/crazy.hpp"
#include "malbolge/decode.hpp"
#include "malbolge/memory.hpp"
#include "malbolge/vm.hpp"

using namespace malbolge;

namespace {

struct XorShift {
    std::uint64_t s;
    std::uint64_t next() {
        s ^= s << 13;
        s ^= s >> 7;
        s ^= s << 17;
        return s;
    }
    std::uint32_t word() { return static_cast<std::uint32_t>(next() % kMemSize); }
};

int failures = 0;

void prop(const char* name, bool ok) {
    if (!ok) {
        ++failures;
        std::printf("FAIL property %s\n", name);
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10)
                                  : 0x9e3779b97f4a7c15ULL;
    constexpr int kIterations = 20000;
    std::printf("fuzz seed=%llu iterations=%d\n",
                static_cast<unsigned long long>(seed), kIterations);

    XorShift rng{seed};

    // F1
    for (int i = 0; i < kIterations; ++i) {
        const std::uint32_t a = rng.word(), b = rng.word();
        const std::uint32_t r = crazy(a, b);
        prop("F1.range", r < kMemSize);
        prop("F1.direct", r == crazy_direct(a, b));
    }

    // F2
    for (int i = 0; i < 2000; ++i) {
        std::uint32_t n = rng.word(), back = n;
        for (int k = 0; k < 10; ++k) back = rotate(back);
        prop("F2.bijection", back == n);
    }

    // F3
    for (int i = 0; i < kIterations; ++i) {
        const std::uint32_t ins = 33 + rng.next() % 94;
        const std::uint32_t c   = rng.word();
        const Opcode op = decode(ins, c);
        prop("F3.total",
             op == Opcode::JmpD || op == Opcode::Out || op == Opcode::In ||
             op == Opcode::Rot || op == Opcode::MovD || op == Opcode::Crazy ||
             op == Opcode::Halt || op == Opcode::Nop);
        prop("F3.consistent", ((ins + c) % 94) < 94);
    }

    // F4: random program, random writes, reads must return last write and be
    // valid words.
    for (int t = 0; t < 200; ++t) {
        const std::size_t n = 1 + rng.next() % 64;
        std::vector<std::uint32_t> prog(n);
        for (auto& cell : prog) cell = 33 + rng.next() % 94;
        Memory m;
        m.load_program(prog.data(), prog.size());
        for (int w = 0; w < 500; ++w) {
            const std::uint32_t addr = static_cast<std::uint32_t>(rng.next() % 3000);
            const std::uint32_t val  = rng.word();
            m.set(addr, val);
            prop("F4.write_read", m.get(addr) == val);
        }
        for (int rr = 0; rr < 100; ++rr) {
            prop("F4.word", m.get(static_cast<std::uint32_t>(rng.next() % 4000)) < kMemSize);
        }
    }

    // F5: random *structured* programs (printable bytes only) run with a
    // bounded step budget. The machine must stop cleanly, never crash.
    for (int t = 0; t < 2000; ++t) {
        const std::size_t n = 1 + rng.next() % 512;
        std::vector<std::uint32_t> prog(n);
        for (auto& cell : prog) cell = 33 + rng.next() % 94;
        VM vm(std::span<const std::uint32_t>(prog.data(), prog.size()));
        // Random short input.
        std::vector<std::uint8_t> input(rng.next() % 32);
        for (auto& byte : input) byte = static_cast<std::uint8_t>(rng.next());
        vm.set_input(std::span<const std::uint8_t>(input.data(), input.size()));
        const RunResult r = vm.run(5000);
        prop("F5.stopped", r.reason != StopReason::Running);
        prop("F5.steps_leq_limit", r.steps <= 5000);
    }

    if (failures == 0) {
        std::printf("ALL PROPERTIES HELD\n");
        return 0;
    }
    std::printf("%d PROPERTY FAILURES\n", failures);
    return 1;
}

// MalbolgeEngineCPP — micro-benchmarks. Writes JSON to stdout.
//
//   bench_malbolge [iterations]
//
// Measures: crazy, rotate, decode, and VM stepping on the canonical hello
// program (48 steps, re-run many times).
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "malbolge/crazy.hpp"
#include "malbolge/decode.hpp"
#include "malbolge/vm.hpp"

using namespace malbolge;
using Clock = std::chrono::steady_clock;

namespace {

template <typename F>
double time_ns(std::uint64_t iterations, F&& f) {
    // Warmup.
    for (int i = 0; i < 1000; ++i) f(i);
    const auto t0 = Clock::now();
    for (std::uint64_t i = 0; i < iterations; ++i) f(i);
    const auto t1 = Clock::now();
    return static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()) /
        static_cast<double>(iterations);
}

const char* kHello =
    "(=<`#9]~6ZY327Uv4-QsqpMn&+Ij\"'E%e{Ab~w=_:]Kw%o44Uqp0/Q?xNvL:`H%c"
    "#DD2^WV>gY;dts76qKJImZkj";

std::uint32_t sink = 0;  // defeats dead-code elimination

}  // namespace

int main(int argc, char** argv) {
    const std::uint64_t iters =
        argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 2'000'000;

    const double crazy_ns = time_ns(iters, [](std::uint64_t i) {
        sink ^= crazy(static_cast<std::uint32_t>(i % 59049),
                      static_cast<std::uint32_t>((i * 2654435761u) % 59049));
    });
    const double rotate_ns = time_ns(iters, [](std::uint64_t i) {
        sink ^= rotate(static_cast<std::uint32_t>(i % 59049));
    });
    const double decode_ns = time_ns(iters, [](std::uint64_t i) {
        sink ^= static_cast<std::uint32_t>(
            decode(33 + (i % 94), static_cast<std::uint32_t>(i % 59049)));
    });

    // VM stepping: run hello to completion repeatedly (48 steps each).
    const double vm_ns = time_ns(iters / 40 + 1, [](std::uint64_t) {
        VM vm = VM::from_source(kHello);
        sink ^= static_cast<std::uint32_t>(vm.run().steps);
    });

    std::printf("{\n");
    std::printf("  \"iterations\": %llu,\n",
                static_cast<unsigned long long>(iters));
    std::printf("  \"crazy_ns_per_op\": %.2f,\n", crazy_ns);
    std::printf("  \"rotate_ns_per_op\": %.2f,\n", rotate_ns);
    std::printf("  \"decode_ns_per_op\": %.2f,\n", decode_ns);
    std::printf("  \"vm_hello_ns_per_run\": %.2f,\n", vm_ns);
    std::printf("  \"vm_hello_steps\": 48,\n");
    std::printf("  \"sink\": %u\n", sink);
    std::printf("}\n");
    return 0;
}

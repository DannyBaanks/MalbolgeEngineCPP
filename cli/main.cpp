// MalbolgeEngineCPP — command line interface.
//
//   malbolge run    <file.mal> [--input <file>] [--max-steps N] [--json]
//   malbolge trace  <file.mal> --output <trace.jsonl> [--input <file>] [--max-steps N]
//   malbolge doctor
//   malbolge version
//
// Exit codes: 0 success (even when the program hits its step limit),
//             1 usage error, 2 file or program-load error.
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "malbolge/crazy.hpp"
#include "malbolge/trace.hpp"
#include "malbolge/vm.hpp"

namespace {

constexpr std::string_view kVersion = "0.1.0";

int usage(const char* argv0) {
    std::cerr <<
        "usage:\n"
        "  " << argv0 << " run    <file.mal> [--input <file>] [--max-steps N] [--json]\n"
        "  " << argv0 << " trace  <file.mal> --output <trace.jsonl> [--input <file>] [--max-steps N]\n"
        "  " << argv0 << " doctor\n"
        "  " << argv0 << " version\n";
    return 1;
}

std::optional<std::string> read_file(const std::string& path, bool binary) {
    std::ifstream f(path, binary ? std::ios::binary : std::ios::in);
    if (!f) return std::nullopt;
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
}

std::optional<std::uint64_t> parse_u64(const std::string& s) {
    try {
        return std::stoull(s);
    } catch (...) {
        return std::nullopt;
    }
}

// Tracer that streams JSON lines to a file.
class FileTracer final : public malbolge::Tracer {
public:
    explicit FileTracer(std::ofstream& out) : out_(out) {}
    void on_event(const malbolge::TraceEvent& ev) override {
        out_ << ev.to_json() << '\n';
    }
private:
    std::ofstream& out_;
};

int cmd_run(const std::string& program_path, const std::optional<std::string>& input_path,
            std::uint64_t max_steps, bool as_json) {
    auto src = read_file(program_path, false);
    if (!src) {
        std::cerr << "error: cannot open " << program_path << "\n";
        return 2;
    }

    malbolge::VM vm;
    try {
        vm.load_source(*src);
    } catch (const std::invalid_argument& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 2;
    }

    if (input_path) {
        auto in = read_file(*input_path, true);
        if (!in) {
            std::cerr << "error: cannot open " << *input_path << "\n";
            return 2;
        }
        vm.set_input(std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t*>(in->data()), in->size()));
    }

    const malbolge::RunResult r = vm.run(max_steps);

    if (as_json) {
        std::cout << "{\"steps\":" << r.steps
                  << ",\"status\":\"" << malbolge::to_string(r.reason) << "\""
                  << ",\"final_a\":" << r.final_a
                  << ",\"final_c\":" << r.final_c
                  << ",\"final_d\":" << r.final_d
                  << ",\"output_bytes\":" << vm.output().size() << "}\n";
    } else {
        std::cout << vm.output_text() << "\n";
        std::cerr << "steps: " << r.steps << " | status: "
                  << malbolge::to_string(r.reason) << "\n";
    }
    return 0;
}

int cmd_trace(const std::string& program_path, const std::string& out_path,
              const std::optional<std::string>& input_path, std::uint64_t max_steps) {
    auto src = read_file(program_path, false);
    if (!src) {
        std::cerr << "error: cannot open " << program_path << "\n";
        return 2;
    }
    std::ofstream out(out_path);
    if (!out) {
        std::cerr << "error: cannot write " << out_path << "\n";
        return 2;
    }

    malbolge::VM vm;
    try {
        vm.load_source(*src);
    } catch (const std::invalid_argument& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 2;
    }
    if (input_path) {
        auto in = read_file(*input_path, true);
        if (!in) {
            std::cerr << "error: cannot open " << *input_path << "\n";
            return 2;
        }
        vm.set_input(std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t*>(in->data()), in->size()));
    }

    FileTracer tracer(out);
    vm.set_tracer(&tracer);
    const malbolge::RunResult r = vm.run(max_steps);
    out.flush();

    std::cerr << "steps: " << r.steps << " | status: "
              << malbolge::to_string(r.reason)
              << " | trace: " << out_path << "\n";
    return 0;
}

int cmd_doctor() {
    // Environment self-check: table sanity + reference-vector smoke runs.
    using namespace malbolge;

    // crazy table sanity: crz must stay inside [0, 3^10).
    for (std::uint32_t a : {0u, 1u, 2u, 242u, 243u, 59048u}) {
        for (std::uint32_t b : {0u, 1u, 2u, 242u, 243u, 59048u}) {
            if (crazy(a, b) >= kMemSize) {
                std::cerr << "doctor: crazy out of range\n";
                return 1;
            }
        }
    }
    // Known end-to-end vector: canonical Hello, world. in 48 steps is
    // exercised by the test suite; here we check a minimal halt program.
    // 'Q' = 81, (81 + 0) % 94 = 81 -> halt in exactly 1 step.
    const std::uint32_t halt_program[1] = {static_cast<std::uint32_t>('Q')};
    VM vm(std::span<const std::uint32_t>(halt_program, 1));
    const RunResult r = vm.run();
    if (r.reason != StopReason::Halt || r.steps != 1) {
        std::cerr << "doctor: halt smoke test failed\n";
        return 1;
    }
    std::cout << "malbolge-cpp doctor: OK\n"
              << "  memory:       " << kMemSize << " cells\n"
              << "  output cap:   " << kOutCap << " bytes\n"
              << "  overlay cap:  " << kMaxOverlay << " entries\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) return usage(argv[0]);

    const std::string cmd = argv[1];
    if (cmd == "version") {
        std::cout << "malbolge-cpp " << kVersion << "\n";
        return 0;
    }
    if (cmd == "doctor") return cmd_doctor();

    if (cmd == "run" || cmd == "trace") {
        if (argc < 3) return usage(argv[0]);
        const std::string program = argv[2];

        std::optional<std::string> input, trace_out;
        std::uint64_t max_steps = malbolge::kDefaultMaxSteps;
        bool as_json = false;

        for (int i = 3; i < argc; ++i) {
            const std::string arg = argv[i];
            auto need_value = [&](const char* name) -> std::optional<std::string> {
                if (i + 1 >= argc) {
                    std::cerr << "error: " << name << " needs a value\n";
                    return std::nullopt;
                }
                return std::string(argv[++i]);
            };
            if (arg == "--input") {
                input = need_value("--input");
                if (!input) return 1;
            } else if (arg == "--output") {
                trace_out = need_value("--output");
                if (!trace_out) return 1;
            } else if (arg == "--max-steps") {
                auto v = need_value("--max-steps");
                if (!v) return 1;
                auto n = parse_u64(*v);
                if (!n) {
                    std::cerr << "error: --max-steps must be a number\n";
                    return 1;
                }
                max_steps = *n;  // 0 keeps 0 here; run() then steps zero times
            } else if (arg == "--json") {
                as_json = true;
            } else {
                std::cerr << "error: unknown argument " << arg << "\n";
                return 1;
            }
        }

        if (cmd == "run") return cmd_run(program, input, max_steps, as_json);
        if (!trace_out) {
            std::cerr << "error: trace needs --output <file>\n";
            return 1;
        }
        return cmd_trace(program, *trace_out, input, max_steps);
    }

    return usage(argv[0]);
}

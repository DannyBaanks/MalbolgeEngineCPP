// MalbolgeEngineCPP — minimal in-repo test framework (no dependencies).
#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace tf {

struct Case {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<Case>& registry() {
    static std::vector<Case> cases;
    return cases;
}

inline int& failure_count() {
    static int n = 0;
    return n;
}

inline int& check_count() {
    static int n = 0;
    return n;
}

struct Registrar {
    Registrar(std::string name, std::function<void()> fn) {
        registry().push_back({std::move(name), std::move(fn)});
    }
};

inline int run_all() {
    int failed_cases = 0;
    for (const auto& c : registry()) {
        const int before = failure_count();
        c.fn();
        if (failure_count() == before) {
            std::printf("PASS  %s\n", c.name.c_str());
        } else {
            std::printf("FAIL  %s (%d failed checks)\n", c.name.c_str(),
                        failure_count() - before);
            ++failed_cases;
        }
    }
    std::printf("---\n%d cases, %d failed; %d checks total\n",
                static_cast<int>(registry().size()), failed_cases, check_count());
    return failed_cases == 0 ? 0 : 1;
}

}  // namespace tf

#define TEST_CASE(name)                                        \
    void tc_##name();                                          \
    static ::tf::Registrar reg_##name(#name, tc_##name);       \
    void tc_##name()

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++::tf::check_count();                                            \
        if (!(cond)) {                                                    \
            ++::tf::failure_count();                                      \
            std::printf("    check failed %s:%d: %s\n", __FILE__,         \
                        __LINE__, #cond);                                 \
        }                                                                 \
    } while (0)

#define CHECK_EQ(a, b)                                                        \
    do {                                                                      \
        ++::tf::check_count();                                                \
        const auto va = (a);                                                  \
        const auto vb = (b);                                                  \
        if (!(va == vb)) {                                                    \
            ++::tf::failure_count();                                          \
            std::printf("    check failed %s:%d: %s == %s\n", __FILE__,       \
                        __LINE__, #a, #b);                                    \
        }                                                                     \
    } while (0)

#define TF_MAIN()                 \
    int main() { return ::tf::run_all(); }

// MalbolgeEngineCPP — TraceEvent JSON serialization.
#include "malbolge/trace.hpp"

#include <string>
#include <type_traits>

namespace malbolge {

namespace {

const char* opcode_name(Opcode op) {
    switch (op) {
        case Opcode::JmpD:  return "jmpd";
        case Opcode::Out:   return "out";
        case Opcode::In:    return "in";
        case Opcode::Rot:   return "rot";
        case Opcode::MovD:  return "movd";
        case Opcode::Crazy: return "crazy";
        case Opcode::Halt:  return "halt";
        case Opcode::Nop:   return "nop";
    }
    return "?";
}

}  // namespace

std::string TraceEvent::to_json() const {
    std::string j = "{";
    auto add = [&j](const char* key, auto&& value, bool first = false) {
        j += first ? "\"" : ",\"";
        j += key;
        j += "\":";
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            j += '"' + value + '"';
        } else if constexpr (std::is_same_v<T, bool>) {
            j += value ? "true" : "false";
        } else {
            j += std::to_string(value);
        }
    };

    add("step", step, true);
    add("a_before", a_before);
    add("c_before", c_before);
    add("d_before", d_before);
    add("fetched", fetched);
    add("opcode", std::string(opcode_name(opcode)));
    add("a_after", a_after);
    add("c_after", c_after);
    add("d_after", d_after);

    if (mem_write) {
        j += ",\"mem_write\":{\"address\":" + std::to_string(mem_write->address) +
             ",\"before\":" + std::to_string(mem_write->before) +
             ",\"after\":" + std::to_string(mem_write->after) + "}";
    }
    if (input_event)  j += ",\"input\":" + std::to_string(*input_event);
    if (output_event) j += ",\"output\":" + std::to_string(*output_event);
    add("halted", halted);
    j += "}";
    return j;
}

}  // namespace malbolge

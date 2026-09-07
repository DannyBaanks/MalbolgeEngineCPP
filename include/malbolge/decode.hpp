// MalbolgeEngineCPP — instruction decode.
#pragma once

#include <cstdint>

#include "malbolge/constants.hpp"

namespace malbolge {

// The eight opcodes of Classic Malbolge, identified by (ins + c) % 94.
// NoClass covers the remaining 86 decode values that all behave as NOP.
enum class Opcode : std::uint8_t {
    JmpD  = 4,   // c = mem[d]
    Out   = 5,   // output a % 256
    In    = 23,  // a = next input byte; EOF terminates the machine
    Rot   = 39,  // r = rotate(mem[d]); mem[d] = r; a = r
    MovD  = 40,  // d = mem[d]
    Crazy = 62,  // r = crazy(a, mem[d]); mem[d] = r; a = r
    Halt  = 81,  // terminate
    Nop   = 255  // any other decode value
};

// Fetch is only valid for printable bytes; anything else terminates the VM.
inline bool is_fetchable(std::uint32_t cell) {
    return cell >= kCharMin && cell <= kCharMax;
}

inline Opcode decode(std::uint32_t ins, std::uint32_t c) {
    switch ((ins + c) % 94) {
        case 4:  return Opcode::JmpD;
        case 5:  return Opcode::Out;
        case 23: return Opcode::In;
        case 39: return Opcode::Rot;
        case 40: return Opcode::MovD;
        case 62: return Opcode::Crazy;
        case 81: return Opcode::Halt;
        default: return Opcode::Nop;
    }
}

// One-trit right rotate: rotate(n) = 19683 * (n % 3) + n / 3.
inline std::uint32_t rotate(std::uint32_t n) {
    return kPow9 * (n % 3) + n / 3;
}

// Self-modification applied to the just-executed cell when it is printable.
inline std::uint32_t encrypt(std::uint32_t cell) {
    return static_cast<std::uint32_t>(kEncrypt[cell - kCharMin]);
}

}  // namespace malbolge

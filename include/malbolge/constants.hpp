// MalbolgeEngineCPP — constants frozen against the reference C engine
// (DannyBaanks/Malbolge-Engine, MIT). See docs/REFERENCE_BEHAVIOR.md.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace malbolge {

// Classic Malbolge holds exactly 3^10 = 59049 ten-trit words.
inline constexpr std::uint32_t kMemSize      = 59049;
inline constexpr std::uint32_t kLast         = kMemSize - 1;
inline constexpr std::uint32_t kPow9         = 19683;  // 3^9, used by rotate
inline constexpr std::uint32_t kHalf         = 243;    // 3^5, half-word for crazy tables
inline constexpr std::uint32_t kBlockSize    = 243;    // lazy-fill block granularity

// Reference-engine compatibility limits (observable behavior:
// reads/writes past these bounds are silently dropped there, and are
// silently dropped here as well — documented in docs/REFERENCE_BEHAVIOR.md).
inline constexpr std::size_t   kMaxOverlay   = 200'000;
inline constexpr std::size_t   kOutCap       = 65'536;

inline constexpr std::uint64_t kDefaultMaxSteps = 100'000'000;

// Printable program-memory window. Anything outside [33,126] fetched as an
// instruction terminates the machine.
inline constexpr std::uint32_t kCharMin = 33;
inline constexpr std::uint32_t kCharMax = 126;

// Post-execution self-encryption table. POSITIONS 0..93 correspond to
// character values 33..126. entry 94 is the NUL terminator of the C string.
inline constexpr std::array<char, 95> kEncrypt = {
    '5', 'z', ']', '&', 'g', 'q', 't', 'y', 'f', 'r', '$', '(', 'w', 'e', '4',
    '{', 'W', 'P', ')', 'H', '-', 'Z', 'n', ',', '[', '%', '\\', '3', 'd',
    'L', '+', 'Q', ';', '>', 'U', '!', 'p', 'J', 'S', '7', '2', 'F', 'h',
    'O', 'A', '1', 'C', 'B', '6', 'v', '^', '=', 'I', '_', '0', '/', '8',
    '|', 'j', 's', 'b', '9', 'm', '<', '.', 'T', 'V', 'a', 'c', '`', 'u',
    'Y', '*', 'M', 'K', '\'', 'X', '~', 'x', 'D', 'l', '}', 'R', 'E', 'o',
    'k', 'N', ':', '#', '?', 'G', '"', 'i', '@', '\0'};

}  // namespace malbolge

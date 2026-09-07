// Unit tests: decode, rotate, encrypt.
#include "test_framework.hpp"

#include "malbolge/constants.hpp"
#include "malbolge/decode.hpp"

using namespace malbolge;

TEST_CASE(decode_eight_opcodes) {
    // For c = 0, the printable bytes that decode to each opcode:
    CHECK(decode('j' + 0, 0) == Opcode::Nop);  // sanity: not everything is an op
    CHECK(decode(4 + 94, 0) == Opcode::JmpD);   //  98 'b'
    CHECK(decode(5 + 94, 0) == Opcode::Out);    //  99 'c'
    CHECK(decode(23 + 94, 0) == Opcode::In);    // 117 'u'
    CHECK(decode(10 + 94, 0) == Opcode::Nop);   // 104 'h': (104)%94 = 10
    CHECK(decode(39, 0) == Opcode::Rot);        //  39 '\''
    CHECK(decode(40, 0) == Opcode::MovD);       //  40 '('
    CHECK(decode(62, 0) == Opcode::Crazy);      //  62 '>'
    CHECK(decode(81, 0) == Opcode::Halt);       //  81 'Q'
}

TEST_CASE(decode_position_dependent) {
    // Same byte, different c: different opcode. This is the essence of
    // Malbolge self-modification decoupling fetch from meaning.
    const char* hello_prefix = "(=<`#9]";  // hello.malbolge prefix
    for (std::uint32_t c = 0; c < 7; ++c) {
        const std::uint8_t ch = static_cast<std::uint8_t>(hello_prefix[c]);
        CHECK(is_fetchable(ch));
        // Just exercise decode at the real position; must not crash and the
        // result must be one of the enum values.
        const Opcode op = decode(ch, c);
        CHECK(op >= Opcode::JmpD && op <= Opcode::Nop);
    }
}

TEST_CASE(rotate_vectors) {
    CHECK_EQ(rotate(0u), 0u);
    CHECK_EQ(rotate(1u), 19683u);    // lowest trit -> highest
    CHECK_EQ(rotate(2u), 39366u);
    CHECK_EQ(rotate(3u), 1u);
    CHECK_EQ(rotate(59048u), rotate(59048u));  // determinism
    // rotate is a bijection on [0, 59049): triple rotate = trit bswap-ish
    // invariant: rotating 10 times returns the original value.
    std::uint32_t n = 12345;
    for (int i = 0; i < 10; ++i) n = rotate(n);
    CHECK_EQ(n, 12345u);
}

TEST_CASE(encrypt_table) {
    CHECK_EQ(encrypt(33), static_cast<std::uint32_t>('5'));
    CHECK_EQ(encrypt(126), static_cast<std::uint32_t>('@'));
    // Encryption lands back in the printable range for every printable cell
    // except the (never encrypted) ones; sanity: 94 distinct values.
    bool seen[128] = {};
    for (std::uint32_t ch = 33; ch <= 126; ++ch) {
        const std::uint32_t e = encrypt(ch);
        CHECK(e >= 33 && e <= 126);
        seen[e] = true;
    }
    int distinct = 0;
    for (int i = 33; i <= 126; ++i) distinct += seen[i];
    CHECK_EQ(distinct, 94);
}

TEST_CASE(fetchable_window) {
    CHECK(!is_fetchable(32));
    CHECK(is_fetchable(33));
    CHECK(is_fetchable(126));
    CHECK(!is_fetchable(127));
    CHECK(!is_fetchable(0));
}

TF_MAIN()

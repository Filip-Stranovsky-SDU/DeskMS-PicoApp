#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "../tls_client/tls_payloads.h"

static int tests_failed = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAILED: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
            tests_failed++; \
        } \
    } while (0)

/* ---------- PONG ---------- */

static void check_text_payload_masked(
    const uint8_t *buf,
    size_t payload_len,
    size_t payload_offset
) {
    uint8_t mask[4] = {1,2,3,4};

    for (size_t i = 0; i < payload_len; i++) {
        CHECK(buf[payload_offset + i] ==
              ((uint8_t)'A' ^ mask[i % 4]));
    }
}






void test_pong_opcode(void) {
    uint8_t buf[32];
    uint8_t payload[1] = {0};
    ws_build_pong_frame(buf, payload, 1);
    CHECK(buf[0] == 0x8A);
}

void test_pong_length(void) {
    uint8_t buf[32];
    uint8_t payload[5] = {0};
    size_t n = ws_build_pong_frame(buf, payload, 5);
    CHECK(n == 11);
}

void test_pong_mask_bit_set(void) {
    uint8_t buf[32];
    uint8_t payload[1] = {0};
    ws_build_pong_frame(buf, payload, 1);
    CHECK(buf[1] & 0x80);
}

void test_pong_payload_masked(void) {
    uint8_t buf[32];
    uint8_t payload[1] = {0xFF};
    ws_build_pong_frame(buf, payload, 1);
    CHECK(buf[6] != payload[0]);
}

/* ---------- TEXT ---------- */

void test_text_opcode(void) {
    uint8_t buf[64];
    ws_build_text_frame(buf, "hi");
    CHECK(buf[0] == 0x81);
}

void test_text_short_length(void) {
    uint8_t buf[64];
    ws_build_text_frame(buf, "hi");
    CHECK((buf[1] & 0x7F) == 2);
}

void test_text_extended_length(void) {
    char msg[130];
    memset(msg, 'A', 129);
    msg[129] = 0;

    uint8_t buf[256];
    ws_build_text_frame(buf, msg);
    CHECK((buf[1] & 0x7F) == 126);
}

void test_text_mask_bit_set(void) {
    uint8_t buf[64];
    ws_build_text_frame(buf, "hi");
    CHECK(buf[1] & 0x80);
}

void test_text_empty_message(void) {
    uint8_t buf[32];
    ws_build_text_frame(buf, "");
    CHECK((buf[1] & 0x7F) == 0);
}

void test_text_len_one(void) {
    uint8_t buf[32];
    ws_build_text_frame(buf, "A");
    CHECK((buf[1] & 0x7F) == 1);
}

void test_text_frame_literal(void) {
    const char *msg = "XDDDDDDD";
    uint8_t buf[64];

    // Build the actual frame
    size_t frame_len = ws_build_text_frame(buf, msg);

    // Build the expected frame literally
    uint8_t expected[] = {
        0x81,           // FIN + text
        0x80 | 8,       // MASK + payload length
        1, 2, 3, 4,     // mask bytes
        'X' ^ 1,        // masked payload
        'D' ^ 2,
        'D' ^ 3,
        'D' ^ 4,
        'D' ^ 1,
        'D' ^ 2,
        'D' ^ 3,
        'D' ^ 4
    };

    // Compare actual vs expected
    CHECK(frame_len == sizeof(expected));
    CHECK(memcmp(buf, expected, sizeof(expected)) == 0);
}

void test_text_frame_extra_long(void) {
    // Build a message longer than 65535
    static char msg[65536 + 10];  // 65546 bytes
    for (size_t i = 0; i < sizeof(msg) - 1; i++) {
        msg[i] = 'A';
    }
    msg[sizeof(msg) - 1] = 0;

    uint8_t buf[70000];

    size_t frame_len = ws_build_text_frame(buf, msg);

    // This should fail (function returns 0 for too long input)
    CHECK(frame_len > 0);
}
/* ---------- CLOSE ---------- */

void test_close_opcode(void) {
    uint8_t buf[16];
    ws_build_close_frame(buf);
    CHECK(buf[0] == 0x88);
}

void test_close_length(void) {
    uint8_t buf[16];
    size_t n = ws_build_close_frame(buf);
    CHECK(n == 8);
}

/* ---------- MAIN ---------- */

int main(void) {
    static const int NUMBER_OF_TESTS = 4 + 8 + 2;
    test_pong_opcode();
    test_pong_length();
    test_pong_mask_bit_set();
    test_pong_payload_masked();

    test_text_opcode();
    test_text_short_length();
    test_text_extended_length();
    test_text_mask_bit_set();
    test_text_frame_literal();
    test_text_empty_message();
    test_text_len_one();
    test_text_frame_extra_long();

    test_close_opcode();
    test_close_length();


    if (tests_failed == 0) {
        printf("All unit tests passed\n");
        return 0;
    } else {
        printf("%d tests failed\n", tests_failed);
        printf("%d tests passed\n", NUMBER_OF_TESTS - tests_failed);
        return 1;
    }
    return 0;
}

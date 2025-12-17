#include "tls_payloads.h"

#include <string.h>

size_t ws_build_pong_frame(uint8_t *out,
                        const uint8_t *payload,
                        uint8_t len ) {
    if (len > 125) len = 125;

    uint8_t mask[4] = {0x12,0x34,0x56,0x78};

    out[0] = 0x8A;
    out[1] = 0x80 | len;

    memcpy(&out[2], mask, 4);

    for (uint8_t i = 0; i < len; i++) {
        out[6 + i] = payload[i] ^ mask[i % 4];
    }

    return 6 + len;
}



size_t ws_build_text_frame(uint8_t *out, const char *msg)
{
    size_t len = strlen(msg);
    if (len > 65535) return 0;

    size_t offset = 0;
    out[offset++] = 0x81;

    if (len <= 125) {
        out[offset++] = 0x80 | (uint8_t)len;
    } else {
        out[offset++] = 0x80 | 126;
        out[offset++] = (len >> 8) & 0xFF;
        out[offset++] = len & 0xFF;
    }

    uint8_t mask[4] = {1,2,3,4};
    memcpy(&out[offset], mask, 4);
    offset += 4;

    for (size_t i = 0; i < len; i++) {
        out[offset + i] = msg[i] ^ mask[i % 4];
    }

    return offset + len;
}


size_t ws_build_close_frame(uint8_t *out)
{
    out[0] = 0x88;
    out[1] = 0x80 | 2;

    uint8_t key[4] = {5,6,7,8};
    memcpy(&out[2], key, 4);

    uint16_t code = 1000;
    uint8_t *p = (uint8_t*)&code;

    out[6] = p[1] ^ key[0];
    out[7] = p[0] ^ key[1];

    return 8;
}

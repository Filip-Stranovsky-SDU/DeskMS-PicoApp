#pragma once

#include <stdint.h>
#include <stddef.h>


size_t ws_build_pong_frame(uint8_t *out, const uint8_t *payload, uint8_t len );

size_t ws_build_text_frame(uint8_t *out, const char *msg);

size_t ws_build_close_frame(uint8_t *out);
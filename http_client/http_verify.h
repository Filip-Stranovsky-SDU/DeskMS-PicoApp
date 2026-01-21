/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Using this url as we know the root cert won't change for a long time
#define HTTP_CLIENT_SERVER "10.203.203.100"

//#define TLS_CLIENT_SERVER "fw-download-alias1.raspberrypi.com"
#define HTTP_CLIENT_HTTP_REQUEST  "GET / HTTP/1.1\r\n" \
                                 "Host: " TLS_CLIENT_SERVER "\r\n" \
                                 "Upgrade: websocket\r\n" \
                                 "Connection: Upgrade\r\n" \
                                 "Sec-WebSocket-Key: %s\r\n" \
                                 "Sec-WebSocket-Version: 13\r\n" \
                                 "\r\n"

//#define TLS_CLIENT_HTTP_REQUEST  "GET Hello from pico\r\n" \
                                 "\r\n"
#define HTTP_CLIENT_TIMEOUT_SECS  15


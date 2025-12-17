/*
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "tls_common.h"

#include "wsparsing.hpp"

#include "tls_payloads.h"

struct altcp_tls_config *tls_config = NULL;  // actual definition + initialization

void ws_send_pong(struct altcp_pcb *pcb,
                  const uint8_t *payload,
                  uint8_t len)
{
    uint8_t frame[2 + 4 + 125];
    size_t n = ws_build_pong_frame(frame, payload, len);
    altcp_write(pcb, frame, n, TCP_WRITE_FLAG_COPY);
    altcp_output(pcb);
}





void ws_send_text(struct altcp_pcb *pcb, const char *msg)
{
    size_t len = strlen(msg);
    
    // Limit for stack buffer (adjust as needed)
    if (len > 65535) {
        // too long for this simple implementation
        return;
    }

    // Calculate header size
    size_t header_len = 2; // basic header
    if (len >= 126 && len <= 65535) header_len += 2; // 16-bit extended length
    // Note: not handling 64-bit length here for simplicity

    size_t total_len = header_len + 4 + len; // 4 bytes for mask
    uint8_t frame[total_len]; // stack buffer

    size_t offset = 0;
    frame[offset++] = 0x81; // FIN + text frame

    // Mask bit = 1
    if (len <= 125) {
        frame[offset++] = 0x80 | (uint8_t)len;
    } else {
        frame[offset++] = 0x80 | 126;
        frame[offset++] = (len >> 8) & 0xFF;
        frame[offset++] = len & 0xFF;
    }
    
    // Random mask key (example, can be random)
    uint8_t mask[4] = {1,2,3,4};
    memcpy(&frame[offset], mask, 4);
    offset += 4;

    // Apply mask to payload
    for (size_t i = 0; i < len; i++) {
        frame[offset + i] = msg[i] ^ mask[i % 4];
    }

    // Write frame
    altcp_write(pcb, frame, offset + len, TCP_WRITE_FLAG_COPY);
    altcp_output(pcb);
}

void ws_send_close(struct altcp_pcb *pcb)
{
    uint8_t frame[8];
    printf("ws_send_close\n");
    frame[0] = 0x88;          // FIN + CLOSE opcode
    frame[1] = 0x80 | 2;      // MASK + payload length = 2 bytes

    uint8_t key[4] = {5,6,7,8};
    memcpy(&frame[2], key, 4);

    uint16_t code = 1000;     // normal close
    uint8_t *p = (uint8_t*)&code;

    frame[6] = p[1] ^ key[0]; // network order: high byte first
    frame[7] = p[0] ^ key[1];

    altcp_write(pcb, frame, 8, TCP_WRITE_FLAG_COPY);
    altcp_output(pcb);
}



err_t tls_client_close(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);

        ws_send_close(state->pcb);
        altcp_shutdown(state->pcb, 0, 1);
        altcp_output(state->pcb);

        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending request\n");
    err = altcp_write(state->pcb, state->http_request, strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }
    // ws_send_text(pcb, "hello world");


    return ERR_OK;
}

err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("timed out WS\n");
    state->error = PICO_ERROR_TIMEOUT;
    return tls_client_close(arg);
}

void tls_client_err(void *arg, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("tls_client_err %d\n", err);
    tls_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (!p) {
        printf("connection closed\n");
        // ws_send_close(pcb);
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {
        // Cap stack buffer to something reasonable, e.g., 4 KB
        if (p->tot_len > 4096) {
            printf("Payload too large for stack\n");
            pbuf_free(p);
            return ERR_MEM;
        }

        char buf[p->tot_len + 1];
        pbuf_copy_partial(p, buf, p->tot_len, 0);
        if (p->tot_len < 2) {
            pbuf_free(p);
            return ERR_OK;
        }
        buf[p->tot_len] = 0;

        // Minimal WebSocket parse
        size_t header_len = 2;
        uint8_t opcode = buf[0] & 0x0F;
        uint64_t payload_len = buf[1] & 0x7F;

        // Extended payload length (16-bit)
        if (payload_len == 126) {
            payload_len = (buf[2] << 8) | buf[3];
            header_len += 2;
        }

        // Mask (client-to-server)
        uint8_t mask[4] = {0};
        if (buf[1] & 0x80) {
            for (int i = 0; i < 4; i++) mask[i] = buf[header_len + i];
            header_len += 4;
        }

        // Payload start
        buf[p->tot_len] = '\0';
        uint8_t *payload = (uint8_t*)&buf[header_len];

        // Unmask
        for (uint64_t i = 0; i < payload_len; i++) {
            payload[i] ^= mask[i % 4];
        }

        if (opcode == 0x1) { // text
            printf("WS text: %.*s\n", (int)payload_len, payload);
            int res = handle_ws_message(payload);
        } else if (opcode == 0x9) { // PING
            printf("WS ping received, sending pong\n");
            ws_send_pong(pcb, payload, payload_len);
        // } else if (opcode == 0x8) { // CLOSE
        //     printf("WS close received\n");
        //     // ws_send_close(pcb);
        //     pbuf_free(p);
        //     return tls_client_close(state); 
        } else {
            printf("Non-text frame opcode=%d len=%llu\n", opcode, payload_len);
        }

        altcp_recved(pcb, p->tot_len);
    }

    pbuf_free(p);
    return ERR_OK;
}

void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;
    u16_t port = TLS_SERVER_PORT;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}


bool tls_client_open(const char *hostname, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;

    // state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY); // TLS VERSION
    state->pcb = altcp_new(NULL);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }
    
    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, state->timeout * 2);
    altcp_recv(state->pcb, tls_client_recv);
    
    altcp_err(state->pcb, tls_client_err);

    /* Set SNI */
    // mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname); // FOR TLS

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
TLS_CLIENT_T* tls_client_init(void) {
    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}

bool run_tls_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout) {

    /* No CA certificate checking */
    tls_config = altcp_tls_create_config_client(cert, cert_len);
    assert(tls_config);


    //mbedtls_ssl_conf_authmode(&tls_config->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        return false;
    }
    state->http_request = request;
    state->timeout = timeout;
    
    if (!tls_client_open(server, state)) {
        return false;
    }
    while(!state->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    int err = state->error;
    free(state);
    altcp_tls_free_config(tls_config);
    return err == 0;
}
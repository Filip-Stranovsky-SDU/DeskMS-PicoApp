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

#include "http_common.h"

#include "tls_common.h"

struct altcp_tls_config *http_config = NULL;  // actual definition + initialization


// void ws_send_text(struct altcp_pcb *pcb, const char *msg)
// {
//     uint8_t frame[128];
//     size_t len = strlen(msg);

//     frame[0] = 0x81; // FIN + text frame
//     frame[1] = 0x80 | len; // MASK bit + payload length

//     uint8_t key[4] = {1,2,3,4}; // any random mask
//     memcpy(&frame[2], key, 4);

//     for (size_t i = 0; i < len; i++)
//         frame[6 + i] = msg[i] ^ key[i % 4];

//     altcp_write(pcb, frame, 6 + len, TCP_WRITE_FLAG_COPY);
//     altcp_output(pcb);
// }

// void ws_send_close(struct altcp_pcb *pcb)
// {
//     uint8_t frame[8];

//     frame[0] = 0x88;          // FIN + CLOSE opcode
//     frame[1] = 0x80 | 2;      // MASK + payload length = 2 bytes

//     uint8_t key[4] = {5,6,7,8};
//     memcpy(&frame[2], key, 4);

//     uint16_t code = 1000;     // normal close
//     uint8_t *p = (uint8_t*)&code;

//     frame[6] = p[1] ^ key[0]; // network order: high byte first
//     frame[7] = p[0] ^ key[1];

//     altcp_write(pcb, frame, 8, TCP_WRITE_FLAG_COPY);
//     altcp_output(pcb);
// }



err_t http_client_close(void *arg) {
    http_CLIENT_T *state = (http_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);

        //ws_send_close(state->pcb);
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

err_t http_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    http_CLIENT_T *state = (http_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return http_client_close(state);
    }

    printf("connected to server, sending request\n");
    err = altcp_write(state->pcb, state->http_request, strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return http_client_close(state);
    }


    return ERR_OK;
}

err_t http_client_poll(void *arg, struct altcp_pcb *pcb) {
    http_CLIENT_T *state = (http_CLIENT_T*)arg;
    printf("timed out\n");
    state->error = PICO_ERROR_TIMEOUT;
    return http_client_close(arg);
}

void http_client_err(void *arg, err_t err) {
    http_CLIENT_T *state = (http_CLIENT_T*)arg;
    printf("http_client_err %d\n", err);
    http_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

err_t http_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err)
{
    http_CLIENT_T *state = (http_CLIENT_T *)arg;

    /* 1. Error = pcb is dead */
    if (err != ERR_OK) {
        state->pcb = NULL;
        state->complete = true;
        return err;
    }

    /* 2. FIN from peer (rare with HTTP/1.1) */
    if (p == NULL) {
        state->complete = true;
        return ERR_OK;
    }

    /* 3. Copy TCP payload */
    size_t len = p->tot_len;
    char tmp[512];                    // fixed buffer, not VLA
    if (len > sizeof(tmp)) len = sizeof(tmp);

    pbuf_copy_partial(p, tmp, len, 0);

    size_t offset = 0;

    /* 4. Header parsing */
    if (!state->headers_done) {
        char *hdr_end = NULL;
        for (size_t i = 0; i + 3 < len; i++) {
            if (tmp[i] == '\r' && tmp[i+1] == '\n' &&
                tmp[i+2] == '\r' && tmp[i+3] == '\n') {
                hdr_end = &tmp[i + 4];
                offset = (hdr_end - tmp);
                break;
            }
        }

        if (hdr_end) {
            /* parse Content-Length */
            char *cl = strstr(tmp, "Content-Length:");
            if (cl) {
                state->expected_len = atoi(cl + 15);
            }

            state->headers_done = true;
        } else {
            /* headers not complete yet */
            goto out;
        }
    }

    /* 5. Append body data */
    if (state->headers_done && offset < len) {
        size_t body_part = len - offset;

        if (state->body_len + body_part <= sizeof(state->body)) {
            memcpy(state->body + state->body_len,
                   tmp + offset,
                   body_part);
            state->body_len += body_part;
        }
    }

    /* 6. Done? */
    if (state->expected_len &&
        state->body_len >= state->expected_len) {

        state->complete = true;
        altcp_close(pcb);   // YOU close, not the server
    }

out:
    altcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}


void http_client_connect_to_server_ip(const ip_addr_t *ipaddr, http_CLIENT_T *state)
{
    err_t err;
    u16_t port = http_SERVER_PORT;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
    err = altcp_connect(state->pcb, ipaddr, port, http_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        http_client_close(state);
    }
}

void http_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        http_client_connect_to_server_ip(ipaddr, (http_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        http_client_close(arg);
    }
}


bool http_client_open(const char *hostname, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    http_CLIENT_T *state = (http_CLIENT_T*)arg;

    state->pcb = altcp_new(NULL);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }
    
    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, http_client_poll, state->timeout * 2);
    altcp_recv(state->pcb, http_client_recv);
    
    altcp_err(state->pcb, http_client_err);

    /* Set SNI */
    // mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err = dns_gethostbyname(hostname, &server_ip, http_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        http_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        http_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
http_CLIENT_T* http_client_init(void) {
    http_CLIENT_T *state = calloc(1, sizeof(http_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}

bool run_http_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout) {

    /* No CA certificate checking */
    http_config = altcp_tls_create_config_client(cert, cert_len);
    assert(http_config);


    //mbedtls_ssl_conf_authmode(&tls_config->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);

    http_CLIENT_T *state = http_client_init();
    if (!state) {
        return false;
    }
    state->http_request = request;
    state->timeout = timeout;
    
    if (!http_client_open(server, state)) {
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
    altcp_tls_free_config(http_config);
    return err == 0;
}
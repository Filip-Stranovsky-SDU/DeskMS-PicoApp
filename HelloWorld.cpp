#include <stdio.h>
#include "pico/stdlib.h"

#include "lwip/apps/sntp.h"
#include "hardware/rtc.h"


#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"


#include "tls_verify.h"
#include "http_verify.h"


extern "C" {
    #include "tls_common.h"
    #include "http_common.h"
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "PushButton.hpp"
#include "wsparsing.hpp"

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void generate_ws_key(char *out) {
    uint8_t bytes[16];
    for (int i = 0; i < 16; i++) bytes[i] = rand() & 0xFF;

    for (int i = 0; i < 16; i += 3) {
        uint32_t val = bytes[i] << 16;
        if (i+1 < 16) val |= bytes[i+1] << 8;
        if (i+2 < 16) val |= bytes[i+2];

        out[0] = b64[(val >> 18) & 0x3F];
        out[1] = b64[(val >> 12) & 0x3F];
        out[2] = (i+1 < 16) ? b64[(val >> 6) & 0x3F] : '=';
        out[3] = (i+2 < 16) ? b64[val & 0x3F] : '=';
        out += 4;
    }
    out[24] = '\0';
}





int main()
{
    stdio_init_all();
    srand(time(NULL));

   

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect\n");
        return 1;
    }
    rtc_init();   // <-- required

    datetime_t dt;
    rtc_get_datetime(&dt);
    dt.year = 2025;
    dt.month = 11;
    dt.day = 30;
    dt.hour = 16;
    dt.min = 2;
    dt.sec = 0;
    rtc_set_datetime(&dt);
    // sntp_setoperatingmode(SNTP_OPMODE_POLL);
    // sntp_setservername(0, "pool.ntp.org");
    // sntp_init();
    //if(true){

    //const char *server = HTTP_CLIENT_SERVER;

    // char request[256];
    // snprintf(request, sizeof(request),
    //          "GET / HTTP/1.1\r\n"
    //          "Host: %s\r\n"
    //          "Connection: close\r\n"
    //          "\r\n",
    //          server);

    // printf("HTTP request:\n%s\n", request);
    /*
    const char* server = HTTP_CLIENT_SERVER;
    char request[512];
    const char *json_body = "{ \"position_mm\": 1000 }";

    snprintf(request, sizeof(request),
            "GET /api/v2/E9Y2LxT4g1hQZ7aD8nR3mWx5P0qK6pV7/desks HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n"
            "\r\n",
            server);

    printf("HTTP GET request:\n%s\n", request);


    
    // --- TLS config (no cert verification) ---
    const uint8_t cert_ok[] = TLS_ROOT_CERT_OK;

    tls_config = altcp_tls_create_config_client(NULL, sizeof(cert_ok));
    assert(tls_config);

    http_CLIENT_T *state = http_client_init();
    if (!state) {
        printf("Failed to allocate state\n");
        return 1;
    }

    state->http_request = request;
    state->timeout = TLS_CLIENT_TIMEOUT_SECS;

    // --- Start connection ---
    if (!http_client_open(server, state)) {
        printf("Failed to open connection\n");
        return 1;
    }

    printf("Connecting...\n");

    // --- Main loop (Wi-Fi already assumed active) ---
    while (!state->complete) {
        sleep_ms(10);
    }

    printf("Client finished.\n");

    int err = state->error;

    free(state);
    altcp_tls_free_config(tls_config);

    return err == 0 ? 0 : 1;
    */

    
    //ACTUAL CODE BELOW
    
    
    // This should work
    const uint8_t cert_ok[] = TLS_ROOT_CERT_OK;
    const char tls_client_server[] = TLS_CLIENT_SERVER;
    const char id[] = "cd:fb:1a:53:fb:e6";
    char key[26];
    generate_ws_key(key);
    char request[512];
    key[24] = '\0';

    snprintf(request, sizeof(request),
        "GET / HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "X-Client-ID: %s\r\n"    // <- custom header with your ID
        "\r\n",
        tls_client_server, key, id);
    
    printf(request);

    tls_config = altcp_tls_create_config_client(nullptr, sizeof(cert_ok));
    assert(tls_config);
    
    

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        printf("Failed to initialize state\n");
        return 1;
    }
    state->http_request = request;
    state->timeout = TLS_CLIENT_TIMEOUT_SECS;
    
    if (!tls_client_open(TLS_CLIENT_SERVER, state)) {
        printf("Failed to open\n");
        return 1;
    }
    // ------------------------------------------
    // INITIALIZATION ENDS HERE
    // ------------------------------------------

    Button button1(10, GPIO_IRQ_EDGE_RISE);

    while(!state->complete) {
        poll_dispatcher(state);
        if (button1.hasEvent()) {
            printf("Button pressed");
            ws_send_text(state->pcb, "Button pressed");
        }

        sleep_ms(10);
    }

    sleep_ms(2000);
    int err = state->error;
    free(state);
    altcp_tls_free_config(tls_config);
    printf("Done\n");
    sleep_ms(100);

    return err == 0;
}

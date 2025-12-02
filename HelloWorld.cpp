#include <stdio.h>
#include "pico/stdlib.h"

#include "lwip/apps/sntp.h"
#include "hardware/rtc.h"


#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"


#include "tls_verify.h"

extern "C" {
    #include "tls_common.h"
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "PushButton.hpp"


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


    // This should work
    const uint8_t cert_ok[] = TLS_ROOT_CERT_OK;
    const char tls_client_server[] = TLS_CLIENT_SERVER;
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
        "\r\n",
        tls_client_server, key);
    
    printf("XD\n");
    printf(request);

    tls_config = altcp_tls_create_config_client(nullptr, sizeof(cert_ok));
    assert(tls_config);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        printf("Failed to initialize state\n");
        return 1;
    }
    printf("XDDD\n");
    state->http_request = request;
    state->timeout = TLS_CLIENT_TIMEOUT_SECS;
    
    if (!tls_client_open(TLS_CLIENT_SERVER, state)) {
        printf("Failed to open\n");
        return 1;
    }
    printf("XDDD\n");
    // ------------------------------------------
    // INITIALIZATION ENDS HERE
    // ------------------------------------------

    Button button1(10, GPIO_IRQ_EDGE_RISE);

    while(!state->complete) {
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

#include "api_call.hpp"
#include <cstdio>
#include <cstring>
#include <cassert>


#include <string_view>



#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"


extern "C" {
    #include "http_common.h"
    #include "http_verify.h"
    #include "tls_common.h"
}

namespace sxd::commands {

ApiCall::ApiCall() {
    stateHTTP = nullptr;
}

int ApiCall::operator()(std::string_view arg) {
    const char* server = HTTP_CLIENT_SERVER;  // replace with your server
    char request[512];

    // Build minimal HTTP request
    // If POST, you could append a JSON body, for GET we skip the body
    bool is_put = arg.substr(0, 3) == "PUT";

    if (is_put) {
        snprintf(request, sizeof(request),
                 "%.*s HTTP/1.1\r\n"           // first line from arg
                 "Host: %s\r\n"
                 "Content-Type: application/json\r\n"
                 "Connection: close\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 (int)arg.size(), arg.data(),
                 server,
                 json_len,
                 json_body.data());
    } else { // GET
        snprintf(request, sizeof(request),
                 "%.*s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 (int)arg.size(), arg.data(),
                 server);
    }

    printf("HTTP request:\n%s\n", request);

    // --- TLS config (no cert verification) ---
    altcp_tls_config* tls_config = altcp_tls_create_config_client(nullptr, 0);
    if (!tls_config) {
        printf("Failed to create TLS config\n");
        return 1;
    }

    // --- HTTP client state ---
    stateHTTP = http_client_init();
    stateHTTP->expected_len = 0;
    if (!stateHTTP) {
        printf("Failed to allocate HTTP client state\n");
        altcp_tls_free_config(tls_config);
        return 1;
    }

    stateHTTP->http_request = request;
    stateHTTP->timeout = HTTP_CLIENT_TIMEOUT_SECS;
    stateHTTP->tls_config = tls_config;
    if (!http_client_open(server, stateHTTP)) {
        printf("Failed to open connection\n");
        free(stateHTTP);
        altcp_tls_free_config(tls_config);
        return 1;
    }

    printf("Connecting...\n");
    requestStartTime = get_absolute_time();

    // // Poll until complete
    // while (!state->complete) {
    //     sleep_ms(10);
    // }

    // ws_send_text(state->pcb, state->body);

    // printf("Client finished.\n");

    // int err = state->error;

    // // Cleanup
    // free(state);
    // altcp_tls_free_config(tls_config);

    return 0;
}

int ApiCall::store_json(std::string_view arg) {
    std::memcpy(json_body.data(), arg.data(), arg.length());
    json_len = arg.length();
    printf("Json stored\n");
    return 0;
}

void ApiCall::poll(void* arg) {
    TLS_CLIENT_T* state = (TLS_CLIENT_T*) arg;
    if(!stateHTTP) return;
    // Check if complete, finish if so
    uint64_t response_time = absolute_time_diff_us(requestStartTime, get_absolute_time()) / 1000;
    if (response_time > MAX_RESPONSE_TIME_MS) {
        printf("HTTP request took too long\n");
        ws_send_text(state->pcb, "res:failed");
        http_client_close(stateHTTP);

        // Cleanup
        free(stateHTTP);
        altcp_tls_free_config(stateHTTP->tls_config);
        stateHTTP = nullptr;
    }
    if (!stateHTTP->complete) {
        return;
    }
    printf("%s\n\n", stateHTTP->body);
    ws_send_text(state->pcb, stateHTTP->body);

    printf("HTTP finished\n");

    int err = stateHTTP->error;
    http_client_close(stateHTTP);

    // Cleanup
    altcp_tls_free_config(stateHTTP->tls_config);
    free(stateHTTP);
    stateHTTP = nullptr;
}


} // namespace sxd::commands

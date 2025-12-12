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
}

namespace sxd::commands {

int ApiCall::operator()(std::string_view arg) {
    const char* server = "example.com";  // replace with your server
    char request[512];

    // Build minimal HTTP request
    // If POST, you could append a JSON body, for GET we skip the body
    bool is_post = arg.substr(0, 4) == "POST";

    if (is_post) {
        const char* json_body = "{ \"position_mm\": 1000 }";
        snprintf(request, sizeof(request),
                 "%.*s\r\n"           // first line from arg
                 "Host: %s\r\n"
                 "Content-Type: application/json\r\n"
                 "Connection: close\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 (int)arg.size(), arg.data(),
                 server,
                 strlen(json_body),
                 json_body);
    } else { // GET
        snprintf(request, sizeof(request),
                 "%.*s\r\n"
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
    http_CLIENT_T* state = http_client_init();
    if (!state) {
        printf("Failed to allocate HTTP client state\n");
        altcp_tls_free_config(tls_config);
        return 1;
    }

    state->http_request = request;
    state->timeout = HTTP_CLIENT_TIMEOUT_SECS;

    if (!http_client_open(server, state)) {
        printf("Failed to open connection\n");
        free(state);
        altcp_tls_free_config(tls_config);
        return 1;
    }

    printf("Connecting...\n");

    // Poll until complete
    while (!state->complete) {
        sleep_ms(10);
    }

    printf("Client finished.\n");

    int err = state->error;

    // Cleanup
    free(state);
    altcp_tls_free_config(tls_config);

    return err;
}

} // namespace sxd::commands

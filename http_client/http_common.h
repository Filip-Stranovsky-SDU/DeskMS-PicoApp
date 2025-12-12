#pragma once

#define http_SERVER_PORT 8000

typedef struct http_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
} http_CLIENT_T;

#ifdef __cplusplus
extern "C" {
#endif

extern struct altcp_tls_config *http_config;

#ifdef __cplusplus
}
#endif

// void ws_send_text(struct altcp_pcb *pcb, const char *msg);


err_t http_client_close(void *arg);
err_t http_client_connected(void *arg, struct altcp_pcb *pcb, err_t err);
err_t http_client_poll(void *arg, struct altcp_pcb *pcb);
void http_client_err(void *arg, err_t err);
err_t http_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err);
void http_client_connect_to_server_ip(const ip_addr_t *ipaddr, http_CLIENT_T *state);
void http_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg);
bool http_client_open(const char *hostname, void *arg);
http_CLIENT_T* http_client_init(void);
bool run_http_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout);

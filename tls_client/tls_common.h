#pragma once

#define TLS_SERVER_PORT 8443

typedef struct TLS_CLIENT_T_ {
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    int timeout;
} TLS_CLIENT_T;

#ifdef __cplusplus
extern "C" {
#endif

extern struct altcp_tls_config *tls_config;

#ifdef __cplusplus
}
#endif

void ws_send_text(struct altcp_pcb *pcb, const char *msg);


err_t tls_client_close(void *arg);
err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err);
err_t tls_client_poll(void *arg, struct altcp_pcb *pcb);
void tls_client_err(void *arg, err_t err);
err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err);
void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state);
void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg);
bool tls_client_open(const char *hostname, void *arg);
TLS_CLIENT_T* tls_client_init(void);
bool run_tls_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout);

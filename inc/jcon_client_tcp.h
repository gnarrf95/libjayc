#ifndef INCLUDE_JCON_CLIENT_TCP_H
#define INCLUDE_JCON_CLIENT_TCP_H

#include <jcon_client.h>
#include <stdint.h>

#include <jlog.h>
#include <netinet/in.h>

#define JCON_CLIENT_TCP_CONNECTIONTYPE "TCP"
#define JCON_CLIENT_TCP_POLL_TIMEOUT_DEFAULT 10
#define JCON_CLIENT_TCP_MAX_MESSAGE_SIZE 4096

struct __jcon_client_tcp_context;

typedef struct __jcon_client_tcp_context
{
  int file_descriptor;
  struct sockaddr_in socket_address;
  char *reference_string;
  int poll_timeout; // timeout in milliseconds
  jlog_t *logger;
} jcon_client_tcp_context_t;

jcon_client_t *jcon_client_tcp_session_init(char *address, uint16_t port, jlog_t *logger);
jcon_client_t *jcon_client_tcp_session_clone(int file_descriptor, struct sockaddr_in socket_address, jlog_t *logger);

void jcon_client_tcp_session_free(void *ctx);

int jcon_client_tcp_reset(void *ctx);
void jcon_client_tcp_close(void *ctx);

int jcon_client_tcp_isConnected(void *ctx);

char *jcon_client_tcp_createReferenceString(struct sockaddr_in sock_address);
const char *jcon_client_tcp_getReferenceString(void *ctx);
char *jcon_client_tcp_getIP(struct sockaddr_in socket_address);
uint16_t jcon_client_tcp_getPort(struct sockaddr_in socket_address);

int jcon_client_tcp_newData(void *ctx);

size_t jcon_client_tcp_recvData(void *ctx, void *data_ptr, size_t data_size);
size_t jcon_client_tcp_sendData(void *ctx, void *data_ptr, size_t data_size);

void jcon_client_tcp_log(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

#endif
/**
 * @file jcon_tcp.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Provides interface to handle TCP sockets.
 * 
 * @date 2020-09-27
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_TCP_H
#define INCLUDE_JCON_TCP_H

#include <jlog.h>
#include <stddef.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __jcon_tcp_session jcon_tcp_t;

jcon_tcp_t *jcon_tcp_simple_init(const char *address, uint16_t port, jlog_t *logger);
jcon_tcp_t *jcon_tcp_clone(int fd, struct sockaddr_in socket_address, jlog_t *logger);
void jcon_tcp_free(jcon_tcp_t *session);

int jcon_tcp_connect(jcon_tcp_t *session);
int jcon_tcp_bind(jcon_tcp_t *session);
void jcon_tcp_close(jcon_tcp_t *session);

int jcon_tcp_pollForInput(jcon_tcp_t *session, int timeout);

jcon_tcp_t *jcon_tcp_accept(jcon_tcp_t *session);

size_t jcon_tcp_recvData(jcon_tcp_t *session, void *data_ptr, size_t data_size);
size_t jcon_tcp_sendData(jcon_tcp_t *session, void *data_ptr, size_t data_size);

int jcon_tcp_isConnected(jcon_tcp_t *session);
const char *jcon_tcp_getReferenceString(jcon_tcp_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_TCP_H */
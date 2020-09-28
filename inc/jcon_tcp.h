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

struct __jcon_tcp_session
{
  int file_descriptor;
  struct sockaddr_in socket_address;
  
  char *referenceString;
  jlog_t *logger;
};

void jcon_tcp_log(jcon_tcp_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

#define DEBUG(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

jcon_tcp_t *jcon_tcp_simple_init(const char *address, uint16_t port, jlog_t *logger);
jcon_tcp_t *jcon_tcp_clone(int fd, struct sockaddr_in socket_address, jlog_t *logger);
void jcon_tcp_free(jcon_tcp_t *session);

int jcon_tcp_connect(jcon_tcp_t *session);
int jcon_tcp_bind(jcon_tcp_t *session);
void jcon_tcp_close(jcon_tcp_t *session);

int jcon_tcp_pollForInput(jcon_tcp_t *session, int timeout);

int jcon_tcp_isConnected(jcon_tcp_t *session);

jcon_tcp_t *jcon_tcp_accept(jcon_tcp_t *session);

size_t jcon_tcp_recvData(jcon_tcp_t *session, void *data_ptr, size_t data_size);
size_t jcon_tcp_sendData(jcon_tcp_t *session, void *data_ptr, size_t data_size);

char *jcon_tcp_getIP(struct sockaddr_in socket_address);
uint16_t jcon_tcp_getPort(struct sockaddr_in socket_address);

char *jcon_tcp_createReferenceString(struct sockaddr_in socket_address);
const char *jcon_tcp_getReferenceString(jcon_tcp_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_TCP_H */
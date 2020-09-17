#include <jcon_client_tcp.h>
#include <jcon_client_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

#define DEBUG(ctx, fmt, ...) jcon_client_tcp_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(ctx, fmt, ...) jcon_client_tcp_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jcon_client_tcp_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jcon_client_tcp_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//------------------------------------------------------------------------------
//
jcon_client_t *jcon_client_tcp_session_init(char *address, uint16_t port, jlog_t *logger)
{
  jcon_client_t *session = (jcon_client_t *)malloc(sizeof(jcon_client_t));
  if(session == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", address, port);
    return NULL;
  }

  session->function_reset = &jcon_client_tcp_reset;
  session->function_close = &jcon_client_tcp_close;
  session->function_isConnected = &jcon_client_tcp_isConnected;
  session->function_newData = &jcon_client_tcp_newData;
  session->function_recvData = &jcon_client_tcp_recvData;
  session->function_sendData = &jcon_client_tcp_sendData;
  session->session_free_handler = &jcon_client_tcp_session_free;
  session->connection_type = JCON_CLIENT_TCP_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_client_tcp_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed. Destroying session.", address, port);
    free(session);
    return NULL;
  }

  jcon_client_tcp_context_t *ctx = (jcon_client_tcp_context_t *)session->session_context;

  ctx->file_descriptor = 0;
  ctx->poll_timeout = JCON_CLIENT_TCP_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;

  ctx->socket_address.sin_family = AF_INET;
  ctx->socket_address.sin_port = htons(port);

  struct hostent *hostinfo;
  hostinfo = gethostbyname(address);
  if(hostinfo == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> gethostbyname() failed. Destroying context and session.", address, port);
    jcon_client_tcp_session_free(ctx);
    free(session);
    return NULL;
  }

  ctx->socket_address.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

  ctx->reference_string = jcon_client_tcp_createReferenceString(ctx->socket_address);
  if(ctx->reference_string == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> json_client_tcp_createReferenceString() failed. Destroying context and session.", jcon_client_tcp_getIP(ctx->socket_address), jcon_client_tcp_getPort(ctx->socket_address));
    free(ctx);
    free(session);
    return NULL;
  }

  // int ret_reset = jcon_client_tcp_reset(ctx);
  // if(ret_reset)
  // {
  //   return session;
  // }
  
  // ERROR(ctx, "jcon_client_tcp_reset() failed. Destroying context and session.");
  // jcon_client_tcp_session_free(ctx);
  // free(session);
  // return NULL;

  return session;
}

//------------------------------------------------------------------------------
//
jcon_client_t *jcon_client_tcp_session_clone(int file_descriptor, struct sockaddr_in socket_address, jlog_t *logger)
{
  jcon_client_t *session = (jcon_client_t *)malloc(sizeof(jcon_client_t));
  if(session == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", jcon_client_tcp_getIP(socket_address), jcon_client_tcp_getPort(socket_address));
    return NULL;
  }

  session->function_reset = &jcon_client_tcp_reset;
  session->function_close = &jcon_client_tcp_close;
  session->function_isConnected = &jcon_client_tcp_isConnected;
  session->function_newData = &jcon_client_tcp_newData;
  session->function_recvData = &jcon_client_tcp_recvData;
  session->function_sendData = &jcon_client_tcp_sendData;
  session->session_free_handler = &jcon_client_tcp_session_free;
  session->connection_type = JCON_CLIENT_TCP_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_client_tcp_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed. Destroying session.", jcon_client_tcp_getIP(socket_address), jcon_client_tcp_getPort(socket_address));
    free(session);
    return NULL;
  }

  jcon_client_tcp_context_t *ctx = (jcon_client_tcp_context_t *)session->session_context;

  ctx->file_descriptor = file_descriptor;
  ctx->socket_address = socket_address;
  ctx->poll_timeout = JCON_CLIENT_TCP_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;
  ctx->reference_string = jcon_client_tcp_createReferenceString(socket_address);
  if(ctx->reference_string == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> json_client_tcp_createReferenceString() failed. Destroying context and session.", jcon_client_tcp_getIP(socket_address), jcon_client_tcp_getPort(socket_address));
    free(ctx);
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_client_tcp_session_free(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_client_tcp_close(ctx);
  free(((jcon_client_tcp_context_t *)ctx)->reference_string);
  free(ctx);
}

//------------------------------------------------------------------------------
//
int jcon_client_tcp_reset(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_client_tcp_close(ctx);
  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(ctx, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(connect(fd, (struct sockaddr *)&(session_context->socket_address), sizeof(session_context->socket_address)) < 0)
  {
    ERROR(ctx, "connect() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(ctx, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  DEBUG(ctx, "Socket is connected.");
  session_context->file_descriptor = fd;
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_client_tcp_close(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  if(session_context->file_descriptor > 0)
  {
    if(close(session_context->file_descriptor) < 0)
    {
      ERROR(ctx, "close() failed [%d : %s].", errno, strerror(errno));
      return;
    }
    session_context->file_descriptor = 0;
    DEBUG(ctx, "File descriptor closed.");
  }
  else
  {
    DEBUG(ctx, "File descriptor is already closed.");
  }
}

//------------------------------------------------------------------------------
//
int jcon_client_tcp_isConnected(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  return (session_context->file_descriptor > 0);
}

//------------------------------------------------------------------------------
//
char *jcon_client_tcp_createReferenceString(struct sockaddr_in socket_address)
{
  char buf[128] = { 0 };
  char *ip;
  uint16_t port;
  
  ip = jcon_client_tcp_getIP(socket_address);
  if(ip == NULL)
  {
    ERROR(NULL, "jcon_client_tcp_getIP() failed.");
    return NULL;
  }

  port = jcon_client_tcp_getPort(socket_address);
  if(port == 0)
  {
    ERROR(NULL, "jcon_client_tcp_getPort() failed.");
    return NULL;
  }

  if(sprintf(buf, "TCP:%s:%u", ip, port) < 0)
  {
    ERROR(NULL, "sprintf() failed.");
    return NULL;
  }

  char *ret = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
  if(ret == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", jcon_client_tcp_getIP(socket_address), jcon_client_tcp_getPort(socket_address));
    return NULL;
  }

  strcpy(ret, buf);

  return ret;
}

//------------------------------------------------------------------------------
//
const char *jcon_client_tcp_getReferenceString(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  return (const char *)session_context->reference_string;
}

//------------------------------------------------------------------------------
//
char *jcon_client_tcp_getIP(struct sockaddr_in socket_address)
{
  char *ip = inet_ntoa(socket_address.sin_addr);
  if(ip == NULL)
  {
    ERROR(NULL, "inet_ntoa() failed.");
    return NULL;
  }

  return ip;
}

//------------------------------------------------------------------------------
//
uint16_t jcon_client_tcp_getPort(struct sockaddr_in socket_address)
{
  return ntohs(socket_address.sin_port);
}

//------------------------------------------------------------------------------
//
int jcon_client_tcp_newData(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  struct pollfd poll_fds[1];
  poll_fds->fd = session_context->file_descriptor;
  poll_fds->events = POLLIN;

  int ret_poll = poll(poll_fds, 1, session_context->poll_timeout);

  if(ret_poll < 0)
  {
    ERROR(ctx, "poll() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(ret_poll == 0)
  {
    return false;
  }

  if(poll_fds->revents & POLLERR)
  {
    DEBUG(ctx, "poll() recieved [POLLERR].");
    jcon_client_tcp_close(ctx);
    return false;
  }
  if(poll_fds->revents & POLLNVAL)
  {
    DEBUG(ctx, "poll() recieved [POLLNVAL].");
    return false;
  }
  if(poll_fds->revents & POLLIN)
  {
    DEBUG(ctx, "poll() recieved [POLLIN].");
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
//
size_t jcon_client_tcp_recvData(void *ctx, void *data_ptr, size_t data_size)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  if(data_size == 0)
  {
    ERROR(ctx, "data_size given is [0].");
    return 0;
  }
  if(data_size > JCON_CLIENT_TCP_MAX_MESSAGE_SIZE)
  {
    ERROR(ctx, "data_size [%u] exceeds maximum message size [%u].", data_size, JCON_CLIENT_TCP_MAX_MESSAGE_SIZE);
    return 0;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  if(jcon_client_tcp_isConnected(ctx) == false)
  {
    WARN(ctx, "Session is not connected.");
    return 0;
  }

  uint8_t buf[JCON_CLIENT_TCP_MAX_MESSAGE_SIZE] = { 0 };
  int ret_recv = recv(session_context->file_descriptor, buf, sizeof(buf), 0);
  if(ret_recv < 0)
  {
    ERROR(ctx, "recv() failed [%d : %s].", errno, strerror(errno));
    return 0;
  }

  if(ret_recv == 0)
  {
    DEBUG(ctx, "recv() returned [0]. Closing connection.");
    jcon_client_tcp_close(ctx);
    return 0;
  }

  // If data_ptr is NULL, data is still read, but not returned.
  if(data_ptr)
  {
    memcpy(data_ptr, buf, ret_recv);
  }
  return ret_recv;
}

//------------------------------------------------------------------------------
//
size_t jcon_client_tcp_sendData(void *ctx, void *data_ptr, size_t data_size)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  if(data_ptr == NULL)
  {
    ERROR(ctx, "data_ptr is NULL.");
    return 0;
  }

  if(data_size == 0)
  {
    ERROR(ctx, "data_size given is [0].");
    return 0;
  }
  if(data_size > JCON_CLIENT_TCP_MAX_MESSAGE_SIZE)
  {
    ERROR(ctx, "data_size [%u] exceeds maximum message size [%u].", data_size, JCON_CLIENT_TCP_MAX_MESSAGE_SIZE);
    return 0;
  }

  jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

  if(jcon_client_tcp_isConnected(ctx) == false)
  {
    WARN(ctx, "Session is not connected.");
    return 0;
  }

  int ret_send = send(session_context->file_descriptor, data_ptr, data_size, 0);
  if(ret_send < 0)
  {
    ERROR(ctx, "send() failed [%d : %s].", errno, strerror(errno));
    return 0;
  }

  return ret_send;
}

//------------------------------------------------------------------------------
//
void jcon_client_tcp_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(ctx)
  {
    jcon_client_tcp_context_t *session_context = (jcon_client_tcp_context_t *)ctx;

    if(session_context->logger)
    {
      jlog_log_message_m(session_context->logger, log_type, file, function, line, "<%s> %s", jcon_client_tcp_getReferenceString(ctx), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_client_tcp_getReferenceString(ctx), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
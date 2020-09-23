/**
 * @file jcon_server_tcp.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jcon_server_tcp.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_server_tcp.h>
#include <jcon_server_dev.h>
#include <jcon_client_tcp.h>
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

typedef struct __jcon_server_tcp_context jcon_server_tcp_context_t;

//==============================================================================
// Define constants and defaults.
//==============================================================================

/**
 * @brief Connection type, to return for @c #jcon_client_getConnectionType() .
 */
#define JCON_SERVER_TCP_CONNECTIONTYPE "TCP"

/**
 * @brief Default value for polling timeout.
 * 
 * When checking, if new data is available, function @c poll()
 * is used. This value tells the function, how long to
 * wait for new data in milliseconds.
 */
#define JCON_SERVER_TCP_POLL_TIMEOUT_DEFAULT 10

//==============================================================================
// Declare handlers and internal functions.
//==============================================================================

/**
 * @brief Function for context free handler.
 * 
 * Will free context data.
 * 
 * @param ctx Session context to free.
 */
static void jcon_server_tcp_session_free(void *ctx);

/**
 * @brief Function for reset handler.
 * 
 * Restarts the server.
 * 
 * @param ctx Context pointer with socket data.
 * 
 * @return    @c true , if reset was successful.
 * @return    @c false , if reset failed.
 */
static int jcon_server_tcp_reset(void *ctx);

/**
 * @brief Function for close handler.
 * 
 * Closes server socket.
 * 
 * @param ctx Context pointer with socket data.
 */
static void jcon_server_tcp_close(void *ctx);

/**
 * @brief Checks if socket is open.
 * 
 * @param ctx Context pointer with socket data.
 * 
 * @return    @c true , if socket is open.
 * @return    @c false , if socket closed or error occured.
 */
static int jcon_server_tcp_isOpen(void *ctx);

/**
 * @brief Creates @c jcon_server_tcp_context_t#reference_string .
 * 
 * @param socket_address  Socket address of server.
 * 
 * @return                Allocated string for reference.
 * @return                @c NULL , if error occured.
 */
static char *jcon_server_tcp_createReferenceString(struct sockaddr_in socket_address);

/**
 * @brief Returnes @c jcon_server_tcp_context_t#reference_string .
 * 
 * @param ctx Context of session to ask from.
 * 
 * @return    Session string.
 * @return    @c NULL , if error occured.
 */
static const char *jcon_server_tcp_getReferenceString(void *ctx);

/**
 * @brief Gets IP address from socket address struct.
 * 
 * @param socket_address  Socket address to check.
 * 
 * @return                String with IP address.
 * @return                @c NULL , if error occured.
 */
static char *jcon_server_tcp_getIP(struct sockaddr_in socket_address);

/**
 * @brief Gets Port number from socket address struct.
 * 
 * @param socket_address  Socket address to check.
 * 
 * @return                Port number.
 * @return                @c 0 , if error occured.
 */
static uint16_t jcon_server_tcp_getPort(struct sockaddr_in socket_address);

/**
 * @brief Checks, if new connection is available.
 * 
 * Polls socket, to check if new connections are available.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    @c true , if new connection is available.
 * @return    @c false , if no connection or error occured.
 */
static int jcon_server_tcp_newConnection(void *ctx);

/**
 * @brief Accepts connection and creates jcon_client.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    New jcon_client session object.
 * @return    @c NULL , if error occured.
 */
static jcon_client_t *jcon_server_tcp_acceptConnection(void *ctx);

/**
 * @brief Logs debug and error messages.
 * 
 * Uses logger from @c ctx , or if logger is @c NULL , uses global logger.
 * 
 * @param ctx       Session for info about connection.
 * @param log_type  Type of log message.
 * @param file      Source code file, where message was logged.
 * @param function  Function in which message was logged.
 * @param line      Line, where message was logged.
 * @param fmt       Format string for stdarg.h .
 */
static void jcon_server_tcp_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

//==============================================================================
// Define context structure and log macros.
//==============================================================================

/**
 * @brief Data for jcon_server_tcp object.
 */
struct __jcon_server_tcp_context
{
  int file_descriptor;                /**< File descriptor for TCP socket. */
  struct sockaddr_in socket_address;  /**< Describes connection address. */
  char *reference_string;             /**< Connection info to return. */
  int poll_timeout;                   /**< Timeout for asking for new data in milliseconds. */
  jlog_t *logger;                     /**< Logger for debug and error messages. */
};

#define DEBUG(ctx, fmt, ...) jcon_server_tcp_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(ctx, fmt, ...) jcon_server_tcp_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jcon_server_tcp_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jcon_server_tcp_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//==============================================================================
// Implement handlers and internal functions.
//==============================================================================

//------------------------------------------------------------------------------
//
jcon_server_t *jcon_server_tcp_session_init(char *address, uint16_t port, jlog_t *logger)
{
  jcon_server_t *session = (jcon_server_t *)malloc(sizeof(jcon_server_t));
  if(session == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", address, port);
    return NULL;
  }

  session->session_free_handler = &jcon_server_tcp_session_free;
  session->function_reset = &jcon_server_tcp_reset;
  session->function_close = &jcon_server_tcp_close;
  session->function_isOpen = &jcon_server_tcp_isOpen;
  session->function_getReferenceString = &jcon_server_tcp_getReferenceString;
  session->function_newConnection = &jcon_server_tcp_newConnection;
  session->function_acceptConnection = &jcon_server_tcp_acceptConnection;
  session->connection_type = JCON_SERVER_TCP_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_server_tcp_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed. Destroying session.", address, port);
    free(session);
    return NULL;
  }

  jcon_server_tcp_context_t *ctx = (jcon_server_tcp_context_t *)session->session_context;

  ctx->file_descriptor = 0;
  ctx->poll_timeout = JCON_SERVER_TCP_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;

  ctx->socket_address.sin_family = AF_INET;
  ctx->socket_address.sin_port = htons(port);

  struct hostent *hostinfo;
  hostinfo = gethostbyname(address);
  if(hostinfo == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> gethostbyname() failed. Destroying context and session.", address, port);
    free(ctx);
    free(session);
    return NULL;
  }

  ctx->socket_address.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

  ctx->reference_string = jcon_server_tcp_createReferenceString(ctx->socket_address);
  if(ctx->reference_string == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> json_client_tcp_createReferenceString() failed. Destroying context and session.", jcon_server_tcp_getIP(ctx->socket_address), jcon_server_tcp_getPort(ctx->socket_address));
    free(ctx);
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_server_tcp_session_free(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_server_tcp_close(ctx);
  free(((jcon_server_tcp_context_t *)ctx)->reference_string);
  free(ctx);
}

//------------------------------------------------------------------------------
//
int jcon_server_tcp_reset(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_server_tcp_close(ctx);
  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(ctx, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(bind(fd, (struct sockaddr *)&session_context->socket_address, sizeof(session_context->socket_address)) < 0)
  {
    ERROR(ctx, "bind() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(ctx, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  if(listen(fd, 5) < 0)
  {
    ERROR(ctx, "listen() failed [%d : %s]. Closing socket.", errno, strerror(errno));
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
void jcon_server_tcp_close(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

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
int jcon_server_tcp_isOpen(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

  return (session_context->file_descriptor > 0);
}

//------------------------------------------------------------------------------
//
char *jcon_server_tcp_createReferenceString(struct sockaddr_in socket_address)
{
  char buf[128] = { 0 };
  char *ip;
  uint16_t port;
  
  ip = jcon_server_tcp_getIP(socket_address);
  if(ip == NULL)
  {
    ERROR(NULL, "jcon_client_tcp_getIP() failed.");
    return NULL;
  }

  port = jcon_server_tcp_getPort(socket_address);
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

  size_t size_refString = sizeof(char) * (strlen(buf) + 1);
  char *ret = (char *)malloc(size_refString);
  if(ret == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", jcon_server_tcp_getIP(socket_address), jcon_server_tcp_getPort(socket_address));
    return NULL;
  }

  /* Changed implementation from using strcpy, to using memcpy;
     to calm down devskim checks. */
  bzero(ret, size_refString);
  memcpy(ret, buf, size_refString);

  return ret;
}

//------------------------------------------------------------------------------
//
const char *jcon_server_tcp_getReferenceString(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

  return (const char *)session_context->reference_string;
}

//------------------------------------------------------------------------------
//
char *jcon_server_tcp_getIP(struct sockaddr_in socket_address)
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
uint16_t jcon_server_tcp_getPort(struct sockaddr_in socket_address)
{
  return ntohs(socket_address.sin_port);
}

//------------------------------------------------------------------------------
//
int jcon_server_tcp_newConnection(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

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
    jcon_server_tcp_close(ctx);
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
jcon_client_t *jcon_server_tcp_acceptConnection(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

  int new_fd;
  struct sockaddr_in new_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  new_fd = accept(session_context->file_descriptor, (struct sockaddr *)&new_addr, &addrlen);
  if(new_fd < 0)
  {
    ERROR(ctx, "accept() failed [%d : %s].", errno, strerror(errno));
    return NULL;
  }

  jcon_client_t *new_client = jcon_client_tcp_session_clone(new_fd, new_addr, session_context->logger);
  if(new_client == NULL)
  {
    ERROR(ctx, "jcon_client_tcp_session_clone() failed with new connection [TCP:%s:%u].", jcon_server_tcp_getIP(new_addr), jcon_server_tcp_getPort(new_addr));
    close(new_fd);
    return NULL;
  }

  return new_client;
}

//------------------------------------------------------------------------------
//
void jcon_server_tcp_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(ctx)
  {
    jcon_server_tcp_context_t *session_context = (jcon_server_tcp_context_t *)ctx;

    if(session_context->logger)
    {
      jlog_log_message_m(session_context->logger, log_type, file, function, line, "<%s> %s", jcon_server_tcp_getReferenceString(ctx), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_server_tcp_getReferenceString(ctx), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
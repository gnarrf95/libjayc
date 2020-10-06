/**
 * @file jcon_socketTCP.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementations for jcon_socketTCP.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_socketTCP.h>
#include <jayc/jcon_socket_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//==============================================================================
// Define constants and structures.
//

#define JCON_SOCKETTCP_CONNECTIONTYPE "TCP"

/**
 * @brief Session context for TCP sockets.
 * 
 */
typedef struct __jcon_socketTCP_context
{
  struct sockaddr_in socket_address;  /**< Address struct. */
} jcon_socketTCP_ctx_t;



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Creates session from existing socket.
 * 
 * Used by @c #jcon_socketTCP_accept() .
 * If server accepts new client, the connection to said
 * client is made into new session.
 * 
 * @param fd              File descriptor of new socket.
 * @param socket_address  Address struct of new socket.
 * @param logger          Logger for new session.
 * 
 * @return                Session object for new client connection.
 * @return                @c NULL in case of error.
 */
static jcon_socket_t *jcon_socketTCP_clone(int fd, struct sockaddr_in socket_address, jlog_t *logger);

/**
 * @brief Frees session memory.
 * 
 * Called by @c #jcon_socket_free() .
 * 
 * @param session Session object to free.
 */
static void jcon_socketTCP_free(jcon_socket_t *session);

/**
 * @brief Connect to server.
 * 
 * Called by @c #jcon_socket_connect() .
 * 
 * @param session Session to connect.
 * 
 * @return        @c true , if connection was established.
 * @return        @c false , if connection failed.
 */
static int jcon_socketTCP_connect(jcon_socket_t *session);

/**
 * @brief Binds socket to address.
 * 
 * Called by @c #jcon_socket_bind() .
 * 
 * @param session Session to bind.
 * 
 * @return        @c true , if socket was bound to address.
 * @return        @c false , if binding failed.
 */
static int jcon_socketTCP_bind(jcon_socket_t *session);

/**
 * @brief Accepts connection request.
 * 
 * Called by @c #jcon_socket_accept() .
 * 
 * @param session Server session, to accept connection.
 * 
 * @return        Session object of client connection.
 * @return        @c NULL , if no new connection was
 *                available or error occured.
 */
static jcon_socket_t *jcon_socketTCP_accept(jcon_socket_t *session);

/**
 * @brief Extract IP address from address struct.
 * 
 * @param socket_address  Address struct.
 * 
 * @return                String with IP address.
 * @return                @c NULL in case of error.
 */
static char *jcon_socketTCP_getIP(struct sockaddr_in socket_address);

/**
 * @brief Extract port number from address struct. 
 * 
 * @param socket_address  Address struct.
 * 
 * @return                Port number.
 * @return                @c 0 in case of error.
 */
static uint16_t jcon_socketTCP_getPort(struct sockaddr_in socket_address);

/**
 * @brief Create reference string from socket address.
 * 
 * Reference string consists of connection type (TCP),
 * IP address ( @c #jcon_tcp_getIP() ) and port number
 * ( @c #jcon_tcp_getPort() ).
 * 
 * These items get combined into one string.
 * 
 * @param socket_address  Address struct.
 * 
 * @return                Allocated reference string.
 * @return                @c NULL in case of error.
 */
static char *jcon_socketTCP_createReferenceString(struct sockaddr_in socket_address);

/**
 * @brief Sends log messages to logger with session data.
 * 
 * Uses logger. If available adds reference string to log messages.
 * 
 * @param session   Session object.
 * @param log_type  (debug, info, warning, error, critical, fatal).
 * @param file      Source code file.
 * @param function  Function name.
 * @param line      Line number of source file.
 * @param fmt       String format for stdarg.h .
 */
static void jcon_socketTCP_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG /* Allow to turn of debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_socketTCP_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface funcions.
//

//------------------------------------------------------------------------------
//
jcon_socket_t *jcon_socketTCP_simple_init(const char *address, uint16_t port, jlog_t *logger)
{
  jcon_socket_t *session = (jcon_socket_t *)malloc(sizeof(jcon_socket_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->function_connect = &jcon_socketTCP_connect;
  session->function_bind = &jcon_socketTCP_bind;
  session->function_close = NULL;
  session->function_accept = &jcon_socketTCP_accept;
  session->session_free_handler = &jcon_socketTCP_free;

  session->file_descriptor = 0;
  session->logger = logger;
  session->socket_type = JCON_SOCKETTCP_CONNECTIONTYPE;
  session->connection_type = 0;

  jcon_socketTCP_ctx_t *ctx = (jcon_socketTCP_ctx_t *)malloc(sizeof(jcon_socketTCP_ctx_t));
  if(ctx == NULL)
  {
    ERROR(NULL, "malloc() failed. Destroying session.");
    free(session);
    return NULL;
  }

  ctx->socket_address.sin_family = AF_INET;
  ctx->socket_address.sin_port = htons(port);

  struct hostent *hostinfo;
  hostinfo = gethostbyname(address);
  if(hostinfo == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> gethostbyname() failed. Destroying context and session.", address, port);
    free(session);
    return NULL;
  }
  ctx->socket_address.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

  session->referenceString = jcon_socketTCP_createReferenceString(ctx->socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<TCP> jcon_socketTCP_createReferenceString() failed. Destroying session.");
    free(session);
    free(ctx);
    return NULL;
  }

  session->session_ctx = (void *)ctx;

  return session;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
jcon_socket_t *jcon_socketTCP_clone(int fd, struct sockaddr_in socket_address, jlog_t *logger)
{
  if(fd <= 0)
  {
    ERROR(NULL, "Invalid file descriptor [%d].", fd);
    return NULL;
  }

  jcon_socket_t *session = (jcon_socket_t *)malloc(sizeof(jcon_socket_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->function_connect = NULL; /* Cloned sessions cannot reconnect. */
  session->function_bind = NULL; /* Cloned sessions cannot bind. */
  session->function_close = NULL;
  session->function_accept = NULL; /* Cloned sessions cannot bind and therefore not accept. */
  session->session_free_handler = &jcon_socketTCP_free;

  session->file_descriptor = fd;
  session->logger = logger;
  session->connection_type = JCON_SOCKET_CONNECTIONTYPE_CLIENT;

  jcon_socketTCP_ctx_t *ctx = (jcon_socketTCP_ctx_t *)malloc(sizeof(jcon_socketTCP_ctx_t));
  if(ctx == NULL)
  {
    ERROR(NULL, "malloc() failed. Destroying session.");
    free(session);
    return NULL;
  }

  ctx->socket_address = socket_address;
  session->referenceString = jcon_socketTCP_createReferenceString(socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<TCP> json_socketTCP_createReferenceString() failed. Destroying context and session.");
    free(session);
    free(ctx);
    return NULL;
  }

  session->session_ctx = ctx;

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_socketTCP_free(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(session->session_ctx)
  {
    free(session->session_ctx);
  }
}

//------------------------------------------------------------------------------
//
int jcon_socketTCP_connect(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_socket_isConnected(session))
  {
    DEBUG(session, "Session is already connected.");
    return true;
  }

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  struct sockaddr_in addr = ((jcon_socketTCP_ctx_t *)session->session_ctx)->socket_address;

  if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    ERROR(session, "connect() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  session->file_descriptor = fd;
  return true;
}


//------------------------------------------------------------------------------
//
int jcon_socketTCP_bind(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_socket_isConnected(session))
  {
    DEBUG(session, "Session is already connected.");
    return true;
  }

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  struct sockaddr_in addr = ((jcon_socketTCP_ctx_t *)session->session_ctx)->socket_address;

  if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    ERROR(session, "bind() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  if(listen(fd, 5) < 0)
  {
    ERROR(session, "listen() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  session->file_descriptor = fd;
  return true;
}


//------------------------------------------------------------------------------
//
jcon_socket_t *jcon_socketTCP_accept(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  int new_fd;
  struct sockaddr_in new_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  new_fd = accept(session->file_descriptor, (struct sockaddr *)&new_addr, &addrlen);
  if(new_fd < 0)
  {
    ERROR(session, "accept() failed [%d : %s].", errno, strerror(errno));
    return NULL;
  }

  jcon_socket_t *new_con = jcon_socketTCP_clone(new_fd, new_addr, session->logger);
  if(new_con == NULL)
  {
    ERROR(session, "jcon_socket_clone() failed with new connection [TCP:%s:%u].", jcon_socketTCP_getIP(new_addr), jcon_socketTCP_getPort(new_addr));
    close(new_fd);
    return NULL;
  }

  return new_con;
}

//------------------------------------------------------------------------------
//
char *jcon_socketTCP_getIP(struct sockaddr_in socket_address)
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
uint16_t jcon_socketTCP_getPort(struct sockaddr_in socket_address)
{
  return ntohs(socket_address.sin_port);
}

//------------------------------------------------------------------------------
//
char *jcon_socketTCP_createReferenceString(struct sockaddr_in socket_address)
{
  char buf[128] = { 0 };
  char *ip;
  uint16_t port;
  
  ip = jcon_socketTCP_getIP(socket_address);
  if(ip == NULL)
  {
    ERROR(NULL, "jcon_socketTCP_getIP() failed.");
    return NULL;
  }

  port = jcon_socketTCP_getPort(socket_address);
  if(port == 0)
  {
    ERROR(NULL, "jcon_socketTCP_getPort() failed.");
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
    ERROR(NULL, "<TCP:%s:%u> malloc() failed.", ip, port);
    return NULL;
  }

  /* Changed implementation from using strcpy, to using memcpy;
     to calm down devskim checks. */
  memset(ret, 0, size_refString);
  memcpy(ret, buf, size_refString);

  return ret;
}

//------------------------------------------------------------------------------
//
void jcon_socketTCP_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session)
  {
    if(session->logger)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_socket_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_socket_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
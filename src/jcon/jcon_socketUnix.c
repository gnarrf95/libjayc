/**
 * @file jcon_socketUnix.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementations for jcon_socketUnix.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_socketUnix.h>
#include <jayc/jcon_socket_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

//==============================================================================
// Define constants and structures.
//

#define JCON_SOCKETUNIX_CONNECTIONTYPE "UNIX"

/**
 * @brief Session context for Unix sockets.
 * 
 */
typedef struct __jcon_socketUnix_context
{
  struct sockaddr_un socket_address;  /**< Address struct. */
} jcon_socketUnix_ctx_t;



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Creates session from existing socket.
 * 
 * Used by @c #jcon_socketUnix_accept() .
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
static jcon_socket_t *jcon_socketUnix_clone(int fd, struct sockaddr_un socket_address, jlog_t *logger);

/**
 * @brief Frees session memory.
 * 
 * Called by @c #jcon_socket_free() .
 * 
 * @param session Session object to free.
 */
static void jcon_socketUnix_free(jcon_socket_t *session);

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
static int jcon_socketUnix_connect(jcon_socket_t *session);

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
static int jcon_socketUnix_bind(jcon_socket_t *session);

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
static jcon_socket_t *jcon_socketUnix_accept(jcon_socket_t *session);

/**
 * @brief Extract filename from address struct.
 * 
 * @param socket_address  Address struct.
 * 
 * @return                String with filename.
 * @return                @c NULL in case of error.
 */
static char *jcon_socketUnix_getFile(struct sockaddr_un socket_address);

/**
 * @brief Create reference string from socket address.
 * 
 * Reference string consists of connection type (Unix),
 * IP address ( @c #jcon_Unix_getIP() ) and port number
 * ( @c #jcon_Unix_getPort() ).
 * 
 * These items get combined into one string.
 * 
 * @param socket_address  Address struct.
 * 
 * @return                Allocated reference string.
 * @return                @c NULL in case of error.
 */
static char *jcon_socketUnix_createReferenceString(struct sockaddr_un socket_address);

/**
 * @brief Create reference string without address.
 * 
 * In the case of clients accepted by a server,
 * there is no valid address available.
 * 
 * In that case return the ref_string "UNIX:-"
 * 
 * @return  Allocated empty ref string.
 * @return  @c NULL in case of error.
 */
static char *jcon_socketUnix_createEmptyReferenceString();

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
static void jcon_socketUnix_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG /* Allow to turn of debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_socketUnix_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface funcions.
//

//------------------------------------------------------------------------------
//
jcon_socket_t *jcon_socketUnix_simple_init(const char *filepath, jlog_t *logger)
{
  jcon_socket_t *session = (jcon_socket_t *)malloc(sizeof(jcon_socket_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  if(filepath == NULL)
  {
    ERROR(NULL, "filepath is NULL.");
    free(session);
    return NULL;
  }

  session->function_connect = &jcon_socketUnix_connect;
  session->function_bind = &jcon_socketUnix_bind;
  session->function_close = NULL;
  session->function_accept = &jcon_socketUnix_accept;
  session->session_free_handler = &jcon_socketUnix_free;

  session->file_descriptor = 0;
  session->logger = logger;
  session->socket_type = JCON_SOCKETUNIX_CONNECTIONTYPE;
  session->connection_type = 0;

  jcon_socketUnix_ctx_t *ctx = (jcon_socketUnix_ctx_t *)malloc(sizeof(jcon_socketUnix_ctx_t));
  if(ctx == NULL)
  {
    ERROR(NULL, "malloc() failed. Destroying session.");
    free(session);
    return NULL;
  }

  if(strlen(filepath) >= sizeof(ctx->socket_address.sun_path))
  {
    ERROR(NULL, "Filepath too long.");
    free(session);
    free(ctx);
    return NULL;
  }

  ctx->socket_address.sun_family = AF_LOCAL;
  
  memset(ctx->socket_address.sun_path, 0, sizeof(ctx->socket_address.sun_path));
  memcpy(ctx->socket_address.sun_path, filepath, strlen(filepath));

  session->referenceString = jcon_socketUnix_createReferenceString(ctx->socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<Unix> jcon_SocketUnix_createReferenceString() failed. Destroying session.");
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
jcon_socket_t *jcon_socketUnix_clone(int fd, struct sockaddr_un socket_address, jlog_t *logger)
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
  session->session_free_handler = &jcon_socketUnix_free;

  session->file_descriptor = fd;
  session->logger = logger;
  session->connection_type = JCON_SOCKET_CONNECTIONTYPE_CLIENT;

  jcon_socketUnix_ctx_t *ctx = (jcon_socketUnix_ctx_t *)malloc(sizeof(jcon_socketUnix_ctx_t));
  if(ctx == NULL)
  {
    ERROR(NULL, "malloc() failed. Destroying session.");
    free(session);
    return NULL;
  }

  ctx->socket_address = socket_address;
  session->referenceString = jcon_socketUnix_createEmptyReferenceString();
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<Unix> json_socketUnix_createReferenceString() failed. Destroying context and session.");
    free(session);
    free(ctx);
    return NULL;
  }

  session->session_ctx = ctx;

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_socketUnix_free(jcon_socket_t *session)
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
int jcon_socketUnix_connect(jcon_socket_t *session)
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

  int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  struct sockaddr_un addr = ((jcon_socketUnix_ctx_t *)session->session_ctx)->socket_address;

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
int jcon_socketUnix_bind(jcon_socket_t *session)
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

  int fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  struct sockaddr_un addr = ((jcon_socketUnix_ctx_t *)session->session_ctx)->socket_address;

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
jcon_socket_t *jcon_socketUnix_accept(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  int new_fd;
  struct sockaddr_un new_addr;
  socklen_t addrlen = sizeof(struct sockaddr_un);

  new_fd = accept(session->file_descriptor, (struct sockaddr *)&new_addr, &addrlen);
  if(new_fd < 0)
  {
    ERROR(session, "accept() failed [%d : %s].", errno, strerror(errno));
    return NULL;
  }

  jcon_socket_t *new_con = jcon_socketUnix_clone(new_fd, new_addr, session->logger);
  if(new_con == NULL)
  {
    ERROR(session, "jcon_socketUnix_clone() failed with new connection [Unix:%s].", jcon_socketUnix_getFile(new_addr));
    close(new_fd);
    return NULL;
  }

  return new_con;
}

//------------------------------------------------------------------------------
//
char *jcon_socketUnix_getFile(struct sockaddr_un socket_address)
{
  char *file = socket_address.sun_path;
  if(file == NULL)
  {
    ERROR(NULL, "Session\'s address invalid.");
    return NULL;
  }

  return file;
}

//------------------------------------------------------------------------------
//
char *jcon_socketUnix_createReferenceString(struct sockaddr_un socket_address)
{
  char buf[128] = { 0 };
  char *file;
  
  file = jcon_socketUnix_getFile(socket_address);
  if(file == NULL)
  {
    ERROR(NULL, "jcon_client_tcp_getFile() failed.");
    return NULL;
  }

  if(sprintf(buf, "UNIX:%s", file) < 0)
  {
    ERROR(NULL, "sprintf() failed.");
    return NULL;
  }

  size_t size_refString = sizeof(char) * (strlen(buf) + 1);
  char *ret = (char *)malloc(size_refString);
  if(ret == NULL)
  {
    ERROR(NULL, "<UNIX:%s> malloc() failed.", file);
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
char *jcon_socketUnix_createEmptyReferenceString()
{
  const char *buf = "UNIX:-";

  size_t size_refString = sizeof(char) * (strlen(buf) + 1);
  char *ret = (char *)malloc(size_refString);
  if(ret == NULL)
  {
    ERROR(NULL, "<UNIX> malloc() failed.");
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
void jcon_socketUnix_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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
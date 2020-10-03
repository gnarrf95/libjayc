/**
 * @file jcon_unix.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jcon_unix.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_unix.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

//==============================================================================
// Define constants.
//

/**
 * @brief The connection type is not defined yet.
 * 
 * In case socket is not yet created or closed.
 */
#define JCON_UNIX_CONNECTIONTYPE_NOTDEF 0

/**
 * @brief Socket operates as client.
 * 
 * Forbids session from using function
 * @c #jcon_unix_accept() .
 */
#define JCON_UNIX_CONNECTIONTYPE_CLIENT 1

/**
 * @brief Socket operates as server.
 * 
 * Forbids session from using functions
 * @c #jcon_unix_recvData() and @c #jcon_unix_sendData() .
 */
#define JCON_UNIX_CONNECTIONTYPE_SERVER 2



//==============================================================================
// Define structures.
//

/**
 * @brief Session object.
 * 
 * Holds data for socket operation.
 */
struct __jcon_unix_session
{
  int file_descriptor;                /**< File descriptor for socket. */
  struct sockaddr_un socket_address;  /**< Address struct for socket. */
  int connection_type;                /**< Enumeration with values:
                                           * @c #jcon_unix_CONNECTIONTYPE_NETDEF ,
                                           * @c #jcon_unix_CONNECTIONTYPE_CLIENT ,
                                           * @c #jcon_unix_CONNECTIONTYPE_SERVER . */
  
  char *referenceString;              /**< Holds information about connection. */
  jlog_t *logger;                     /**< Logger to use for debug and error output. */
};



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Creates session from existing socket.
 * 
 * Used by @c #jcon_unix_accept() .
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
static jcon_unix_t *jcon_unix_clone(int fd, struct sockaddr_un socket_address, jlog_t *logger);

/**
 * @brief Extract uds filepath from address struct. 
 * 
 * @param socket_address  Address struct.
 * 
 * @return                File path string.
 * @return                @c NULL in case of error.
 */
static char *jcon_unix_getFile(struct sockaddr_un socket_address);

/**
 * @brief Create reference string from socket address.
 * 
 * Reference string consists of connection type (UNIX)
 * and the filepath of the uds file.
 * 
 * These items get combined into one string.
 * 
 * @param socket_address  Address struct.
 * 
 * @return                Allocated reference string.
 * @return                @c NULL in case of error.
 */
static char *jcon_unix_createReferenceString(struct sockaddr_un socket_address);

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
static char *jcon_unix_createEmptyReferenceString();

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
static void jcon_unix_log(jcon_unix_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG /* Allow to turn of debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_unix_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jcon_unix_t *jcon_unix_simple_init(const char *filepath, jlog_t *logger)
{
  jcon_unix_t *session = (jcon_unix_t *)malloc(sizeof(jcon_unix_t));
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

  if(strlen(filepath) >= sizeof(session->socket_address.sun_path))
  {
    ERROR(NULL, "Filepath too long.");
    free(session);
    return NULL;
  }

  session->file_descriptor = 0;
  session->logger = logger;
  session->connection_type = JCON_UNIX_CONNECTIONTYPE_NOTDEF;
  session->socket_address.sun_family = AF_LOCAL;
  
  memset(session->socket_address.sun_path, 0, sizeof(session->socket_address.sun_path));
  memcpy(session->socket_address.sun_path, filepath, strlen(filepath));

  session->referenceString = jcon_unix_createReferenceString(session->socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<UNIX> jcon_unix_createReferenceString() failed. Destroying session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_unix_free(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_unix_isConnected(session))
  {
    jcon_unix_close(session);
  }

  free(session->referenceString);
  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_unix_connect(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_unix_isConnected(session))
  {
    DEBUG(session, "Session is already connected.");
    return true;
  }

  int fd = socket(PF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(connect(fd, (struct sockaddr *)&session->socket_address, sizeof(session->socket_address)) < 0)
  {
    ERROR(session, "connect() failed [%d : %s]. Closing socket.", errno, strerror(errno));
    if(close(fd) < 0)
    {
      ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
    }
    return false;
  }

  session->file_descriptor = fd;
  session->connection_type = JCON_UNIX_CONNECTIONTYPE_CLIENT;
  return true;
}

//------------------------------------------------------------------------------
//
int jcon_unix_bind(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_unix_isConnected(session))
  {
    DEBUG(session, "Session is already connected.");
    return true;
  }

  int fd = socket(PF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0)
  {
    ERROR(session, "socket() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(unlink(session->socket_address.sun_path) < 0)
  {
    ERROR(session, "unlink() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(bind(fd, (struct sockaddr *)&session->socket_address, sizeof(session->socket_address)) < 0)
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
  session->connection_type = JCON_UNIX_CONNECTIONTYPE_SERVER;
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_unix_close(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_unix_isConnected(session) == false)
  {
    DEBUG(session, "Session is already closed.");
    return;
  }

  if(close(session->file_descriptor) < 0)
  {
    ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
  }

  session->file_descriptor = 0;
  session->connection_type = JCON_UNIX_CONNECTIONTYPE_NOTDEF;
}

//------------------------------------------------------------------------------
//
void jcon_unix_shutdown(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_unix_isConnected(session) == false)
  {
    DEBUG(session, "Session is already closed.");
    return;
  }

  if(session->connection_type != JCON_UNIX_CONNECTIONTYPE_CLIENT)
  {
    DEBUG(session, "Server socket cannot be shut down.");
    return;
  }

  if(shutdown(session->file_descriptor, SHUT_WR) < 0)
  {
    ERROR(session, "shutdown() failed [%d : %s].", errno, strerror(errno));
  }

  int wait = true;
  char buf[16];
  while(wait)
  {
    int ret = recv(session->file_descriptor, buf, sizeof(buf), 0);

    if(ret == 0)
    {
      DEBUG(session, "Socket shutdown successful.");
      wait = false;
    }
    else if(ret < 0)
    {
      ERROR(session, "recv() failed [%d : %s].", errno, strerror(errno));
      wait = false;
    }
  }
}

//------------------------------------------------------------------------------
//
int jcon_unix_pollForInput(jcon_unix_t *session, int timeout)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_unix_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return false;
  }

  struct pollfd poll_fds[1];
  poll_fds->fd = session->file_descriptor;
  poll_fds->events = POLLIN;

  int ret_poll = poll(poll_fds, 1, timeout);

  if(ret_poll < 0)
  {
    ERROR(session, "poll() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  if(ret_poll == 0)
  {
    return false;
  }

  int ret = true;

  if(poll_fds->revents & POLLERR)
  {
    DEBUG(session, "poll() recieved [POLLERR].");
    jcon_unix_close(session);
    ret = false;
  }
  if(poll_fds->revents & POLLNVAL)
  {
    DEBUG(session, "poll() recieved [POLLNVAL].");
    ret = false;
  }
  if(poll_fds->revents & POLLIN)
  {
    DEBUG(session, "poll() recieved [POLLIN].");
  }
  if(poll_fds->revents & POLLHUP)
  {
    DEBUG(session, "poll() recieved [POLLHUP].");
  }

  return ret;
}

//------------------------------------------------------------------------------
//
jcon_unix_t *jcon_unix_accept(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->connection_type != JCON_UNIX_CONNECTIONTYPE_SERVER)
  {
    ERROR(session, "Session is not of type server.");
    return 0;
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

  jcon_unix_t *new_con = jcon_unix_clone(new_fd, new_addr, session->logger);
  if(new_con == NULL)
  {
    ERROR(session, "jcon_unix_clone() failed with new connection [UNIX:%s].", jcon_unix_getFile(new_addr));
    close(new_fd);
    return NULL;
  }

  return new_con;
}

//------------------------------------------------------------------------------
//
size_t jcon_unix_recvData(jcon_unix_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_unix_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_UNIX_CONNECTIONTYPE_CLIENT)
  {
    ERROR(session, "Session is not of type client.");
    return 0;
  }

  if(data_size == 0)
  {
    ERROR(session, "data_size given is [0].");
    return 0;
  }

  void *buf = malloc(data_size);
  if(buf == NULL)
  {
    ERROR(session, "malloc() failed.");
    return 0;
  }

  int ret_recv = recv(session->file_descriptor, buf, data_size, 0);
  if(ret_recv < 0)
  {
    ERROR(session, "recv() failed [%d : %s].", errno, strerror(errno));
    free(buf);
    return 0;
  }

  if(ret_recv == 0)
  {
    DEBUG(session, "recv() returned [0]. Closing connection.");
    jcon_unix_close(session);
    free(buf);
    return 0;
  }

  size_t cpy_size = ret_recv;

  /* Potential for buffer overflow. Checking and trimming data if necessary. */
  if(ret_recv > data_size)
  {
    DEBUG(session, "Buffer overflow detected [%d > %d]. Trimming data.", ret_recv, data_size);
    cpy_size = data_size;
  }

  if(data_ptr)
  {
    memcpy(data_ptr, buf, cpy_size);
  }
  free(buf);

  return cpy_size;
}

//------------------------------------------------------------------------------
//
size_t jcon_unix_sendData(jcon_unix_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_unix_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_UNIX_CONNECTIONTYPE_CLIENT)
  {
    ERROR(session, "Session is not of type client.");
    return 0;
  }

  if(data_ptr == NULL)
  {
    ERROR(session, "data_ptr is NULL.");
    return 0;
  }

  if(data_size == 0)
  {
    ERROR(session, "data_size given is [0].");
    return 0;
  }

  /* Fixed broken pipe termination, by preventing SIGPIPE. */
  int ret_send = send(session->file_descriptor, data_ptr, data_size, MSG_NOSIGNAL);
  if(ret_send < 0)
  {
    if(errno == ECONNRESET || errno == EPIPE)
    {
      jcon_unix_close(session);
    }
    else
    {
      ERROR(session, "send() failed [%d : %s].", errno, strerror(errno));
    }
    return 0;
  }

  return ret_send;
}

//------------------------------------------------------------------------------
//
int jcon_unix_isConnected(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return (session->file_descriptor > 0);
}

//------------------------------------------------------------------------------
//
const char *jcon_unix_getReferenceString(jcon_unix_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  return session->referenceString;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
jcon_unix_t *jcon_unix_clone(int fd, struct sockaddr_un socket_address, jlog_t *logger)
{
  if(fd <= 0)
  {
    ERROR(NULL, "Invalid file descriptor [%d].", fd);
    return NULL;
  }

  jcon_unix_t *session = (jcon_unix_t *)malloc(sizeof(jcon_unix_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->file_descriptor = fd;
  session->logger = logger;
  session->socket_address = socket_address;
  session->connection_type = JCON_UNIX_CONNECTIONTYPE_CLIENT;
  session->socket_address.sun_family = socket_address.sun_family;

  session->referenceString = jcon_unix_createEmptyReferenceString();
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<TCP> json_client_tcp_createEmptyReferenceString() failed. Destroying context and session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
char *jcon_unix_getFile(struct sockaddr_un socket_address)
{
  char *file = socket_address.sun_path;
  if(file == NULL)
  {
    ERROR(NULL, "Session\' address invalid.");
    return NULL;
  }

  return file;
}

//------------------------------------------------------------------------------
//
char *jcon_unix_createReferenceString(struct sockaddr_un socket_address)
{
  char buf[128] = { 0 };
  char *file;
  
  file = jcon_unix_getFile(socket_address);
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
char *jcon_unix_createEmptyReferenceString()
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
void jcon_unix_log(jcon_unix_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_unix_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_unix_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
/**
 * @file jcon_socket.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jcon_socket.
 * 
 * @date 2020-10-05
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_socket.h>
#include <jayc/jcon_socket_dev.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

//==============================================================================
// Define Log function and macros.
//

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
static void jcon_socket_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

#ifdef JCON_NO_DEBUG /* Allow to turn of debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_socket_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
void jcon_socket_free(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_socket_isConnected(session))
  {
    jcon_socket_close(session);
  }

  if(session->session_free_handler)
  {
    session->session_free_handler(session);
  }

  free(session->referenceString);
  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_socket_connect(jcon_socket_t *session)
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

  if(session->function_connect == NULL)
  {
    ERROR(session, "function_connect() is NULL.");
    return false;
  }

  if(session->function_connect(session) == false)
  {
    DEBUG(session, "function_connect() failed.");
    return false;
  }

  session->connection_type = JCON_SOCKET_CONNECTIONTYPE_CLIENT;
  return true;
}

//------------------------------------------------------------------------------
//
int jcon_socket_bind(jcon_socket_t *session)
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

  if(session->function_bind == NULL)
  {
    ERROR(session, "function_bind() is NULL.");
    return false;
  }

  if(session->function_bind(session) == false)
  {
    DEBUG(session, "function_bind() failed.");
    return false;
  }

  session->connection_type = JCON_SOCKET_CONNECTIONTYPE_SERVER;
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_socket_close(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_socket_isConnected(session) == false)
  {
    DEBUG(session, "Session is already closed.");
    return;
  }

  if(session->function_close)
  {
    session->function_close(session);
  }

  if(close(session->file_descriptor) < 0)
  {
    ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
  }

  session->file_descriptor = 0;
  session->connection_type = JCON_SOCKET_CONNECTIONTYPE_NOTDEF;
}

//------------------------------------------------------------------------------
//
int jcon_socket_pollForInput(jcon_socket_t *session, int timeout)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_socket_isConnected(session) == false)
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
    jcon_socket_close(session);
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
jcon_socket_t *jcon_socket_accept(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->connection_type != JCON_SOCKET_CONNECTIONTYPE_SERVER)
  {
    ERROR(session, "Session is not of type server.");
    return NULL;
  }

  if(jcon_socket_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return NULL;
  }

  if(session->function_accept == NULL)
  {
    ERROR(session, "function_accept() is NULL.");
    return NULL;
  }

  jcon_socket_t *new_session = session->function_accept(session);
  if(new_session == NULL)
  {
    DEBUG(session, "function_accept() failed.");
    return NULL;
  }

  return new_session;
}

//------------------------------------------------------------------------------
//
size_t jcon_socket_recvData(jcon_socket_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_socket_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_SOCKET_CONNECTIONTYPE_CLIENT)
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
    jcon_socket_close(session);
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
size_t jcon_socket_sendData(jcon_socket_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_socket_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_SOCKET_CONNECTIONTYPE_CLIENT)
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
      jcon_socket_close(session);
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
int jcon_socket_isConnected(jcon_socket_t *session)
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
const char *jcon_socket_getSocketType(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  return session->socket_type;
}

//------------------------------------------------------------------------------
//
const char *jcon_socket_getReferenceString(jcon_socket_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  return session->referenceString;
}



//==============================================================================
// Implement log function.
//

//------------------------------------------------------------------------------
//
void jcon_socket_log(jcon_socket_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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
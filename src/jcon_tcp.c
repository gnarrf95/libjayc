/**
 * @file jcon_tcp.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-28
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_tcp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define JCON_TCP_CONNECTIONTYPE_NOTDEF 0
#define JCON_TCP_CONNECTIONTYPE_CLIENT 1
#define JCON_TCP_CONNECTIONTYPE_SERVER 2

struct __jcon_tcp_session
{
  int file_descriptor;
  struct sockaddr_in socket_address;
  int connection_type;
  
  char *referenceString;
  jlog_t *logger;
};

static char *jcon_tcp_getIP(struct sockaddr_in socket_address);
static uint16_t jcon_tcp_getPort(struct sockaddr_in socket_address);

static char *jcon_tcp_createReferenceString(struct sockaddr_in socket_address);

static void jcon_tcp_log(jcon_tcp_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

#define DEBUG(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_tcp_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//------------------------------------------------------------------------------
//
void jcon_tcp_log(jcon_tcp_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_tcp_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_tcp_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}

//------------------------------------------------------------------------------
//
jcon_tcp_t *jcon_tcp_simple_init(const char *address, uint16_t port, jlog_t *logger)
{
  jcon_tcp_t *session = (jcon_tcp_t *)malloc(sizeof(jcon_tcp_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->file_descriptor = 0;
  session->logger = logger;
  session->connection_type = JCON_TCP_CONNECTIONTYPE_NOTDEF;

  session->socket_address.sin_family = AF_INET;
  session->socket_address.sin_port = htons(port);

  struct hostent *hostinfo;
  hostinfo = gethostbyname(address);
  if(hostinfo == NULL)
  {
    ERROR(NULL, "<TCP:%s:%u> gethostbyname() failed. Destroying context and session.", address, port);
    free(session);
    return NULL;
  }
  session->socket_address.sin_addr = *(struct in_addr *)hostinfo->h_addr_list[0];

  session->referenceString = jcon_tcp_createReferenceString(session->socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<TCP> json_client_tcp_createReferenceString() failed. Destroying context and session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
jcon_tcp_t *jcon_tcp_clone(int fd, struct sockaddr_in socket_address, jlog_t *logger)
{
  if(fd <= 0)
  {
    ERROR(NULL, "Invalid file descriptor [%d].", fd);
    return NULL;
  }

  jcon_tcp_t *session = (jcon_tcp_t *)malloc(sizeof(jcon_tcp_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->file_descriptor = fd;
  session->socket_address = socket_address;
  session->logger = logger;
  session->connection_type = JCON_TCP_CONNECTIONTYPE_NOTDEF;

  session->referenceString = jcon_tcp_createReferenceString(session->socket_address);
  if(session->referenceString == NULL)
  {
    ERROR(NULL, "<TCP> json_client_tcp_createReferenceString() failed. Destroying context and session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_tcp_free(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_tcp_isConnected(session))
  {
    jcon_tcp_close(session);
  }

  free(session->referenceString);
  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_tcp_connect(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_tcp_isConnected(session))
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
  session->connection_type = JCON_TCP_CONNECTIONTYPE_CLIENT;
  return true;
}

//------------------------------------------------------------------------------
//
int jcon_tcp_bind(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_tcp_isConnected(session))
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
  session->connection_type = JCON_TCP_CONNECTIONTYPE_SERVER;
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_tcp_close(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_tcp_isConnected(session) == false)
  {
    DEBUG(session, "Session is already closed.");
    return;
  }

  if(close(session->file_descriptor) < 0)
  {
    ERROR(session, "close() failed [%d : %s].", errno, strerror(errno));
  }

  session->file_descriptor = 0;
  session->connection_type = JCON_TCP_CONNECTIONTYPE_NOTDEF;
}

//------------------------------------------------------------------------------
//
int jcon_tcp_pollForInput(jcon_tcp_t *session, int timeout)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_tcp_isConnected(session) == false)
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
    jcon_tcp_close(session);
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
int jcon_tcp_isConnected(jcon_tcp_t *session)
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
jcon_tcp_t *jcon_tcp_accept(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->connection_type != JCON_TCP_CONNECTIONTYPE_SERVER)
  {
    ERROR(session, "Session is not of type server.");
    return 0;
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

  jcon_tcp_t *new_con = jcon_tcp_clone(new_fd, new_addr, session->logger);
  if(new_con == NULL)
  {
    ERROR(session, "jcon_tcp_clone() failed with new connection [TCP:%s:%u].", jcon_tcp_getIP(new_addr), jcon_tcp_getPort(new_addr));
    close(new_fd);
    return NULL;
  }

  return new_con;
}

//------------------------------------------------------------------------------
//
size_t jcon_tcp_recvData(jcon_tcp_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_tcp_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_TCP_CONNECTIONTYPE_CLIENT)
  {
    ERROR(session, "Session is not of type client.");
    return 0;
  }

  if(data_size == 0)
  {
    ERROR(session, "data_size given is [0].");
    return 0;
  }

  char *buf = (char *)malloc(data_size);
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
    jcon_tcp_close(session);
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
size_t jcon_tcp_sendData(jcon_tcp_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  if(jcon_tcp_isConnected(session) == false)
  {
    ERROR(session, "Session is not connected.");
    return 0;
  }

  if(session->connection_type != JCON_TCP_CONNECTIONTYPE_CLIENT)
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
    ERROR(session, "send() failed [%d : %s].", errno, strerror(errno));
    return 0;
  }

  return ret_send;
}

//------------------------------------------------------------------------------
//
char *jcon_tcp_getIP(struct sockaddr_in socket_address)
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
uint16_t jcon_tcp_getPort(struct sockaddr_in socket_address)
{
  return ntohs(socket_address.sin_port);
}

//------------------------------------------------------------------------------
//
char *jcon_tcp_createReferenceString(struct sockaddr_in socket_address)
{
  char buf[128] = { 0 };
  char *ip;
  uint16_t port;
  
  ip = jcon_tcp_getIP(socket_address);
  if(ip == NULL)
  {
    ERROR(NULL, "jcon_client_tcp_getIP() failed.");
    return NULL;
  }

  port = jcon_tcp_getPort(socket_address);
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
const char *jcon_tcp_getReferenceString(jcon_tcp_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  return session->referenceString;
}
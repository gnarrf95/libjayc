/**
 * @file jcon_system.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-24
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for nanosleep() */

#include <jcon_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define JCON_SYSTEM_LOOPSLEEP_DEFAULT 100000000

static void jcon_system_log(jcon_system_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

static void *jcon_system_control_function(void *ctx);

static void jcon_system_loop_sleep(jcon_system_t *session);

static void jcon_system_connectionThread_create(void *ctx, int create_type, const char *reference_string);
static void jcon_system_connectionThread_close(void *ctx, int close_type, const char *reference_string);

static int jcon_system_pthread_init(jcon_system_t *session);
static void jcon_system_pthread_join(jcon_system_t *session);

static int jcon_system_pthread_mutex_init(jcon_system_t *session);
static void jcon_system_pthread_mutex_destroy(jcon_system_t *session);

static void jcon_system_pthread_mutex_lock(jcon_system_t *session);
static void jcon_system_pthread_mutex_unlock(jcon_system_t *session);

#define DEBUG(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//------------------------------------------------------------------------------
//
jcon_system_t *jcon_system_init
(
  jcon_server_t *server,
  jcon_system_threadData_handler_t data_handler,
  jcon_system_threadCreate_handler_t create_handler,
  jcon_system_threadClose_handler_t close_handler,
  jlog_t *logger,
  void *ctx
)
{
  if(server == NULL)
  {
    ERROR(NULL,"Server is NULL.");
    return NULL;
  }

  jcon_system_t *session = (jcon_system_t *)malloc(sizeof(jcon_system_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->server = server;
  session->connections = NULL;
  session->loop_sleep = JCON_SYSTEM_LOOPSLEEP_DEFAULT;
  session->control_run = false;
  session->data_handler = data_handler;
  session->create_handler = create_handler;
  session->close_handler = close_handler;
  session->logger = logger;
  session->session_context = ctx;

  if(jcon_system_control_start(session) == false)
  {
    ERROR(session, "jcon_system_control_start() failed. Destroying session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_system_free(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jcon_system_control_stop(session);
  jcon_system_clearConnections(session);

  free(session);
}

//------------------------------------------------------------------------------
//
const char *jcon_system_getConnectionType(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return NULL;
  }

  return jcon_server_getConnectionType(session->server);
}

//------------------------------------------------------------------------------
//
const char *jcon_system_getReferenceString(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return NULL;
  }

  return jcon_server_getReferenceString(session->server);
}

//------------------------------------------------------------------------------
//
int jcon_system_isServerOpen(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return false;
  }

  return jcon_server_isOpen(session->server);
}

//------------------------------------------------------------------------------
//
int jcon_system_resetServer(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return false;
  }

  return jcon_server_reset(session->server);
}

//------------------------------------------------------------------------------
//
void jcon_system_closeServer(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return;
  }

  jcon_server_close(session->server);
}

//------------------------------------------------------------------------------
//
int jcon_system_control_isRunning(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return session->control_run;
}

//------------------------------------------------------------------------------
//
int jcon_system_control_start(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL");
    return false;
  }

  if(session->control_run)
  {
    WARN(session, "Control thread is already running.");
    return true;
  }

  if(jcon_system_pthread_mutex_init(session) == false)
  {
    ERROR(session, "pthread_mutex could not be initialized.");
    return false;
  }

  jcon_system_pthread_mutex_lock(session);
  if(jcon_system_pthread_init(session) == false)
  {
    ERROR(session, "pthread could not be initialized.");
    jcon_system_pthread_mutex_unlock(session);
    jcon_system_pthread_mutex_destroy(session);
    return false;
  }

  session->control_run = true;
  if(session->create_handler)
  {
    session->create_handler(
      session->session_context,
      jcon_system_getReferenceString(session)
    );
  }
  jcon_system_pthread_mutex_unlock(session);

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_system_control_stop(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jcon_system_pthread_mutex_lock(session);
  session->control_run = false;
  jcon_system_pthread_mutex_unlock(session);

  jcon_system_pthread_join(session);
  jcon_system_pthread_mutex_destroy(session);
}

//------------------------------------------------------------------------------
//
size_t jcon_system_getConnectionNumber(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  return jutil_linkedlist_size(session->connections);
}

//------------------------------------------------------------------------------
//
void jcon_system_clearConnections(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_linkedlist_t *itr = session->connections;
  while(itr != NULL)
  {
    DEBUG(session, "Destroying node [%p].", itr);
    jutil_linkedlist_t *tmp = itr;
    itr = jutil_linkedlist_iterate(itr);
    jcon_system_freeConnection(session, tmp);
  }
}

//------------------------------------------------------------------------------
//
int jcon_system_addConnection(jcon_system_t *session, jcon_client_t *client)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(client == NULL)
  {
    ERROR(session, "Client is NULL.");
    return false;
  }

  jcon_system_connection_t *new_connection = (jcon_system_connection_t *)malloc(sizeof(jcon_system_connection_t));
  if(new_connection == NULL)
  {
    ERROR(session, "malloc() failed.");
    return false;
  }

  new_connection->client = client;
  new_connection->thread = jcon_thread_init
  (
    client,
    session->data_handler,
    jcon_system_connectionThread_create,
    jcon_system_connectionThread_close,
    session->logger,
    NULL
  );
  if(new_connection->thread == NULL)
  {
    ERROR(session, "jcon_thread_init() failed.");
    free(new_connection);
    return false;
  }

  session->connections = jutil_linkedlist_append(session->connections, (void *)new_connection);

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_system_freeConnection(jcon_system_t *session, jutil_linkedlist_t *connection_node)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(connection_node == NULL)
  {
    ERROR(session, "Connection Node is NULL.");
    return;
  }

  jcon_system_connection_t *connection = (jcon_system_connection_t *)jutil_linkedlist_getData(connection_node);
  if(connection)
  {
    if(connection->thread)
    {
      jcon_thread_free(connection->thread);
    }
    if(connection->client)
    {
      DEBUG(session, "Freeing client [%s].", jcon_client_getReferenceString(connection->client));
      jcon_client_session_free(connection->client);
    }

    free(connection);
  }

  session->connections = jutil_linkedlist_removeNode(session->connections, connection_node);
}

//------------------------------------------------------------------------------
//
void jcon_system_log(jcon_system_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session)
  {
    if(session->logger && session->server)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_system_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_system_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}

//------------------------------------------------------------------------------
//
void *jcon_system_control_function(void *ctx)
{
  if(ctx == NULL)
  {
    CRITICAL(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_system_t *session = (jcon_system_t *)ctx;
  int run_loop;

  jcon_system_pthread_mutex_lock(session);
  run_loop = session->control_run;
  jcon_system_pthread_mutex_unlock(session);

  while(run_loop)
  {
    /* Check for closed connections. */
    jcon_system_pthread_mutex_lock(session);
    jutil_linkedlist_t *itr = session->connections;
    while(itr != NULL)
    {
      jcon_system_connection_t *connection;
      if( (connection = (jcon_system_connection_t *)jutil_linkedlist_getData(itr)) )
      {
        if(jcon_thread_isRunning(connection->thread) == false)
        {
          jcon_system_freeConnection(session, itr);
          break;
        }
        else
        {
          itr = jutil_linkedlist_iterate(itr);
        }
      }
      else
      {
        session->connections = jutil_linkedlist_removeNode(session->connections, itr);
        break;
      }
    }
    jcon_system_pthread_mutex_unlock(session);

    /* Check for new connections. */
    jcon_system_pthread_mutex_lock(session);
    if(jcon_server_newConnection(session->server))
    {
      jcon_client_t *new_client = jcon_server_acceptConnection(session->server);
      if(new_client)
      {
        if(jcon_system_addConnection(session, new_client) == false)
        {
          ERROR(session, "jcon_system_addConnection() failed.");
        }
      }
      else
      {
        ERROR(session, "jcon_server_acceptConnection() failed.");
      }
    }
    jcon_system_pthread_mutex_unlock(session);

    jcon_system_loop_sleep(session);

    jcon_system_pthread_mutex_lock(session);
    run_loop = session->control_run;
    jcon_system_pthread_mutex_unlock(session);
  }

  DEBUG(session, "Control thread stopped.");
  return NULL;
}

//------------------------------------------------------------------------------
//
void jcon_system_loop_sleep(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  struct timespec slp;
  slp.tv_sec = 0;
  slp.tv_nsec = session->loop_sleep;

  int ret_sleep = nanosleep(&slp, NULL);
  if(ret_sleep)
  {
    ERROR(session, "nanosleep() failed [%d : %s].", errno, strerror(errno));
  }
}

//------------------------------------------------------------------------------
//
void jcon_system_connectionThread_create(void *ctx, int create_type, const char *reference_string)
{
}

//------------------------------------------------------------------------------
//
void jcon_system_connectionThread_close(void *ctx, int close_type, const char *reference_string)
{
}

//------------------------------------------------------------------------------
//
int jcon_system_pthread_init(jcon_system_t *session)
{
  int error = pthread_create(&session->control_thread, NULL, &jcon_system_control_function, session);
  if(error)
  {
    switch(error)
    {
      case EAGAIN:
      {
        CRITICAL(session, "pthread_create() failed [%d : %s]. Resource limit.", error, strerror(error));
        break;
      }

      case EINVAL:
      {
        ERROR(session, "pthread_create() failed [%d : %s]. Invalid attributes.", error, strerror(error));
        break;
      }

      case EPERM:
      {
        ERROR(session, "pthread_create() failed [%d : %s]. Missing permission.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_create() failed [%d : %s].", error, strerror(error));
        break;
      }
    }

    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_system_pthread_join(jcon_system_t *session)
{
  int error = pthread_join(session->control_thread, NULL);
  if(error)
  {
    switch(error)
    {
      case EDEADLK:
      {
        ERROR(session, "pthread_join() failed [%d : %s] Deadlock detected..", error, strerror(error));
        break;
      }

      case EINVAL:
      {
        ERROR(session, "pthread_join() failed [%d : %s] Thread not joinable.", error, strerror(error));
        break;
      }

      case ESRCH:
      {
        ERROR(session, "pthread_join() failed [%d : %s] Invalid thread-ID.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_join() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}

//------------------------------------------------------------------------------
//
int jcon_system_pthread_mutex_init(jcon_system_t *session)
{
  int error = pthread_mutex_init(&session->mutex, NULL);
  if(error)
  {
    switch(error)
    {
      case EAGAIN:
      {
        ERROR(session, "pthread_mutex_init() failed [%d : %s]. Missing resources.", error, strerror(error));
        break;
      }

      case ENOMEM:
      {
        ERROR(session, "pthread_mutex_init() failed [%d : %s]. Insufficient memory.", error, strerror(error));
        break;
      }

      case EPERM:
      {
        ERROR(session, "pthread_mutex_init() failed [%d : %s]. Missing privilege.", error, strerror(error));
        break;
      }

      case EBUSY:
      {
        WARN(session, "pthread_mutex_init() failed [%d : %s]. Mutex already initialized.", error, strerror(error));
        return true;
        break;
      }

      case EINVAL:
      {
        ERROR(session, "pthread_mutex_init() failed [%d : %s]. Invalid attributes.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_mutex_init() failed [%d : %s].", error, strerror(error));
        break;
      }
    }

    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_system_pthread_mutex_destroy(jcon_system_t *session)
{
  int error = pthread_mutex_destroy(&session->mutex);
  if(error)
  {
    switch(error)
    {
      case EBUSY:
      {
        WARN(session, "pthread_mutex_destroy() failed [%d : %s]. Mutex is locked.", error, strerror(error));
        break;
      }

      case EINVAL:
      {
        ERROR(session, "pthread_mutex_destroy() failed [%d : %s].", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_mutex_destroy() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}

//------------------------------------------------------------------------------
//
void jcon_system_pthread_mutex_lock(jcon_system_t *session)
{
  int error = pthread_mutex_lock(&session->mutex);
  if(error)
  {
    switch(error)
    {
      case EINVAL:
      {
        ERROR(session, "pthread_mutex_lock() failed [%d : %s]. Invalid priority.", error, strerror(error));
        break;
      }

      case EAGAIN:
      {
        ERROR(session, "pthread_mutex_lock() failed [%d : %s]. Max number of recursive locks.", error, strerror(error));
        break;
      }

      case EDEADLK:
      {
        ERROR(session, "pthread_mutex_lock() failed [%d : %s]. Current thread already owns mutex.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_mutex_lock() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}

//------------------------------------------------------------------------------
//
void jcon_system_pthread_mutex_unlock(jcon_system_t *session)
{
  int error = pthread_mutex_unlock(&session->mutex);
  if(error)
  {
    switch(error)
    {
      case EINVAL:
      {
        ERROR(session, "pthread_mutex_unlock() failed [%d : %s]. Invalid mutex.", error, strerror(error));
        break;
      }

      case EAGAIN:
      {
        ERROR(session, "pthread_mutex_unlock() failed [%d : %s]. Max number of recursive locks.", error, strerror(error));
        break;
      }

      case EPERM:
      {
        ERROR(session, "pthread_mutex_unlock() failed [%d : %s]. Current thread does not own the mutex.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(session, "pthread_mutex_unlock() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}
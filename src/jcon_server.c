/**
 * @file jcon_server.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_server_dev.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>

//==============================================================================
// Define log macros.
//==============================================================================

#define DEBUG(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_server_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//==============================================================================
// Declare internal functions.
//==============================================================================

static void *jcon_server_control_threadFunction(void *session_ptr);

static void jcon_server_clearConnections(jcon_server_t *session);

static int jcon_server_control_start(jcon_server_t *session);
static void jcon_server_control_stop(jcon_server_t *session);
static int jcon_server_control_isRunning(jcon_server_t *session);

static int jcon_server_pthread_init(jcon_server_t *session);
static void jcon_server_pthread_join(jcon_server_t *session);

static int jcon_server_pthread_mutex_init(jcon_server_t *session);
static void jcon_server_pthread_mutex_destroy(jcon_server_t *session);

static void jcon_server_pthread_mutex_lock(jcon_server_t *session);
static void jcon_server_pthread_mutex_unlock(jcon_server_t *session);

//==============================================================================
// Implement interface functions.
//==============================================================================

//------------------------------------------------------------------------------
//
void jcon_server_free(jcon_server_t *session)
{
  jcon_server_control_stop(session);
  jcon_server_clearConnections(session);

  if(session->server_free_handler)
  {
    session->server_free_handler(session->implementation_context);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
const char *jcon_server_getConnectionType(jcon_server_t *session)
{
  return session->connection_type;
}

//------------------------------------------------------------------------------
//
const char *jcon_server_getReferenceString(jcon_server_t *session)
{
  if(session->server_getReferenceString_handler)
  {
    return session->server_getReferenceString_handler(session->implementation_context);
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
int jcon_server_reset(jcon_server_t *session)
{
  jcon_server_control_stop(session);

  if(session->server_reset_handler)
  {
    if(session->server_reset_handler(session->implementation_context) == false)
    {
      return false;
    }
  }

  return jcon_server_control_start(session);
}

//------------------------------------------------------------------------------
//
void jcon_server_close(jcon_server_t *session)
{
  jcon_server_control_stop(session);

  if(session->server_close_handler)
  {
    session->server_close_handler(session->implementation_context);
  }
}

//------------------------------------------------------------------------------
//
size_t jcon_server_getConnectionNumber(jcon_server_t *session)
{
  return jcon_linked_list_size(session->connections);
}

//==============================================================================
// Implement internal functions.
//==============================================================================

//------------------------------------------------------------------------------
//
void *jcon_server_control_threadFunction(void *session_ptr)
{
  int run_loop;

  if(session_ptr == NULL)
  {
    CRITICAL(NULL, "session_ptr is NULL.");
    return NULL;
  }

  jcon_server_t *session = (jcon_server_t *)session_ptr;

  jcon_server_pthread_mutex_lock(session);
  run_loop = session->run;
  jcon_server_pthread_mutex_unlock(session);

  while(run_loop)
  {
    jcon_linked_list_node_t *iterator;

    /* Remove disconnected clients. */
    jcon_server_pthread_mutex_lock(session);
    iterator = session->connections;
    while(iterator != NULL)
    {
      jcon_thread_t *client = (jcon_thread_t *)iterator;
      if(jcon_thread_isRunning(client) == false)
      {
        jcon_thread_free(client);
        iterator = jcon_linked_list_delete(session->connections, iterator);
      }
      else
      {
        iterator = jcon_linked_list_next(iterator);
      }
    }
    jcon_server_pthread_mutex_unlock(session);

    /* Check for new connections. */
    jcon_server_pthread_mutex_lock(session);
    if(session->server_listen_handler(session->implementation_context))
    {
      jcon_client_t *new_connection = session->server_accept_handler(session->implementation_context);
      if(new_connection == NULL)
      {
        ERROR(session, "New connection could not be accepted.");
      }
      else
      {
        jcon_thread_t *new_thread = jcon_thread_init
        (
          new_connection,
          session->data_handler,
          session->create_handler,
          session->close_handler,
          session->logger,
          (void *)new_connection
        );
        if(new_thread == NULL)
        {
          ERROR(session, "New connection thread could not be initialized.");
        }
        else
        {
          jcon_linked_list_insert(session->connections, (void *)new_thread);
        }
      }
    }
    jcon_server_pthread_mutex_unlock(session);

    /* Check for termination. */
    jcon_server_pthread_mutex_lock(session);
    run_loop = session->run;
    jcon_server_pthread_mutex_unlock(session);
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
void jcon_server_log(jcon_server_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_server_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_server_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}

//------------------------------------------------------------------------------
//
void jcon_server_clearConnections(jcon_server_t *session)
{
  jcon_linked_list_node_t *iterator = session->connections;

  while(iterator != NULL)
  {
    iterator = jcon_linked_list_delete(iterator, iterator);
  }

  session->connections = NULL;
}

//------------------------------------------------------------------------------
//
int jcon_server_control_start(jcon_server_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jcon_server_control_isRunning(session))
  {
    WARN(session, "Control thread is already running.");
    return true;
  }

  if(jcon_server_pthread_mutex_init(session) == false)
  {
    ERROR(session, "pthread_mutex could not be initialized.");
    return false;
  }

  jcon_server_pthread_mutex_lock(session);
  if(jcon_server_pthread_init(session) == false)
  {
    ERROR(session, "pthread could not be created.");
    jcon_server_pthread_mutex_unlock(session);
    jcon_server_pthread_mutex_destroy(session);
    return false;
  }

  session->run = true;
  jcon_server_pthread_mutex_unlock(session);
  
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_server_control_stop(jcon_server_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_server_control_isRunning(session) == false)
  {
    WARN(session, "Control thread is not running.");
    return;
  }

  jcon_server_pthread_mutex_lock(session);
  session->run = false;
  jcon_server_pthread_mutex_unlock(session);

  jcon_server_pthread_join(session);
  jcon_server_pthread_mutex_destroy(session);
}

//------------------------------------------------------------------------------
//
int jcon_server_control_isRunning(jcon_server_t *session)
{
  int ret;

  jcon_server_pthread_mutex_lock(session);
  ret = session->run;
  jcon_server_pthread_mutex_unlock(session);

  return ret;
}

//------------------------------------------------------------------------------
//
int jcon_server_pthread_init(jcon_server_t *session)
{
  int error = pthread_create(&session->control_thread, NULL, &jcon_server_control_threadFunction, session);
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
void jcon_server_pthread_join(jcon_server_t *session)
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
int jcon_server_pthread_mutex_init(jcon_server_t *session)
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
void jcon_server_pthread_mutex_destroy(jcon_server_t *session)
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
void jcon_server_pthread_mutex_lock(jcon_server_t *session)
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
void jcon_server_pthread_mutex_unlock(jcon_server_t *session)
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
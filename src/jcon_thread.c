/**
 * @file jcon_thread.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements jcon_thread.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for nanosleep() */

#include <jcon_thread.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Holds data necessary for jcon_thread runtime.
 */
typedef struct __jcon_thread_runtime_data jcon_thread_runtime_data_t;

#define JCON_THREAD_LOOPSLEEP_DEFAULT 100000000

struct __jcon_thread_runtime_data
{
  jcon_client_t *client;
  pthread_mutex_t mutex;
  long loop_sleep;                          /**< How long the loop will wait (in nanoseconds), before continuing. */
  int run;

  jcon_thread_data_handler_t data_handler;
  jcon_thread_create_handler_t create_handler;
  jcon_thread_close_handler_t close_handler;

  jlog_t *logger;
  void *session_context;
};

struct __jcon_thread_session
{
  pthread_t thread;
  jcon_thread_runtime_data_t runtime_data;
};

static void *jcon_thread_run_function(void *session_ptr);

static void jcon_thread_loop_sleep(jcon_thread_t *session);

static void jcon_thread_log(jcon_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

static int jcon_thread_start(jcon_thread_t *session);

static void jcon_thread_stop(jcon_thread_t *session);

static int jcon_thread_pthread_init(jcon_thread_t *session);
static void jcon_thread_pthread_join(jcon_thread_t *session);

static int jcon_thread_pthread_mutex_init(jcon_thread_t *session);
static void jcon_thread_pthread_mutex_destroy(jcon_thread_t *session);

static void jcon_thread_pthread_mutex_lock(jcon_thread_t *session);
static void jcon_thread_pthread_mutex_unlock(jcon_thread_t *session);

//==============================================================================
// Define log macros.
//==============================================================================

#define DEBUG(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//==============================================================================
// Implement interface functions.
//==============================================================================

//------------------------------------------------------------------------------
//
jcon_thread_t *jcon_thread_init
(
  jcon_client_t *client,
  jcon_thread_data_handler_t data_handler,
  jcon_thread_create_handler_t create_handler,
  jcon_thread_close_handler_t close_handler,
  jlog_t *logger,
  void *ctx
)
{
  if(client == NULL)
  {
    ERROR(NULL, "Client is NULL.");
    return NULL;
  }

  jcon_thread_t *session = (jcon_thread_t *)malloc(sizeof(jcon_thread_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->runtime_data.client = client;
  session->runtime_data.run = false;
  session->runtime_data.loop_sleep = JCON_THREAD_LOOPSLEEP_DEFAULT;
  
  session->runtime_data.data_handler = data_handler;
  session->runtime_data.create_handler = create_handler;
  session->runtime_data.close_handler = close_handler;

  session->runtime_data.logger = logger;
  session->runtime_data.session_context = ctx;

  if(jcon_thread_start(session) == false)
  {
    ERROR(session, "jcon_thread_start() failed. Destroying session.");
    jcon_thread_free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_thread_free(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_thread_isRunning(session))
  {
    jcon_thread_stop(session);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_thread_isRunning(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  int ret;

  jcon_thread_pthread_mutex_lock(session);
  ret = session->runtime_data.run;
  jcon_thread_pthread_mutex_unlock(session);

  return ret;
}

//------------------------------------------------------------------------------
//
const char *jcon_thread_getConnectionType(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return jcon_client_getConnectionType(session->runtime_data.client);
}

//------------------------------------------------------------------------------
//
const char *jcon_thread_getReferenceString(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return jcon_client_getReferenceString(session->runtime_data.client);
}

//==============================================================================
// Implement internal functions.
//==============================================================================

//------------------------------------------------------------------------------
//
void *jcon_thread_run_function(void *session_ptr)
{
  int run_loop;

  if(session_ptr == NULL)
  {
    CRITICAL(NULL, "session_ptr is NULL.");
    return NULL;
  }

  jcon_thread_t *session = (jcon_thread_t *)session_ptr;

  jcon_thread_pthread_mutex_lock(session);
  run_loop = session->runtime_data.run;
  jcon_thread_pthread_mutex_unlock(session);
  
  while(run_loop)
  {
    /* Check connection state */
    jcon_thread_pthread_mutex_lock(session);
    if(jcon_client_isConnected(session->runtime_data.client) == false)
    {
      session->runtime_data.run = false;
      if(session->runtime_data.close_handler)
      {
        session->runtime_data.close_handler(
          session->runtime_data.session_context,
          JCON_THREAD_CLOSETYPE_DISCONNECT,
          jcon_thread_getReferenceString(session)
        );
      }
    }
    jcon_thread_pthread_mutex_unlock(session);

    /* Check for new data */
    jcon_thread_pthread_mutex_lock(session);
    if(jcon_client_newData(session->runtime_data.client))
    {
      if(session->runtime_data.data_handler)
      {
        session->runtime_data.data_handler(
          session->runtime_data.session_context,
          session->runtime_data.client
        );
      }
    }
    jcon_thread_pthread_mutex_unlock(session);

    jcon_thread_loop_sleep(session);

    /* Check for termination */
    jcon_thread_pthread_mutex_lock(session);
    run_loop = session->runtime_data.run;
    jcon_thread_pthread_mutex_unlock(session);
  }

  DEBUG(session, "jcon_thread stopped.");
  return NULL;
}

//------------------------------------------------------------------------------
//
void jcon_thread_loop_sleep(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  struct timespec slp;
  slp.tv_sec = 0;
  slp.tv_nsec = session->runtime_data.loop_sleep;

  int ret_sleep = nanosleep(&slp, NULL);
  if(ret_sleep)
  {
    ERROR(session, "nanosleep() failed [%d : %s].", errno, strerror(errno));
  }
}

//------------------------------------------------------------------------------
//
void jcon_thread_log(jcon_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session && session->runtime_data.client)
  {
    if(session->runtime_data.logger)
    {
      jlog_log_message_m(session->runtime_data.logger, log_type, file, function, line, "<%s> %s", jcon_thread_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_thread_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}

//------------------------------------------------------------------------------
//
int jcon_thread_start(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(session->runtime_data.run)
  {
    WARN(session, "Thread is already running.");
    return true;
  }

  if(jcon_thread_pthread_mutex_init(session) == false)
  {
    ERROR(session, "pthread_mutex could not be initialized.");
    return false;
  }

  jcon_thread_pthread_mutex_lock(session);
  if(jcon_thread_pthread_init(session) == false)
  {
    ERROR(session, "pthread could not be created.");
    jcon_thread_pthread_mutex_unlock(session);
    jcon_thread_pthread_mutex_destroy(session);
    return false;
  }

  session->runtime_data.run = true;
  if(session->runtime_data.create_handler)
  {
    session->runtime_data.create_handler(
      session->runtime_data.session_context,
      JCON_THREAD_CREATETYPE_INIT,
      jcon_thread_getReferenceString(session)
    );
  }
  jcon_thread_pthread_mutex_unlock(session);

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_thread_stop(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(session->runtime_data.run == false)
  {
    WARN(session, "Thread not running.");
    return;
  }

  jcon_thread_pthread_mutex_lock(session);
  session->runtime_data.run = false;
  jcon_thread_pthread_mutex_unlock(session);

  jcon_thread_pthread_join(session);
  jcon_thread_pthread_mutex_destroy(session);
}

//==============================================================================
// Implement helper functions.
//==============================================================================

//------------------------------------------------------------------------------
//
int jcon_thread_pthread_init(jcon_thread_t *session)
{
  int error = pthread_create(&session->thread, NULL, &jcon_thread_run_function, session);
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
void jcon_thread_pthread_join(jcon_thread_t *session)
{
  int error = pthread_join(session->thread, NULL);
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
int jcon_thread_pthread_mutex_init(jcon_thread_t *session)
{
  int error = pthread_mutex_init(&session->runtime_data.mutex, NULL);
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
void jcon_thread_pthread_mutex_destroy(jcon_thread_t *session)
{
  int error = pthread_mutex_destroy(&session->runtime_data.mutex);
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
void jcon_thread_pthread_mutex_lock(jcon_thread_t *session)
{
  int error = pthread_mutex_lock(&session->runtime_data.mutex);
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
void jcon_thread_pthread_mutex_unlock(jcon_thread_t *session)
{
  int error = pthread_mutex_unlock(&session->runtime_data.mutex);
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
/**
 * @file jutil_thread.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jutil_thread.
 * 
 * @date 2020-09-25
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jutil_thread.h>
#include <jayc/jutil_time.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

//==============================================================================
// Define constants.
//

/**
 * @brief pthread can be created.
 */
#define JUTIL_THREAD_STATE_STOPPED 0

/**
 * @brief pthread is initializing.
 */
#define JUTIL_THREAD_STATE_INIT 1

/**
 * @brief pthread is running, can be stopped and joined.
 */
#define JUTIL_THREAD_STATE_RUNNING 2

/**
 * @brief pthread is finished, waiting to be joined.
 */
#define JUTIL_THREAD_STATE_FINISHED 3



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Internal pthread handler.
 * 
 * Manages loop, sleep function and shutting down safely.
 * 
 * @param ctx Context provided by user.
 * 
 * @return    Currently not used.
 */
static void *jutil_thread_pthread_handler(void *ctx);

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
static void jutil_thread_log(jutil_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Declare helper functions.
//

/**
 * @brief Creates pthread instance.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread to create.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jutil_thread_pthread_create(jutil_thread_t *session);

/**
 * @brief Joines pthread.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread to join.
 */
static void jutil_thread_pthread_join(jutil_thread_t *session);

/**
 * @brief Initializes pthread_mutex instance.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread_mutex to create.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jutil_thread_pmutex_init(jutil_thread_t *session);

/**
 * @brief Destroys pthread_mutex.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread_mutex to destroy.
 */
static void jutil_thread_pmutex_destroy(jutil_thread_t *session);

/**
 * @brief Locks pthread_mutex.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread_mutex.
 */
static void jutil_thread_pmutex_lock(jutil_thread_t *session);

/**
 * @brief Unlocks pthread_mutex.
 * 
 * Manages error handling.
 * 
 * @param session Session with pthread_mutex.
 */
static void jutil_thread_pmutex_unlock(jutil_thread_t *session);



//==============================================================================
// Define log macros.
//

#ifdef JUTIL_NO_DEBUG /* Allow to disable debug messages at compile time. */
  #define DEBUG(ctx, fmt, ...)
#else
  #define DEBUG(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(ctx, fmt, ...)jutil_thread_log(ctx, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

//==============================================================================
// Define structures.
//

/**
 * @brief Holds data for thread runtime and operation.
 * 
 */
struct __jutil_thread_session
{
  pthread_t thread;                           /**< pthread instance. */
  pthread_mutex_t mutex;                      /**< pthread_mutex instance. */
  jutil_thread_loop_function_t loop_function; /**< User provided function to run in loop. */
  long loop_sleep;                            /**< How long the loop will wait (in nanoseconds), before continuing. */
  jlog_t *logger;                             /**< Logger used for debug and error messages. */

  int thread_state;                           /**< State of thread. Enumeration with following values:
                                                   * @c #JUTIL_THREAD_STATE_STOPPED
                                                   * @c #JUTIL_THREAD_STATE_INIT
                                                   * @c #JUTIL_THREAD_STATE_RUNNING
                                                   * @c #JUTIL_THREAD_STATE_FINISHED */
  int run_signal;                             /**< If set to 0, the thread will stop. */
  
  void *ctx;                                  /**< Context provided to @c #loop_function() . */
};



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jutil_thread_t *jutil_thread_init(jutil_thread_loop_function_t function, jlog_t *logger, long sleep_ns, void *ctx)
{
  if(function == NULL)
  {
    ERROR(NULL, "Function is NULL.");
    return NULL;
  }

  jutil_thread_t *session = (jutil_thread_t *)malloc(sizeof(jutil_thread_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->thread = 0;
  session->loop_function = function;
  session->loop_sleep = sleep_ns;
  session->logger = logger;
  session->thread_state = JUTIL_THREAD_STATE_STOPPED;
  session->ctx = ctx;

  if(jutil_thread_pmutex_init(session) == false)
  {
    ERROR(session, "Mutex could not be initialized. Destroying session.");
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jutil_thread_free(jutil_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jutil_thread_isRunning(session))
  {
    jutil_thread_stop(session);
  }

  jutil_thread_pmutex_destroy(session);
  free(session);
}

//------------------------------------------------------------------------------
//
void jutil_thread_manage(jutil_thread_t *session)
{
  int state;

  jutil_thread_pmutex_lock(session);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(session);

  if(state == JUTIL_THREAD_STATE_FINISHED)
  {
    jutil_thread_pthread_join(session);
    session->thread_state = JUTIL_THREAD_STATE_STOPPED;
  }
}

//------------------------------------------------------------------------------
//
int jutil_thread_start(jutil_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jutil_thread_isRunning(session))
  {
    WARN(session, "Thread already running.");
    return true;
  }

  session->run_signal = true;
  if(jutil_thread_pthread_create(session) == false)
  {
    ERROR(session, "Thread could not be started.");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void jutil_thread_stop(jutil_thread_t * session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  int state;
  jutil_thread_pmutex_lock(session);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(session);

  if(state == JUTIL_THREAD_STATE_STOPPED)
  {
    WARN(session, "Thread is already stopped.");
    return;
  }

  if(state == JUTIL_THREAD_STATE_INIT || state == JUTIL_THREAD_STATE_RUNNING)
  {
    jutil_thread_pmutex_lock(session);
    session->run_signal = false;
    jutil_thread_pmutex_unlock(session);
  }

  jutil_thread_pthread_join(session);
  session->thread_state = JUTIL_THREAD_STATE_STOPPED;
}

//------------------------------------------------------------------------------
//
void jutil_thread_lockMutex(jutil_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_thread_pmutex_lock(session);
}

//------------------------------------------------------------------------------
//
void jutil_thread_unlockMutex(jutil_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_thread_pmutex_unlock(session);
}

//------------------------------------------------------------------------------
//
int jutil_thread_isRunning(jutil_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  int state;
  jutil_thread_pmutex_lock(session);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(session);

  if(state == JUTIL_THREAD_STATE_STOPPED)
  {
    return false;
  }
  else
  {
    return true;
  }
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
void *jutil_thread_pthread_handler(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jutil_thread_t *session = (jutil_thread_t *)ctx;

  if(session->loop_function == NULL)
  {
    ERROR(session, "Loop function is NULL.");
    return NULL;
  }

  int run;
  jutil_thread_pmutex_lock(session);
  run = session->run_signal;
  session->thread_state = JUTIL_THREAD_STATE_RUNNING;
  jutil_thread_pmutex_unlock(session);

  DEBUG(session, "Thread start ...");

  while(run)
  {
    /* Run loop function. */
    int ret_loop = session->loop_function(session->ctx, session);

    /* Sleep. */
    jutil_time_sleep(0, session->loop_sleep);

    /* Check, if thread should exit. */
    jutil_thread_pmutex_lock(session);
    run = (session->run_signal && ret_loop);
    jutil_thread_pmutex_unlock(session);
  }

  jutil_thread_pmutex_lock(session);
  session->thread_state = JUTIL_THREAD_STATE_FINISHED;
  jutil_thread_pmutex_unlock(session);

  DEBUG(session, "Thread exit.");

  return NULL;
}

//------------------------------------------------------------------------------
//
void jutil_thread_log(jutil_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session)
  {
    unsigned long id;
    if(session->thread_state == JUTIL_THREAD_STATE_STOPPED)
    {
      id = 0;
    }
    else
    {
      id = session->thread;
    }

    if(session->logger)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<pthread:%lu> %s", id, buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<pthread:%lu> %s", id, buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}



//==============================================================================
// Implement helper functions.
//

//------------------------------------------------------------------------------
//
int jutil_thread_pthread_create(jutil_thread_t *session)
{
  int error = pthread_create(&session->thread, NULL, &jutil_thread_pthread_handler, session);
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
void jutil_thread_pthread_join(jutil_thread_t *session)
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
int jutil_thread_pmutex_init(jutil_thread_t *session)
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
        DEBUG(session, "pthread_mutex_init() failed [%d : %s]. Mutex already initialized.", error, strerror(error));
        return true;
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
void jutil_thread_pmutex_destroy(jutil_thread_t *session)
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
void jutil_thread_pmutex_lock(jutil_thread_t *session)
{
  int error = pthread_mutex_lock(&session->mutex);
  if(error)
  {
    switch(error)
    {
      case EINVAL:
      {
        ERROR(NULL, "pthread_mutex_lock() failed [%d : %s]. Invalid priority.", error, strerror(error));
        break;
      }

      case EAGAIN:
      {
        ERROR(NULL, "pthread_mutex_lock() failed [%d : %s]. Max number of recursive locks.", error, strerror(error));
        break;
      }

      case EDEADLK:
      {
        ERROR(NULL, "pthread_mutex_lock() failed [%d : %s]. Current thread already owns mutex.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(NULL, "pthread_mutex_lock() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}

//------------------------------------------------------------------------------
//
void jutil_thread_pmutex_unlock(jutil_thread_t *session)
{
  int error = pthread_mutex_unlock(&session->mutex);
  if(error)
  {
    switch(error)
    {
      case EINVAL:
      {
        ERROR(NULL, "pthread_mutex_unlock() failed [%d : %s]. Invalid mutex.", error, strerror(error));
        break;
      }

      case EAGAIN:
      {
        ERROR(NULL, "pthread_mutex_unlock() failed [%d : %s]. Max number of recursive locks.", error, strerror(error));
        break;
      }

      case EPERM:
      {
        ERROR(NULL, "pthread_mutex_unlock() failed [%d : %s]. Current thread does not own the mutex.", error, strerror(error));
        break;
      }

      default:
      {
        ERROR(NULL, "pthread_mutex_unlock() failed [%d : %s].", error, strerror(error));
        break;
      }
    }
  }
}
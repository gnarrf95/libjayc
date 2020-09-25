/**
 * @file jutil_thread.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-25
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for nanosleep() */

#include <jutil_thread.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(ctx, fmt, ...)jutil_thread_log(ctx, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(ctx, fmt, ...) jutil_thread_log(ctx, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

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

  struct timespec sleep_time;
  sleep_time.tv_sec = 0;
  sleep_time.tv_nsec = session->loop_sleep;

  int run;
  int ret_loop;
  int ret_sleep;
  jutil_thread_pmutex_lock(&session->mutex);
  run = session->run_signal;
  session->thread_state = JUTIL_THREAD_STATE_RUNNING;
  jutil_thread_pmutex_unlock(&session->mutex);

  while(run)
  {
    /* Run loop function. */
    ret_loop = session->loop_function(session->ctx, (void *)&session->mutex);

    /* Sleep. */
    ret_sleep = nanosleep(&sleep_time, NULL);
    if(ret_sleep)
    {
      ERROR(session, "nanosleep() failed [%d : %s].", errno, strerror(errno));
    }

    /* Check, if thread should exit. */
    jutil_thread_pmutex_lock(&session->mutex);
    run = (session->run_signal && !ret_loop);
    jutil_thread_pmutex_unlock(&session->mutex);
  }

  jutil_thread_pmutex_lock(&session->mutex);
  session->thread_state = JUTIL_THREAD_STATE_FINISHED;
  jutil_thread_pmutex_unlock(&session->mutex);

  DEBUG(session, "Thread exit.");

  return NULL;
}

//------------------------------------------------------------------------------
//
void jutil_thread_manage(jutil_thread_t *session)
{
  int state;

  jutil_thread_pmutex_lock(&session->mutex);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(&session->mutex);

  if(state == JUTIL_THREAD_STATE_FINISHED)
  {
    jutil_thread_pthread_join(session);
    session->thread_state = JUTIL_THREAD_STATE_STOPPED;
  }
}

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
int jutil_thread_start(jutil_thread_t *session)
{
  if(session)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(jutil_thread_isRunning(session))
  {
    WARN(session, "Thread already running.");
    return true;
  }

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
  jutil_thread_pmutex_lock(&session->mutex);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(&session->mutex);

  if(state == JUTIL_THREAD_STATE_STOPPED)
  {
    WARN(session, "Thread is already stopped.");
    return;
  }

  if(state == JUTIL_THREAD_STATE_INIT || state == JUTIL_THREAD_STATE_RUNNING)
  {
    jutil_thread_pmutex_lock(&session->mutex);
    session->run_signal = false;
    jutil_thread_pmutex_unlock(&session->mutex);
  }

  jutil_thread_pthread_join(session);
  session->thread_state = JUTIL_THREAD_STATE_STOPPED;
}

//------------------------------------------------------------------------------
//
void *jutil_thread_getMutexPtr(jutil_thread_t *session)
{
  return (void *)&session->mutex;
}

//------------------------------------------------------------------------------
//
void jutil_thread_lockMutex(void *mutex_ptr)
{
  if(mutex_ptr == NULL)
  {
    ERROR(NULL, "Mutex pointer is NULL.");
    return;
  }

  jutil_thread_pmutex_lock((pthread_mutex_t *)mutex_ptr);
}

//------------------------------------------------------------------------------
//
void jutil_thread_unlockMutex(void *mutex_ptr)
{
  if(mutex_ptr == NULL)
  {
    ERROR(NULL, "Mutex pointer is NULL.");
    return;
  }

  jutil_thread_pmutex_unlock((pthread_mutex_t *)mutex_ptr);
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
  jutil_thread_pmutex_lock(&session->mutex);
  state = session->thread_state;
  jutil_thread_pmutex_unlock(&session->mutex);

  if(state == JUTIL_THREAD_STATE_STOPPED)
  {
    return false;
  }
  else
  {
    return true;
  }
}


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
void jutil_thread_pmutex_lock(pthread_mutex_t *mutex)
{
  int error = pthread_mutex_lock(mutex);
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
void jutil_thread_pmutex_unlock(pthread_mutex_t *mutex)
{
  int error = pthread_mutex_unlock(mutex);
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
    if(session->thread_state == JUTIL_THREAD_STATE_INIT || session->thread_state == JUTIL_THREAD_STATE_RUNNING)
    {
      id = session->thread;
    }
    else
    {
      id = 0;
    }

    if(session->logger)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<pthread:%d> %s", id, buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<pthread:%d> %s", id, buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
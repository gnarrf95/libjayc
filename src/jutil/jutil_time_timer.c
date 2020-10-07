/**
 * @file jutil_time_timer.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementations for jutil_time timer.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for timer functions */

#include <jayc/jutil_time.h>
#include <jayc/jlog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

//==============================================================================
// Define structures and constants.
//

/**
 * @brief Timer session object.
 */
struct __jutil_time_timer_session
{
  struct timespec interval;           /**< Interval struct. */
  struct sigevent timer_event;        /**< Timer event. */
  timer_t timer_id;                   /**< Timer object reference. */

  jutil_time_timer_handler_t handler; /**< Handler function to get executed. */
  void *session_ctx;                  /**< Session context pointer passed to handler. */
};

/**
 * @brief Empty time definition for stopping timer.
 */
static const struct timespec JUTIL_TIME_TIMER_INT_EMPTY = { 0, 0 };



//==============================================================================
// Define log macros.
//

#ifdef JUTIL_NO_DEBUG /* Allow to disable debug messages at compile time. */
  #define DEBUG(fmt, ...)
#else
  #define DEBUG(fmt, ...) JLOG_DEBUG(fmt, ##__VA_ARGS__)
#endif
#define INFO(fmt, ...) JLOG_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) JLOG_WARN(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) JLOG_ERROR(fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...)JLOG_CRITICAL(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) JLOG_FATAL(fmt, ##__VA_ARGS__)



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Internal handler for timer.
 * 
 * Handles user provided handler and passing data.
 * 
 * @param ctx Timer value union.
 */
static void sigevent_function(union sigval ctx);



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jutil_time_timer_t *jutil_time_timer_init(void *ctx, jutil_time_timer_handler_t handler, long interval_secs, long interval_nanosecs)
{
  if(handler == NULL)
  {
    return NULL;
  }

  if(interval_secs == 0 && interval_nanosecs == 0)
  {
    return NULL;
  }

  jutil_time_timer_t *session = (jutil_time_timer_t *)malloc(sizeof(jutil_time_timer_t));
  if(session == NULL)
  {
    return NULL;
  }

  session->interval.tv_sec = interval_secs;
  session->interval.tv_nsec = interval_nanosecs;

  session->session_ctx = ctx;
  session->handler = handler;

  session->timer_event.sigev_notify = SIGEV_THREAD;
  session->timer_event.sigev_signo = 0;
  session->timer_event.sigev_value.sival_int = 0;
  session->timer_event.sigev_value.sival_ptr = (void *)session;
  session->timer_event.sigev_notify_function = &sigevent_function;
  session->timer_event.sigev_notify_attributes = NULL;

  if(timer_create(CLOCK_REALTIME, &session->timer_event, &session->timer_id) < 0)
  {
    ERROR("timer_create() failed [%d : %s].", errno, strerror(errno));
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jutil_time_timer_free(jutil_time_timer_t *session)
{
  if(session == NULL)
  {
    return;
  }

  if(timer_delete(session->timer_id) < 0)
  {
    ERROR("timer_delete() failed [%d : %s].", errno, strerror(errno));
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jutil_time_timer_start(jutil_time_timer_t *session)
{
  // https://www.man7.org/linux/man-pages/man2/timer_settime.2.html

  if(session == NULL)
  {
    return false;
  }

  struct itimerspec timer_interval;
  timer_interval.it_interval = session->interval;
  timer_interval.it_value = session->interval;

  if(timer_settime(session->timer_id, 0, &timer_interval, NULL) < 0)
  {
    ERROR("timer_settime() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_time_timer_stop(jutil_time_timer_t *session)
{
  // https://www.man7.org/linux/man-pages/man2/timer_settime.2.html

  if(session == NULL)
  {
    return false;
  }

  struct itimerspec timer_interval;
  timer_interval.it_interval = session->interval;
  timer_interval.it_value = JUTIL_TIME_TIMER_INT_EMPTY;

  if(timer_settime(session->timer_id, 0, &timer_interval, NULL) < 0)
  {
    ERROR("timer_settime() failed [%d : %s].", errno, strerror(errno));
    return false;
  }

  return true;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
void sigevent_function(union sigval ctx)
{
  if(ctx.sival_ptr == NULL)
  {
    return;
  }

  jutil_time_timer_t *session = (jutil_time_timer_t *)ctx.sival_ptr;

  if(session->handler(session->session_ctx) == false)
  {
    jutil_time_timer_stop(session);
  }
}
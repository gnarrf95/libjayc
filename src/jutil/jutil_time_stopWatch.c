/**
 * @file jutil_time_stopWatch.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementations for jutil_time stopWatch.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for clock_gettime() */

#include <jayc/jutil_time.h>
#include <jayc/jlog.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>

//==============================================================================
// Define structure and log macros.
//

/**
 * @brief StopWatch session object.
 */
struct __jutil_time_stopWatch_session
{
  struct timespec time_buffer;  /**< Holds time data of last reset. */
};

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
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jutil_time_stopWatch_t *jutil_time_stopWatch_init()
{
  jutil_time_stopWatch_t *session = (jutil_time_stopWatch_t *)malloc(sizeof(jutil_time_stopWatch_t));
  if(session == NULL)
  {
    return NULL;
  }

  if(clock_gettime(CLOCK_MONOTONIC, &session->time_buffer) < 0)
  {
    ERROR("clock_gettime() failed [%d : %s].", errno, strerror(errno));
    free(session);
    return NULL;
  }
  
  return session;
}

//------------------------------------------------------------------------------
//
unsigned long jutil_time_stopWatch_reset(jutil_time_stopWatch_t *session)
{
  if(session == NULL)
  {
    return 0;
  }

  struct timespec tmp;

  if(clock_gettime(CLOCK_MONOTONIC, &tmp) < 0)
  {
    ERROR("clock_gettime() failed [%d : %s].", errno, strerror(errno));
    return 0;
  }

  double diff_ns = tmp.tv_nsec - session->time_buffer.tv_nsec;
  double diff_sec = tmp.tv_sec - session->time_buffer.tv_sec;

  /* Time difference in milliseconds. */
  unsigned long ret = (unsigned long)(diff_ns / 1000000) + (unsigned long)(diff_sec * 1000);

  session->time_buffer = tmp;
  return ret;
}

//------------------------------------------------------------------------------
//
unsigned long jutil_time_stopWatch_check(jutil_time_stopWatch_t *session)
{
  if(session == NULL)
  {
    return 0;
  }

  struct timespec tmp;

  if(clock_gettime(CLOCK_MONOTONIC, &tmp) < 0)
  {
    ERROR("clock_gettime() failed [%d : %s].", errno, strerror(errno));
    return 0;
  }

  double diff_ns = tmp.tv_nsec - session->time_buffer.tv_nsec;
  double diff_sec = tmp.tv_sec - session->time_buffer.tv_sec;

  /* Time difference in milliseconds. */
  unsigned long ret = (unsigned long)(diff_ns / 1000000) + (unsigned long)(diff_sec * 1000);

  return ret;
}

//------------------------------------------------------------------------------
//
void jutil_time_stopWatch_free(jutil_time_stopWatch_t *session)
{
  if(session)
  {
    free(session);
  }
}
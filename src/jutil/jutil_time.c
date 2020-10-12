/**
 * @file jutil_time.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementations for jutil_time.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @see https://www.tutorialspoint.com/c_standard_library/time_h.htm
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for nanosleep() */

#include <jayc/jutil_time.h>
#include <jayc/jlog.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>

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
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
int jutil_time_formatTime(char *str_buf, size_t str_size, const char *format, time_t timestamp)
{
  if(str_buf == NULL)
  {
    return false;
  }

  struct tm *time_info = localtime(&timestamp);

  if(strftime(str_buf, str_size, format, time_info) == 0)
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_time_getTimeString(char *str_buf, size_t str_size, time_t timestamp)
{
  if(str_buf == NULL)
  {
    return false;
  }


  return jutil_time_formatTime(str_buf, str_size, "%Y-%m-%d %H:%M:%S", timestamp);
}

//------------------------------------------------------------------------------
//
int jutil_time_getCurrentTimeString(char *str_buf, size_t str_size)
{
  if(str_buf == NULL)
  {
    return false;
  }

  time_t timestamp = time(NULL);
  if(timestamp == -1)
  {
    return false;
  }

  return jutil_time_formatTime(str_buf, str_size, "%Y-%m-%d %H:%M:%S", timestamp);
}

//------------------------------------------------------------------------------
//
void jutil_time_sleep(long secs, long nanosecs, int exit_on_int)
{
  if(secs < 0)
  {
    secs = 0;
  }

  if(nanosecs > 999999999L)
  {
    long ns_tmp = nanosecs;
    nanosecs = ns_tmp % 1000000000L;

    ns_tmp -= nanosecs;
    secs += ns_tmp / 1000000000L;
  }
  else if(nanosecs < 0)
  {
    nanosecs = 0;
  }

  struct timespec sleep_time_rem;
  struct timespec sleep_time;
  sleep_time.tv_sec = secs;
  sleep_time.tv_nsec = nanosecs;

  int run = true;
  while(run)
  {
    if(nanosleep(&sleep_time, &sleep_time_rem))
    {
      if(errno == EINTR)
      {
        DEBUG("nanosleep() interrupted.");

        if(exit_on_int)
        {
          run = false;
        }
        sleep_time = sleep_time_rem;
      }
      else
      {
        ERROR("nanosleep() failed [%d : %s].", errno, strerror(errno));
      }
    }
    else
    {
      run = false;
    }
  }
}
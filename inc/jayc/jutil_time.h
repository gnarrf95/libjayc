/**
 * @file jutil_time.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Provides functionality for time management.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_TIME_H
#define INCLUDE_JUTIL_TIME_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif



//==============================================================================
// Definitions for jutil_time_stopWatch.
//

/**
 * @brief StopWatch session object.
 */
typedef struct __jutil_time_stopWatch_session jutil_time_stopWatch_t;

/**
 * @brief Initializes stop watch.
 * 
 * @return  Session object for new stop watch.
 * @return  @c NULL , if error occured. 
 */
jutil_time_stopWatch_t *jutil_time_stopWatch_init();

/**
 * @brief Get milliseconds since last reset and reset stop watch.
 * 
 * @param session Session object.
 * 
 * @return        Time since last reset in milliseconds.
 * @return        @c 0 , if error occured.
 */
unsigned long jutil_time_stopWatch_reset(jutil_time_stopWatch_t *session);

/**
 * @brief Get milliseconds since last reset, without resetting.
 * 
 * @param session Session object.
 * 
 * @return        Time since last reset in milliseconds.
 * @return        @c 0 , if error occured.
 */
unsigned long jutil_time_stopWatch_check(jutil_time_stopWatch_t *session);

/**
 * @brief Free memory of stop watch session object.
 * 
 * @param session Session to free.
 */
void jutil_time_stopWatch_free(jutil_time_stopWatch_t *session);



//==============================================================================
// Time string functions.
//

/**
 * @brief Formats a timestamp into string.
 * 
 * @param str_buf   String buffer.
 * @param str_size  Size of string buffer.
 * @param format    Format string (f.ex. "%Y-%m-%d %H:%M:%S").
 * @param timestamp Timestamp to format.
 * 
 * @return          @c true , if successful.
 * @return          @c false , if error occured.
 */
int jutil_time_formatTime(char *str_buf, size_t str_size, const char *format, long timestamp);

/**
 * @brief Gets standardized string for timestamp
 * 
 * Format example: "2020-10-07 13:25:44"
 * 
 * @param str_buf   String buffer.
 * @param str_size  Size of string buffer.
 * @param timestamp Timestamp to format.
 * 
 * @return          @c true , if successful.
 * @return          @c false , if error occured.
 */
int jutil_time_getTimeString(char *str_buf, size_t str_size, long timestamp);

/**
 * @brief Creates standardized string for current time.
 * 
 * Like @c #jutil_time_getTimeString() but with current time.
 * 
 * @param str_buf   String buffer.
 * @param str_size  Size of string buffer.
 * 
 * @return          @c true , if successful.
 * @return          @c false , if error occured.
 */
int jutil_time_getCurrentTimeString(char *str_buf, size_t str_size);



//==============================================================================
// Sleep functions.
//

/**
 * @brief Suspends current thread for specified amount of time.
 * 
 * @param secs        Seconds to sleep.
 * @param nanosecs    Additional nanoseconds to sleep.
 * @param exit_on_int If @c true , will exit when interrupt signal
 *                    arrives.
 *                    If @c false , will keep going until
 *                    time has elapsed.
 */
void jutil_time_sleep(long secs, long nanosecs, int exit_on_int);



//==============================================================================
// Definitions for jutil_time_timer.
//

/**
 * @brief Timer session object.
 */
typedef struct __jutil_time_timer_session jutil_time_timer_t;

/**
 * @brief Handler called by timer.
 * 
 * @param ctx Session context provided by user.
 * 
 * @return    @c true , if timer should continue.
 * @return    @c false , if timer should stop after execution.
 */
typedef int (*jutil_time_timer_handler_t)(void *ctx);

/**
 * @brief Initializes timer object.
 * 
 * @param ctx                 Context pointer to be passed to handler.
 * @param handler             Handler function.
 * @param interval_secs       Seconds after which handler should be called.
 * @param interval_nanosecs   Additional nanoseconds to wait before calling handler.
 * 
 * @return                    New timer session object.
 * @return                    @c NULL , if error occured.
 */
jutil_time_timer_t *jutil_time_timer_init(void *ctx, jutil_time_timer_handler_t handler, long interval_secs, long interval_nanosecs);

/**
 * @brief Stops timer and frees memory.
 * 
 * @bug valgrind still finds lost memory, but this seems to be a false positive.
 *      https://lng-odp.linaro.narkive.com/dLxhUyXZ/bug-1101-new-odp-timer-valgrind-memory
 * 
 * @param session Session object to free.
 */
void jutil_time_timer_free(jutil_time_timer_t *session);

/**
 * @brief Starts timer.
 * 
 * @param session Session to start.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
int jutil_time_timer_start(jutil_time_timer_t *session);

/**
 * @brief Stops timer.
 * 
 * @param session Session to stop.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
int jutil_time_timer_stop(jutil_time_timer_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_TIME_H */
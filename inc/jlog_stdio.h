/**
 * @file jlog_stdio.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jlog, that uses fprintf for output.
 * 
 * This jlog implementation logs to console using @c fprintf() .
 * Debug, info and warning logs get printed to @c stdout and
 * error, critical and fatal logs get printed to @c stderr .
 * 
 * For normal output without color, use @c #jlog_stdio_session_init() .
 * 
 * If you want your output to be color coded, first initialize a
 * color context using @c #jlog_stdio_color_context_init() .
 * There you chose, what colors are used for what.
 * The error color will be used for error, critical and fatal logs.
 * 
 * After that you initialize a session, by calling
 * @c #jlog_stdio_color_session_init() with the color context as parameter.
 * 
 * The session gets used with the normal jlog functions, and freed using
 * the normal @c #jlog_session_free() function.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @see jlog.h
 * 
 */

#ifndef INCLUDE_JLOG_STDIO_H
#define INCLUDE_JLOG_STDIO_H

#include <jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates pointer for color context.
 *
 * @param debug_color ANSI color code used for debug messages.
 * @param info_color  ANSI color code used for info messages.
 * @param warn_color  ANSI color code used for warning messages.
 * @param error_color ANSI color code used for error, critical and fatal messages.
 *
 * @return            Pointer to @c jlog_stdio_color_context_t object.
 * @return            @c NULL , if failed.
 */
void *jlog_stdio_color_context_init(const char *debug_color,
                                    const char *info_color,
                                    const char *warn_color,
                                    const char *error_color);

/**
 * @brief Creates @c #jlog_t session, that logs using @c fprintf .
 *
 * @param log_level Log level to use.
 *
 * @return          Session pointer.
 * @return          @c NULL , if failed.
 */
jlog_t *jlog_stdio_session_init(int log_level);

/**
 * @brief Creates @c #jlog_t session that logs colored outputs using @c fprintf .
 *
 * @param log_level Log level to use.
 * @param ctx       Color context.
 *
 * @return          Session pointer.
 * @return          @c NULL , if failed.
 * 
 */
jlog_t *jlog_stdio_color_session_init(int log_level, void *ctx);

#ifdef __cpluslpus
}
#endif

#endif /* INCLUDE_JLOG_STDIO_H */
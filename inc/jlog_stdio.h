/**
 * @file jlog_stdio.h
 * @author Manuel Nadji
 * 
 * @brief This is an implementation of jlog, that uses fprintf for output.
 * 
 * @todo add details...
 * 
 * @date 2020-09-16
 * 
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * @see jlog.h
 * 
 */

#ifndef INCLUDE_JLOG_STDIO_H
#define INCLUDE_JLOG_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jlog.h>

/**
 * @brief Creates pointer for color context.
 * 
 * @todo add details...
 *
 * @param debug_color ANSI color code used for debug messages.
 * @param info_color  ANSI color code used for info messages.
 * @param warn_color  ANSI color code used for warning messages.
 * @param error_color ANSI color code used for error messages.
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
 * @todo add details...
 *
 * @param log_level Log level to use.
 *
 * @return          Session pointer.
 * @return          @c NULL , if failed.
 */
jlog_t *jlog_stdio_session_init(int log_level);

/**
 * @brief Creates @c #jlog_t session that logs colored outputs using @c fprintf .
 * @see @c #jlog_stdio_session_init()
 * 
 * @todo add details...
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
} /* extern "C" */
#endif

#endif /* INCLUDE_JLOG_STDIO_H */
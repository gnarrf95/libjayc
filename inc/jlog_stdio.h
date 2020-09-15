#ifndef INCLUDE_JLOG_STDIO_H
#define INCLUDE_JLOG_STDIO_H

#include <jlog.h>

/*******************************************************************************
 * @brief Creates jlog_stdio_color_context object.
 *
 * @param debug_color : ANSI color code used for debug messages.
 * @param info_color : ANSI color code used for info messages.
 * @param warn_color : ANSI color code used for warning messages.
 * @param error_color : ANSI color code used for error messages.
 *
 * @return : Pointer to jlog_stdio_color_context object.
 */
void *jlog_stdio_color_context_init(const char *debug_color, const char *info_color, const char *warn_color, const char *error_color);

/*******************************************************************************
 * @brief Creates jlog_stdio session object.
 *
 * @param log_level : Log level to use.
 *
 * @return : Session pointer.
 */
jlog_t *jlog_stdio_session_init(uint8_t log_level);

/*******************************************************************************
 * @brief Creates jlog_stdio_color session object.
 *
 * @param log_level : Log level to use.
 * @param ctx : Color context.
 *
 * @return : Session pointer.
 */
jlog_t *jlog_stdio_color_session_init(uint8_t log_level, void *ctx);

#endif
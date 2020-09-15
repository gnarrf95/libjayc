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
 * @brief Destroys jlog_stdio_color_context object.
 * 
 * @param ctx : Object to destroy.
 */
void jlog_stdio_color_context_free(void *ctx);

/*******************************************************************************
 * @brief Creates jlog_stdio session object.
 *
 * @param log_level : Log level to use.
 *
 * @return : Session pointer.
 */
jlog_t *jlog_stdio_init(uint8_t log_level);

/*******************************************************************************
 * @brief Creates jlog_stdio_color session object.
 *
 * @param log_level : Log level to use.
 * @param ctx : Color context.
 *
 * @return : Session pointer.
 */
jlog_t *jlog_stdio_color_init(uint8_t log_level, void *ctx);

/*******************************************************************************
 * @brief Destroys context for jlog_stdio_color session.
 * 
 * @param ctx: Session context to destroy.
 */
void jlog_stdio_color_free_handler(void *ctx);

/*******************************************************************************
 * @brief Handler to log message.
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_stdio_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...);

/*******************************************************************************
 * @brief Handler to log message. Contains additional information (filename, function name, line number).
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_stdio_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

#endif
#ifndef INCLUDE_JLOG_STDIO_COLOR_H
#define INCLUDE_JLOG_STDIO_COLOR_H

#include <jlog.h>

/*******************************************************************************
 * @brief Color context for jlog_stdio_color session.
 *        Colors are saved in ANSI color codes.
 *        http://web.theurbanpenguin.com/adding-color-to-your-output-from-c/
 */
typedef struct __jlog_stdio_color_context
{
  char *debug_color;
  char *info_color;
  char *warn_color;
  char *error_color;
} jlog_stdio_color_context_t;

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
jlog_stdio_color_context_t *jlog_stdio_color_context_init(const char *debug_color, const char *info_color, const char *warn_color, const char *error_color);

/*******************************************************************************
 * @brief Destroys jlog_stdio_color_context object.
 * 
 * @param ctx : Object to destroy.
 */
void jlog_stdio_color_context_free(jlog_stdio_color_context_t *ctx);

/*******************************************************************************
 * @brief Creates jlog_stdio_color session object.
 *
 * @param log_level : Log level to use.
 * @param ctx : Color context.
 *
 * @return : Session pointer.
 */
jlog_t *jlog_stdio_color_init(uint8_t log_level, jlog_stdio_color_context_t *ctx);

/*******************************************************************************
 * @brief Destroys context for jlog_stdio_color session.
 * 
 * @param ctx: Session context to destroy.
 */
void jlog_stdio_color_free_handler(void *ctx);

/*******************************************************************************
 * @brief Handler to log message.
 *
 * @param ctx : Session context; saved color for each log type.
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_stdio_color_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...);

/*******************************************************************************
 * @brief Handler to log message. Contains additional information (filename, function name, line number).
 *
 * @param ctx : Session context; saved color for each log type.
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_stdio_color_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

#endif
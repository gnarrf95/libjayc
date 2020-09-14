#ifndef INCLUDE_JLOG_SYSLOG_H
#define INCLUDE_JLOG_SYSLOG_H

#include <jlog.h>

/*******************************************************************************
 * @brief Create session object. jlog_syslog session is handled as singleton.
 *        Only one session can exists in program.
 *
 * @param id : Program name.
 * @param facility : Type of program (see '$ man syslog').
 * 
 * @return : jlog session object.
 */
jlog_t *jlog_syslog_init(uint8_t log_level, const char *id, int facility);

void jlog_syslog_free_handler(void *ctx);

/*******************************************************************************
 * @brief Handler to log message.
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_syslog_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...);

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
void jlog_syslog_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

#endif
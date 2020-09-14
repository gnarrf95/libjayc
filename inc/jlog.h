#ifndef INCLUDE_JLOG_H
#define INCLUDE_JLOG_H

#include <stdint.h>

/*******************************************************************************
 * @brief Log type definitions.
 */
#define JLOG_LOGTYPE_DEBUG 0
#define JLOG_LOGTYPE_INFO 1
#define JLOG_LOGTYPE_WARN 2
#define JLOG_LOGTYPE_ERROR 3

/*******************************************************************************
 * @brief Predefinition for use in handler functions.
 */
struct __jlog_session;

/*******************************************************************************
 * @brief Function to handle log calls.
 *
 * @param ctx : Context pointer for session data used by certain loggers.
 * @param log_type : Type of log message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
typedef void (*jlog_message_handler_t)(void *ctx, uint8_t log_type, const char *fmt, ...);

/*******************************************************************************
 * @brief Function to handle log calls, uses information from preprocessor
 * macros.
 *
 * @param ctx : Context pointer for session data used by certain loggers.
 * @param log_type : Type of log message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param fmt : Format string used for stdarg.h .
 */
typedef void (*jlog_message_handler_m_t)(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

/*******************************************************************************
 * @brief Handler to destroy session. Session carries its own function to free the context memory.
 *
 * @param ctx : Session context to free.
 */
typedef void (*jlog_free_handler_t)(void *ctx);

/*******************************************************************************
 * @brief jlog session object, holds data for log calls.
 *
 * @data log_function : Holds function pointer to log handler.
 * @data log_function_m : Holds function pointer to log data with additional information (filename, function name, line number).
 * @data free_handler : Function to free session context memory. Called by jlog_session_free() .
 * @data log_level : Only log messages with log type >= log level.
 * @data session_context : Context pointer for session data used by certain loggers.
 */
typedef struct __jlog_session
{
  jlog_message_handler_t log_function;
  jlog_message_handler_m_t log_function_m;
  jlog_free_handler_t free_handler;
  uint8_t log_level;
  void *session_context;
} jlog_t;

/*******************************************************************************
 * @brief Frees session memory.
 *
 * @param session : Session object to Destroy.
 */
void jlog_session_free(struct __jlog_session *session);

/*******************************************************************************
 * @brief Logs message with session.
 *
 * @param session : Session to use.
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_log_message(struct __jlog_session *session, uint8_t log_type, const char *fmt, ...);

/*******************************************************************************
 * @brief Logs message with session. Contains additional information (filename, function name, line number).
 *
 * @param session : Session to use.
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_log_message_m(struct __jlog_session *session, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

/*******************************************************************************
 * @brief Set global session variable.
 * 
 * @param session : Session object to save as global.
 */
void jlog_global_session_set(struct __jlog_session *session);

/*******************************************************************************
 * @brief Free global session object.
 */
void jlog_global_session_free();

/*******************************************************************************
 * @brief Log message via global session object.
 *
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_global_log_message(uint8_t log_type, const char *fmt, ...);

/*******************************************************************************
 * @brief Log message via global session object. Contains additional information (filename, function name, line number).
 *
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param fmt : Format string used for stdarg.h .
 */
void jlog_global_log_message_m(uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

#define JLOG_DEBUG(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define JLOG_INFO(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define JLOG_WARN(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define JLOG_ERROR(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

#endif

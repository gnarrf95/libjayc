/**
 * @file jlog_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definitions for implementations of jlog.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JLOG_DEV_H
#define INCLUDE_JLOG_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jlog.h>

/**
 * @brief Function to handle log calls.
 *
 * @param ctx       Context pointer for session data used by certain loggers.
 * @param log_type  Type of log message (debug, info, warning, error).
 * @param msg       Message string to log.
 */
typedef void(*jlog_message_handler_t)(void *ctx, int log_type,
                                      const char *msg);

/**
 * @brief Function to handle log calls, uses source code information.
 *
 * @param ctx       Context pointer for session data used by certain loggers.
 * @param log_type  Type of log message (debug, info, warning, error).
 * @param file      File name in which log was called.
 * @param function  Function name in which log was called.
 * @param line      Line number on which log was called.
 * @param msg       Message string to log.
 */
typedef void(*jlog_message_handler_m_t)(void *ctx, int log_type,
                                        const char *file, const char *function,
                                        int line, const char *msg);

/**
 * @brief Handler to destroy session.
 * 
 * A session, that has a session context, needs a free handler, to free the
 * memory space.
 *
 * @param ctx Session context to free.
 */
typedef void(*jlog_session_free_handler_t)(void *ctx);

/**
 * @brief jlog session object, holds data for log calls.
 */
struct __jlog_session
{
  jlog_message_handler_t log_function;              /**< Holds function pointer to log handler. */
  jlog_message_handler_m_t log_function_m;          /**< Holds function pointer to log data with source code info. */
  jlog_session_free_handler_t session_free_handler; /**< Function to free session context memory. Called by @c jlog_session_free() . */
  int log_level;                                    /**< Only log messages with log type >= log level. */
  void *session_context;                            /**< Context pointer for session data used by certain loggers. */
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JLOG_DEV_H */
/**
 * @file jlog.h
 * @author Manuel Nadji (manuel.nadji@gmail.com)
 * 
 * @brief This is a logger system, that can call customized implementations.
 * 
 * jlog is a customizable logger system. This file contains the core
 * functionality and interface descriptions.
 * A logger session needs handlers for log messages
 * ( @c #jlog_message_handler_t ), log messages with source code info
 * ( @c #jlog_message_handler_m_t ), as well as an optional session context for
 * implementation relevant data and a free handler
 * ( @c #jlog_session_free_handler_t ) to free the memory of the session
 * context.
 * Also every logger session has a maximum log level (to filter out
 * unnecessary debug logs) and a there needs to be a @c _session_init
 * function, to initiate the session.
 * 
 * Loggers sessions can be used as a global session
 * ( @c jlog_global_session_set() ). They can then either be used directly
 * with the global functions ( @c jlog_global_log_message() and
 * @c jlog_global_log_message_m() ), or with the macros ( @c #JLOG_DEBUG ,
 * @c #JLOG_INFO , @c #JLOG_WARN and @c #JLOG_ERROR ), which already use the
 * preprocessor macros for source code information ( @c \_\_FILE\_\_ ,
 * @c \_\_func\_\_ and @c \_\_LINE\_\_ ).
 * The global session can then be freed using @c jlog_global_session_free() .
 * 
 * @date 2020-09-16
 * 
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JLOG_H
#define INCLUDE_JLOG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief jlog session object, holds data for log calls.
 */
typedef struct __jlog_session jlog_t;

//==============================================================================
// Define log types.
//==============================================================================

#define JLOG_LOGTYPE_DEBUG 0  /**< Marks debug messages. */
#define JLOG_LOGTYPE_INFO 1   /**< Marks info messages. */
#define JLOG_LOGTYPE_WARN 2   /**< Marks warning messages. */
#define JLOG_LOGTYPE_ERROR 3  /**< Marks error messages. */

//==============================================================================
// Define functions.
//==============================================================================

/**
 * @brief Creates quiet session, that doesn't log.
 * 
 * @return Pointer to quiet jlog session.
 */
jlog_t *jlog_session_quiet();

/**
 * @brief Frees session memory.
 * 
 * If available, frees session context using @c jlog_t#session_free_handler .
 * Afterwards frees session itself.
 *
 * @param session Session object to Destroy.
 */
void jlog_session_free(jlog_t *session);

/**
 * @brief Logs message with session.
 * 
 * If available, calls @c jlog_t#log_function .
 *
 * @param session   Session to use.
 * @param log_type  Log type of message (debug, info, warning, error).
 * @param fmt       Format string used for stdarg.h .
 */
void jlog_log_message(jlog_t *session, int log_type,
                      const char *fmt, ...);

/**
 * @brief Logs message with session, including source code info.
 *
 * If available, calls @c jlog_t#log_function_m .
 * 
 * @param session   Session to use.
 * @param log_type  Log type of message (debug, info, warning, error).
 * @param file      File name in which log was called.
 * @param function  Function name in which log was called.
 * @param line      Line number on which log was called.
 * @param fmt       Format string used for stdarg.h .
 */
void jlog_log_message_m(jlog_t *session, int log_type,
                        const char *file, const char *function, int line,
                        const char *fmt, ...);

/**
 * @brief Set global session variable.
 * 
 * Saves session as global variable. Session can be used to log with
 * @c jlog_global_ functions.
 * 
 * @param session Session object to save as global.
 */
void jlog_global_session_set(jlog_t *session);

/**
 * @brief Free global session object.
 * 
 * Calls @c jlog_session_free() with global session variable.
 * If global session variable is @c NULL , returns without doing anything.
 */
void jlog_global_session_free();

/**
 * @brief Log message via global session object.
 * 
 * Calls @c jlog_log_message() with global session variable.
 * If global session variable is @c NULL , returns without doing anything.
 *
 * @param log_type  Log type of message (debug, info, warning, error).
 * @param fmt       Format string used for stdarg.h .
 */
void jlog_global_log_message(int log_type, const char *fmt, ...);

/**
 * @brief Log message via global session object including source code info.
 *
 * Calls @c jlog_log_message_m() with global session variable.
 * If global session variable is @c NULL , returns without doing anything.
 * 
 * @param log_type  Log type of message (debug, info, warning, error).
 * @param file      File name in which log was called.
 * @param function  Function name in which log was called.
 * @param line      Line number on which log was called.
 * @param fmt       Format string used for stdarg.h .
 */
void jlog_global_log_message_m(int log_type, const char *file,
                               const char *function, int line,
                               const char *fmt, ...);

//==============================================================================
// Define global macros.
//==============================================================================

/**
 * @brief Sends global debug log with current code info.
 * 
 * Calls @c jlog_global_log_message_m() with log type @c #JLOG_LOGTYPE_DEBUG
 * and filename, function name and line number from where it was called.
 *
 * @param fmt Format string used for stdarg.h .
 */
#define JLOG_DEBUG(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_DEBUG, \
                             __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Sends global info log with current code info.
 * 
 * Calls @c jlog_global_log_message_m() with log type @c #JLOG_LOGTYPE_INFO
 * and filename, function name and line number from where it was called.
 *
 * @param fmt Format string used for stdarg.h .
 */
#define JLOG_INFO(fmt, ...) jlog_global_log_message_m( \JLOG_LOGTYPE_INFO, \
                            __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Sends global warning log with current code info.
 * 
 * Calls @c jlog_global_log_message_m() with log type @c #JLOG_LOGTYPE_WARN
 * and filename, function name and line number from where it was called.
 *
 * @param fmt Format string used for stdarg.h .
 */
#define JLOG_WARN(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_WARN, \
                            __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Sends global error log with current code info.
 * 
 * Calls @c jlog_global_log_message_m() with log type @c #JLOG_LOGTYPE_ERROR
 * and filename, function name and line number from where it was called.
 *
 * @param fmt Format string used for stdarg.h .
 */
#define JLOG_ERROR(fmt, ...) jlog_global_log_message_m(JLOG_LOGTYPE_ERROR, \
                             __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cpluslpus
} /* extern "C" */
#endif

#endif /* INCLUDE_JLOG_H */
/**
 * @file jlog_syslog.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jlog_syslog.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel nadji
 * 
 */

#include <jayc/jlog_syslog.h>
#include <jayc/jlog_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

//==============================================================================
// Define global variables.
//

/**
 * @brief Singleton session.
 * 
 */
static jlog_t *singleton_session;



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Handler to free memory.
 * 
 * Closes syslog connection and frees the session memory.
 * 
 * @param ctx Session memory to free.
 */
static void jlog_syslog_session_free_handler(void *ctx);

/**
 * @brief Handler to log message.
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param msg : Message string to log.
 */
static void jlog_syslog_message_handler(void *ctx, int log_type, const char *msg);

/**
 * @brief Handler to log message. Contains additional information (filename, function name, line number).
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param msg : Message string to log.
 */
static void jlog_syslog_message_handler_m(void *ctx, int log_type, const char *file, const char *function, int line, const char *msg);



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jlog_t *jlog_syslog_session_init(int log_level, const char *id, int facility)
{
  if(singleton_session == NULL)
  {
    jlog_t *new_session = (jlog_t *)malloc(sizeof(jlog_t));

    new_session->log_function = &jlog_syslog_message_handler;
    new_session->log_function_m = &jlog_syslog_message_handler_m;
    new_session->session_free_handler = &jlog_syslog_session_free_handler;
    new_session->log_level = log_level;
    new_session->session_context = NULL;
    openlog(id, 0, facility);
    singleton_session = new_session;
  }

  return singleton_session;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
void jlog_syslog_session_free_handler(void *ctx)
{
  closelog();
  singleton_session = NULL;
}

//------------------------------------------------------------------------------
//
void jlog_syslog_message_handler(void *ctx, int log_type, const char *msg)
{
  int type;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = LOG_DEBUG;
      break;
    }

    case JLOG_LOGTYPE_INFO:
    {
      type = LOG_INFO;
      break;
    }

    case JLOG_LOGTYPE_WARN:
    {
      type = LOG_WARNING;
      break;
    }

    case JLOG_LOGTYPE_ERROR:
    {
      type = LOG_ERR;
      break;
    }

    case JLOG_LOGTYPE_CRITICAL:
    {
      type = LOG_CRIT;
      break;
    }

    case JLOG_LOGTYPE_FATAL:
    {
      type = LOG_EMERG;
      break;
    }

    default:
    {
      type = LOG_DEBUG;
      break;
    }
  }

  syslog(type, "%s", msg);
}

//------------------------------------------------------------------------------
//
void jlog_syslog_message_handler_m(void *ctx, int log_type, const char *file, const char *function, int line, const char *msg)
{
  int type;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = LOG_DEBUG;
      break;
    }

    case JLOG_LOGTYPE_INFO:
    {
      type = LOG_INFO;
      break;
    }

    case JLOG_LOGTYPE_WARN:
    {
      type = LOG_WARNING;
      break;
    }

    case JLOG_LOGTYPE_ERROR:
    {
      type = LOG_ERR;
      break;
    }

    case JLOG_LOGTYPE_CRITICAL:
    {
      type = LOG_CRIT;
      break;
    }

    case JLOG_LOGTYPE_FATAL:
    {
      type = LOG_EMERG;
      break;
    }

    default:
    {
      type = LOG_DEBUG;
      break;
    }
  }

  syslog(type, "[ %s:%d %s() ] %s", file, line, function, msg);
}
#include <jlog_syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

static jlog_t *singleton_session;

static void jlog_syslog_session_free_handler(void *ctx);

/*******************************************************************************
 * @brief Handler to log message.
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param fmt : Format string used for stdarg.h .
 */
static void jlog_syslog_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...);

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
static void jlog_syslog_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...);

//------------------------------------------------------------------------------
//
jlog_t *jlog_syslog_session_init(uint8_t log_level, const char *id, int facility)
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

//------------------------------------------------------------------------------
//
void jlog_syslog_session_free_handler(void *ctx)
{
  closelog();
  singleton_session = NULL;
}

//------------------------------------------------------------------------------
//
void jlog_syslog_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...)
{
  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

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

    default:
    {
      type = LOG_DEBUG;
      break;
    }
  }

  syslog(type, "%s", buf);
}

//------------------------------------------------------------------------------
//
void jlog_syslog_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

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

    default:
    {
      type = LOG_DEBUG;
      break;
    }
  }

  syslog(type, "[ %s:%u %s() ] %s", file, line, function, buf);
}
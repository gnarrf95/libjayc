#include <jlog.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

static struct __jlog_session *global_session;

//------------------------------------------------------------------------------
//
void jlog_session_free(struct __jlog_session *session)
{
  if(session == NULL)
  {
    return;
  }

  if(session->free_handler)
  {
    session->free_handler(session->session_context);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
void jlog_log_message(struct __jlog_session *session, uint8_t log_type, const char *fmt, ...)
{
  if(session == NULL)
  {
    return;
  }

  if(log_type < session->log_level)
  {
    return;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  session->log_function(session->session_context, log_type, buf);
  va_end(args);
}

//------------------------------------------------------------------------------
//
void jlog_log_message_m(struct __jlog_session *session, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
  if(session == NULL)
  {
    return;
  }

  if(log_type < session->log_level)
  {
    return;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  session->log_function_m(session->session_context, log_type, file, function, line, buf);
  va_end(args);
}

//------------------------------------------------------------------------------
//
void jlog_global_session_set(struct __jlog_session *session)
{
  global_session = session;
}

//------------------------------------------------------------------------------
//
void jlog_global_session_free()
{
  if(global_session)
  {
    jlog_session_free(global_session);
  }
}

//------------------------------------------------------------------------------
//
void jlog_global_log_message(uint8_t log_type, const char *fmt, ...)
{
  if(global_session == NULL)
  {
    return;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  jlog_log_message(global_session, log_type, buf);
  va_end(args);
}

//------------------------------------------------------------------------------
//
void jlog_global_log_message_m(uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
  if(global_session == NULL)
  {
    return;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  jlog_log_message_m(global_session, log_type, file, function, line, buf);
  va_end(args);
}

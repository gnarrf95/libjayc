#include <jlog_stdio.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
//
jlog_t *jlog_stdio_init(uint8_t log_level)
{
  jlog_t *session = (jlog_t *)malloc(sizeof(jlog_t));

  session->log_function = &jlog_stdio_message_handler;
  session->log_function_m = &jlog_stdio_message_handler_m;
  session->free_handler = NULL;
  session->log_level = log_level;
  session->session_context = NULL;

  return session;
}

//------------------------------------------------------------------------------
//
void jlog_stdio_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...)
{
  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  char *type;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = "DBG";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = "INF";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = "WRN";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = "ERR";
      out_stream = stderr;
      break;
    }
    default:
    {
      type = "DBG";
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ =%s= ] %s\n", type, buf);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  char *type;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = "DBG";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = "INF";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = "WRN";
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = "ERR";
      out_stream = stderr;
      break;
    }
    default:
    {
      type = "DBG";
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ =%s= %s:%u %s() ] %s\n", type, file, line, function, buf);
}
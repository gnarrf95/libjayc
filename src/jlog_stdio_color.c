#include <jlog_stdio_color.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define JLOG_STDIO_COLOR_RESET "\033[0m"

//------------------------------------------------------------------------------
//
jlog_stdio_color_context_t *jlog_stdio_color_context_init(const char *debug_color, const char *info_color, const char *warn_color, const char *error_color)
{
  jlog_stdio_color_context_t *ctx = (jlog_stdio_color_context_t *)malloc(sizeof(jlog_stdio_color_context_t));

  if(debug_color)
  {
    ctx->debug_color = (char *)malloc(sizeof(char) * (strlen(debug_color) + 1));
    strcpy(ctx->debug_color, debug_color);
  }
  else
  {
    ctx->debug_color = NULL;
  }

  if(info_color)
  {
    ctx->info_color = (char *)malloc(sizeof(char) * (strlen(info_color) + 1));
    strcpy(ctx->info_color, info_color);
  }
  else
  {
    ctx->info_color = NULL;
  }

  if(warn_color)
  {
    ctx->warn_color = (char *)malloc(sizeof(char) * (strlen(warn_color) + 1));
    strcpy(ctx->warn_color, warn_color);
  }
  else
  {
    ctx->warn_color = NULL;
  }

  if(error_color)
  {
    ctx->error_color = (char *)malloc(sizeof(char) * (strlen(error_color) + 1));
    strcpy(ctx->error_color, error_color);
  }
  else
  {
    ctx->error_color = NULL;
  }

  return ctx;
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_context_free(jlog_stdio_color_context_t *ctx)
{
  if(ctx == NULL)
  {
    return;
  }

  if(ctx->debug_color)
  {
    free(ctx->debug_color);
  }

  if(ctx->info_color)
  {
    free(ctx->info_color);
  }

  if(ctx->warn_color)
  {
    free(ctx->warn_color);
  }

  if(ctx->error_color)
  {
    free(ctx->error_color);
  }

  free(ctx);
}

//------------------------------------------------------------------------------
//
jlog_t *jlog_stdio_color_init(uint8_t log_level, jlog_stdio_color_context_t *ctx)
{
  jlog_t *session = (jlog_t *)malloc(sizeof(jlog_t));

  session->log_function = &jlog_stdio_color_message_handler;
  session->log_function_m = &jlog_stdio_color_message_handler_m;
  session->free_handler = &jlog_stdio_color_free_handler;
  session->log_level = log_level;
  session->session_context = (void *)ctx;

  return session;
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_free_handler(void *ctx)
{
  jlog_stdio_color_context_free((jlog_stdio_color_context_t *)ctx);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_message_handler(void *ctx, uint8_t log_type, const char *fmt, ...)
{
  if(ctx == NULL)
  {
    return;
  }

  jlog_stdio_color_context_t *color_context = (jlog_stdio_color_context_t *)ctx;

  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  char *type;
  char *color = JLOG_STDIO_COLOR_RESET;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = "DBG";
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = "INF";
      if(color_context->info_color)
      {
        color = color_context->info_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = "WRN";
      if(color_context->warn_color)
      {
        color = color_context->warn_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = "ERR";
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    default:
    {
      type = "DBG";
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ =%s%s%s= ] %s%s%s\n", color, type, JLOG_STDIO_COLOR_RESET, color, buf, JLOG_STDIO_COLOR_RESET);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_message_handler_m(void *ctx, uint8_t log_type, const char *file, const char *function, uint32_t line, const char *fmt, ...)
{
  if(ctx == NULL)
  {
    return;
  }

  jlog_stdio_color_context_t *color_context = (jlog_stdio_color_context_t *)ctx;

  va_list args;
  char buf[2048] = { 0 };

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  char *type;
  char *color = JLOG_STDIO_COLOR_RESET;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = "DBG";
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = "INF";
      if(color_context->info_color)
      {
        color = color_context->info_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = "WRN";
      if(color_context->warn_color)
      {
        color = color_context->warn_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = "ERR";
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    default:
    {
      type = "DBG";
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ =%s%s%s= %s:%u %s() ] %s%s%s\n", color, type, JLOG_STDIO_COLOR_RESET, file, line, function, color, buf, JLOG_STDIO_COLOR_RESET);
}
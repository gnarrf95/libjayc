/**
 * @file jlog_stdio.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jlog_stdio.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jlog_stdio.h>
#include <jlog_dev.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define JLOG_STDIO_COLOR_RESET "\033[0m"

#define JLOG_STDIO_LOGSTRING_DEBUG "=DBG="
#define JLOG_STDIO_LOGSTRING_INFO "=INF="
#define JLOG_STDIO_LOGSTRING_WARN "=WRN="
#define JLOG_STDIO_LOGSTRING_ERROR "=ERR="
#define JLOG_STDIO_LOGSTRING_CRITICAL "*CRT*"
#define JLOG_STDIO_LOGSTRING_FATAL "**FATAL**"

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

/**
 * @brief Allocates memory for color string and copies from param.
 * 
 * This was implemented, because @c strcpy() triggers
 * devskim warnings.
 * 
 * @param str String to copy.
 * 
 * @return    New allocated string.
 * @return    @c NULL , if an error occured.
 */
static char *jlog_stdio_color_init_colorString(const char *str);

/*******************************************************************************
 * @brief Destroys jlog_stdio_color_context object.
 * 
 * @param ctx : Object to destroy.
 */
static void jlog_stdio_color_context_free(void *ctx);

/*******************************************************************************
 * @brief Destroys context for jlog_stdio_color session.
 * 
 * @param ctx: Session context to destroy.
 */
static void jlog_stdio_color_session_free_handler(void *ctx);

/*******************************************************************************
 * @brief Handler to log message.
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param msg : Message string to log.
 */
static void jlog_stdio_message_handler(void *ctx, int log_type, const char *msg);

/*******************************************************************************
 * @brief Handler to log message. Contains additional information (filename, function name, line number).
 *
 * @param ctx : Session context (not used in this logger).
 * @param log_type : Log type of message (debug, info, warning, error).
 * @param file : File name in which log was called.
 * @param function : Function name in which log was called.
 * @param line : Line number on which log was called.
 * @param msg : Message string to log.
 */
static void jlog_stdio_message_handler_m(void *ctx, int log_type, const char *file, const char *function, int line, const char *msg);

static void jlog_stdio_print(int log_type, const char *msg);
static void jlog_stdio_print_m(int log_type, const char *file, const char *function, int line, const char *msg);

static void jlog_stdio_color_print(jlog_stdio_color_context_t *color_context, int log_type, const char *msg);
static void jlog_stdio_color_print_m(jlog_stdio_color_context_t *color_context, int log_type, const char *file, const char *function, int line, const char *msg);

//------------------------------------------------------------------------------
//
void *jlog_stdio_color_context_init(const char *debug_color, const char *info_color, const char *warn_color, const char *error_color)
{
  jlog_stdio_color_context_t *ctx = (jlog_stdio_color_context_t *)malloc(sizeof(jlog_stdio_color_context_t));

  if(debug_color)
  {
    ctx->debug_color = jlog_stdio_color_init_colorString(debug_color);
  }
  else
  {
    ctx->debug_color = NULL;
  }

  if(info_color)
  {
    ctx->info_color = jlog_stdio_color_init_colorString(info_color);
  }
  else
  {
    ctx->info_color = NULL;
  }

  if(warn_color)
  {
    ctx->warn_color = jlog_stdio_color_init_colorString(warn_color);
  }
  else
  {
    ctx->warn_color = NULL;
  }

  if(error_color)
  {
    ctx->error_color = jlog_stdio_color_init_colorString(error_color);
  }
  else
  {
    ctx->error_color = NULL;
  }

  return (void *)ctx;
}

//------------------------------------------------------------------------------
//
char *jlog_stdio_color_init_colorString(const char *str)
{
  if(str == NULL)
  {
    return NULL;
  }

  size_t size_ret = sizeof(char) * (strlen(str) + 1);
  char *ret = (char *)malloc(size_ret);
  if(ret == NULL)
  {
    return NULL;
  }

  memset(ret, 0, size_ret);
  memcpy(ret, str, size_ret);

  return ret;
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_context_free(void *ctx)
{
  if(ctx == NULL)
  {
    return;
  }

  jlog_stdio_color_context_t *color_context = (jlog_stdio_color_context_t *)ctx;

  if(color_context->debug_color)
  {
    free(color_context->debug_color);
  }

  if(color_context->info_color)
  {
    free(color_context->info_color);
  }

  if(color_context->warn_color)
  {
    free(color_context->warn_color);
  }

  if(color_context->error_color)
  {
    free(color_context->error_color);
  }

  free(color_context);
}

//------------------------------------------------------------------------------
//
jlog_t *jlog_stdio_session_init(int log_level)
{
  jlog_t *session = (jlog_t *)malloc(sizeof(jlog_t));

  session->log_function = &jlog_stdio_message_handler;
  session->log_function_m = &jlog_stdio_message_handler_m;
  session->session_free_handler = NULL;
  session->log_level = log_level;
  session->session_context = NULL;

  return session;
}

//------------------------------------------------------------------------------
//
jlog_t *jlog_stdio_color_session_init(int log_level, void *ctx)
{
  jlog_t *session = (jlog_t *)malloc(sizeof(jlog_t));

  session->log_function = &jlog_stdio_message_handler;
  session->log_function_m = &jlog_stdio_message_handler_m;
  session->session_free_handler = &jlog_stdio_color_session_free_handler;
  session->log_level = log_level;
  session->session_context = ctx;

  return session;
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_session_free_handler(void *ctx)
{
  jlog_stdio_color_context_free(ctx);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_message_handler(void *ctx, int log_type, const char *msg)
{
  if(ctx)
  {
    jlog_stdio_color_print((jlog_stdio_color_context_t *)ctx, log_type, msg);
  }
  else
  {
    jlog_stdio_print(log_type, msg);
  }
}

//------------------------------------------------------------------------------
//
void jlog_stdio_message_handler_m(void *ctx, int log_type, const char *file, const char *function, int line, const char *msg)
{
  if(ctx)
  {
    jlog_stdio_color_print_m((jlog_stdio_color_context_t *)ctx, log_type, file, function, line, msg);
  }
  else
  {
    jlog_stdio_print_m(log_type, file, function, line, msg);
  }
}

//------------------------------------------------------------------------------
//
void jlog_stdio_print(int log_type, const char *msg)
{
  char *type;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = JLOG_STDIO_LOGSTRING_INFO;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = JLOG_STDIO_LOGSTRING_WARN;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = JLOG_STDIO_LOGSTRING_ERROR;
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_CRITICAL:
    {
      type = JLOG_STDIO_LOGSTRING_CRITICAL;
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_FATAL:
    {
      type = JLOG_STDIO_LOGSTRING_FATAL;
      out_stream = stderr;
      break;
    }
    default:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ %s ] %s\n", type, msg);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_print_m(int log_type, const char *file, const char *function, int line, const char *msg)
{
  char *type;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = JLOG_STDIO_LOGSTRING_INFO;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = JLOG_STDIO_LOGSTRING_WARN;
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = JLOG_STDIO_LOGSTRING_ERROR;
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_CRITICAL:
    {
      type = JLOG_STDIO_LOGSTRING_CRITICAL;
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_FATAL:
    {
      type = JLOG_STDIO_LOGSTRING_FATAL;
      out_stream = stderr;
      break;
    }
    default:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ %s %s:%d %s() ] %s\n", type, file, line, function, msg);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_print(jlog_stdio_color_context_t *color_context, int log_type, const char *msg)
{
  char *type;
  char *color = JLOG_STDIO_COLOR_RESET;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = JLOG_STDIO_LOGSTRING_INFO;
      if(color_context->info_color)
      {
        color = color_context->info_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = JLOG_STDIO_LOGSTRING_WARN;
      if(color_context->warn_color)
      {
        color = color_context->warn_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = JLOG_STDIO_LOGSTRING_ERROR;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_CRITICAL:
    {
      type = JLOG_STDIO_LOGSTRING_CRITICAL;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_FATAL:
    {
      type = JLOG_STDIO_LOGSTRING_FATAL;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    default:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ %s%s%s ] %s%s%s\n", color, type, JLOG_STDIO_COLOR_RESET, color, msg, JLOG_STDIO_COLOR_RESET);
}

//------------------------------------------------------------------------------
//
void jlog_stdio_color_print_m(jlog_stdio_color_context_t *color_context, int log_type, const char *file, const char *function, int line, const char *msg)
{
  char *type;
  char *color = JLOG_STDIO_COLOR_RESET;
  FILE *out_stream;
  switch(log_type)
  {
    case JLOG_LOGTYPE_DEBUG:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_INFO:
    {
      type = JLOG_STDIO_LOGSTRING_INFO;
      if(color_context->info_color)
      {
        color = color_context->info_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_WARN:
    {
      type = JLOG_STDIO_LOGSTRING_WARN;
      if(color_context->warn_color)
      {
        color = color_context->warn_color;
      }
      out_stream = stdout;
      break;
    }
    case JLOG_LOGTYPE_ERROR:
    {
      type = JLOG_STDIO_LOGSTRING_ERROR;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_CRITICAL:
    {
      type = JLOG_STDIO_LOGSTRING_CRITICAL;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    case JLOG_LOGTYPE_FATAL:
    {
      type = JLOG_STDIO_LOGSTRING_FATAL;
      if(color_context->error_color)
      {
        color = color_context->error_color;
      }
      out_stream = stderr;
      break;
    }
    default:
    {
      type = JLOG_STDIO_LOGSTRING_DEBUG;
      if(color_context->debug_color)
      {
        color = color_context->debug_color;
      }
      out_stream = stdout;
      break;
    }
  }

  fprintf(out_stream, "[ %s%s%s %s:%d %s() ] %s%s%s\n", color, type, JLOG_STDIO_COLOR_RESET, file, line, function, color, msg, JLOG_STDIO_COLOR_RESET);
}
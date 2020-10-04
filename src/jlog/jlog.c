/**
 * @file jlog.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Definition of functions for jlog interface.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jlog.h>
#include <jayc/jlog_dev.h>
#include <jayc/jproc.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief Global session object.
 * 
 * Used for global functions, so that logger can be accessed
 * from anywhere in program.
 */
static struct __jlog_session *global_session;

//------------------------------------------------------------------------------
//
jlog_t *jlog_session_quiet()
{
  jlog_t *session = (jlog_t *)malloc(sizeof(jlog_t));
  if(session == NULL)
  {
    return NULL;
  }

  session->log_function = NULL;
  session->log_function_m = NULL;
  session->session_free_handler = NULL;
  session->log_level = 0;
  session->session_context = NULL;

  return session;
}

//------------------------------------------------------------------------------
//
void jlog_session_free(struct __jlog_session *session)
{
  if(session == NULL)
  {
    return;
  }

  if(session->session_free_handler)
  {
    session->session_free_handler(session->session_context);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
void jlog_log_message(struct __jlog_session *session, int log_type, const char *fmt, ...)
{
  if(session == NULL)
  {
    return;
  }

  if(session->log_function == NULL)
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

  if(log_type == JLOG_LOGTYPE_FATAL)
  {
    jproc_exit(EXIT_FAILURE);
  }

  #ifdef JLOG_EXIT_ATCRITICAL
  if(log_type == JLOG_LOGTYPE_CRITICAL)
  {
    jproc_exit(EXIT_FAILURE);
  }
  #endif /* JLOG_EXIT_ATCRITICAL */

  #ifdef JLOG_EXIT_ATERROR
  if(log_type == JLOG_LOGTYPE_ERROR)
  {
    jproc_exit(EXIT_FAILURE);
  }
  #endif /* JLOG_EXIT_ATERROR */
}

//------------------------------------------------------------------------------
//
void jlog_log_message_m(struct __jlog_session *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  if(session == NULL)
  {
    return;
  }

  if(session->log_function_m == NULL)
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

  if(log_type == JLOG_LOGTYPE_FATAL)
  {
    jproc_exit(EXIT_FAILURE);
  }

  #ifdef JLOG_EXIT_ATCRITICAL
  if(log_type == JLOG_LOGTYPE_CRITICAL)
  {
    jproc_exit(EXIT_FAILURE);
  }
  #endif /* JLOG_EXIT_ATCRITICAL */

  #ifdef JLOG_EXIT_ATERROR
  if(log_type == JLOG_LOGTYPE_ERROR)
  {
    jproc_exit(EXIT_FAILURE);
  }
  #endif /* JLOG_EXIT_ATERROR */
}

//------------------------------------------------------------------------------
//
void jlog_global_session_set(struct __jlog_session *session)
{
  if(global_session)
  {
    jlog_session_free(global_session);
  }

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
void jlog_global_log_message(int log_type, const char *fmt, ...)
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
void jlog_global_log_message_m(int log_type, const char *file, const char *function, int line, const char *fmt, ...)
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

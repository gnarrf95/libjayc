/**
 * @file jutil_io.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jutil_io.
 * 
 * @date 2020-10-10
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jutil_io.h>
#include <jayc/jutil_io_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
//
void jutil_io_free(jutil_io_t *session)
{
  if(session == NULL)
  {
    return;
  }

  if(session->session_free_handler)
  {
    session->session_free_handler(session->session_ctx);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jutil_io_print(jutil_io_t *session, const char *fmt, ...)
{
  if(session == NULL)
  {
    return false;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session->function_print == NULL)
  {
    return false;
  }

  return session->function_print(session->session_ctx, buf);
}

//------------------------------------------------------------------------------
//
int jutil_io_printLine(jutil_io_t *session, const char *fmt, ...)
{
  if(session == NULL)
  {
    return false;
  }

  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session->function_printLine == NULL)
  {
    return false;
  }

  return session->function_printLine(session->session_ctx, buf);
}

//------------------------------------------------------------------------------
//
size_t jutil_io_read(jutil_io_t *session, char *buffer, size_t buf_size)
{
  if(session == NULL)
  {
    return 0;
  }

  if(session->function_read == NULL)
  {
    return 0;
  }

  return session->function_read(session->session_ctx, buffer, buf_size);
}

//------------------------------------------------------------------------------
//
int jutil_io_readLine(jutil_io_t *session, char **buf_ptr, size_t *buf_size)
{
  if(session == NULL)
  {
    return false;
  }

  if(buf_ptr == NULL || buf_size == NULL)
  {
    return false;
  }

  if(session->function_readLine == NULL)
  {
    return false;
  }

  return session->function_readLine(session->session_ctx, buf_ptr, buf_size);
}
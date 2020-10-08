/**
 * @file jutil_cli.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jutil_cli.
 * 
 * @date 2020-10-07
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 200809L /* Needed for getline() */

#include <jayc/jutil_cli.h>
#include <jayc/jlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

//==============================================================================
// Define constants and structures.
//

/**
 * @brief Maximum number of space seperated arguments.
 */
#define JUTIL_CLI_ARGS_MAX 16

/**
 * @brief Delimiter characters for @c strtok() .
 */
#define JUTIL_CLI_DELIMS " "

/**
 * @brief Session object.
 */
struct __jutil_cli_session
{
  jutil_cli_cmdHandler_t handler; /**< Handler to call, when data recieved. */
  void *session_ctx;              /**< Context pointer to pass to handler. */
};



//==============================================================================
// Define log macros.
//

#ifdef JUTIL_NO_DEBUG /* Allow to disable debug messages at compile time. */
  #define DEBUG(fmt, ...)
#else
  #define DEBUG(fmt, ...) JLOG_DEBUG(fmt, ##__VA_ARGS__)
#endif
#define INFO(fmt, ...) JLOG_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) JLOG_WARN(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) JLOG_ERROR(fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...)JLOG_CRITICAL(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) JLOG_FATAL(fmt, ##__VA_ARGS__)



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Reads line from input stream.
 * 
 * @param input_stream  Input stream to read from.
 * 
 * @return              Allocated string with input.
 * @return              @c NULL , if empty input or error occured.
 */
static char *jutil_cli_getInputString(FILE *input_stream);



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jutil_cli_t *jutil_cli_init(jutil_cli_cmdHandler_t handler, void *ctx)
{
  if(handler == NULL)
  {
    return NULL;
  }

  jutil_cli_t *session = (jutil_cli_t *)malloc(sizeof(jutil_cli_t));
  if(session == NULL)
  {
    return NULL;
  }

  session->handler = handler;
  session->session_ctx = ctx;

  return session;
}

//------------------------------------------------------------------------------
//
void jutil_cli_free(jutil_cli_t *session)
{
  if(session == NULL)
  {
    return;
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jutil_cli_run(jutil_cli_t *session)
{
  char *cmd_str = jutil_cli_getInputString(stdin);
  if(cmd_str == NULL)
  {
    return false;
  }

  /* Processing arguments. */
  char *args_buf[JUTIL_CLI_ARGS_MAX];
  size_t arg_size;

  char *arg_buf = strtok(cmd_str, JUTIL_CLI_DELIMS);
  for(arg_size = 0; arg_buf != NULL; arg_size++)
  {
    /* Check not to overflow args_buf. */
    if(arg_size >= JUTIL_CLI_ARGS_MAX)
    {
      DEBUG("Too many arguments [%lu].", arg_size+1);
      break;
    }

    size_t len_arg = strlen(arg_buf);

    /* Check if string empty. */
    if(len_arg == 0)
    {
      ERROR("Read empty string [ctr = %lu].", arg_size);
      break;
    }

    args_buf[arg_size] = (char *)malloc(sizeof(char) * (len_arg+1));
    if(args_buf[arg_size] == NULL)
    {
      ERROR("malloc() failed.");
      break;
    }

    memset(args_buf[arg_size], 0, len_arg+1);
    memcpy(args_buf[arg_size], arg_buf, len_arg);

    arg_buf = strtok(NULL, JUTIL_CLI_DELIMS);
  }

  free(cmd_str);
  int ret = session->handler((const char **)args_buf, arg_size, session->session_ctx);

  size_t ctr;
  for(ctr = 0; ctr < arg_size; ctr++)
  {
    free(args_buf[ctr]);
  }

  return ret;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
char *jutil_cli_getInputString(FILE *input_stream)
{
  char *line_ptr = NULL;
  size_t line_size = 0;

  /* Read line. */
  if(getline(&line_ptr, &line_size, input_stream) < 0)
  {
    if(errno == EINTR)
    {
      DEBUG("getline() interrupted.");
    }
    else
    {
      ERROR("getline() failed [%d : %s].", errno, strerror(errno));
    }
    free(line_ptr);
    return NULL;
  }

  /* Check for empty lines. */
  if(line_ptr[0] == '\n')
  {
    free(line_ptr);
    return NULL;
  }

  /* Create buffer. */
  char *cmd_str = (char *)malloc(sizeof(char) * line_size);
  if(cmd_str == NULL)
  {
    ERROR("malloc() failed.");
    free(line_ptr);
    return NULL;
  }

  /* Remove newline byte. */
  memset(cmd_str, 0, line_size);
  if(sscanf(line_ptr, "%[^\n]\n", cmd_str) != 1)
  {
    ERROR("sscanf() failed.");
    free(line_ptr);
    free(cmd_str);
    return false;
  }

  free(line_ptr);

  return cmd_str;
}
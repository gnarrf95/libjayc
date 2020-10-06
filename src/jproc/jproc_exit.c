/**
 * @file jproc_exit.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements exit functionality for jproc.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jproc.h>
#include <jayc/jlog.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct __jproc_exit_function_pair
{
  jproc_exit_handler_t handler;
  void *ctx;
} jproc_exit_function_t;

static jproc_exit_function_t exit_function = { NULL, NULL };

//------------------------------------------------------------------------------
//
void jproc_exit(int exit_value)
{
  if(exit_function.handler)
  {
    exit_function.handler(exit_value, exit_function.ctx);
  }

  jlog_global_session_free();
  exit(exit_value);
}

//------------------------------------------------------------------------------
//
int jproc_exit_setHandler(jproc_exit_handler_t handler, void *ctx)
{
  exit_function.handler = handler;
  exit_function.ctx = ctx;

  return true;
}
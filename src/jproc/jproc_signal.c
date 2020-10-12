/**
 * @file jproc_signal.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements signal functionality for jproc.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jproc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

typedef struct __jproc_signal_handlerStruct
{
  jproc_signal_handler_t handler;
  void *ctx;
} jproc_signal_handlerStruct_t;

static jproc_signal_handlerStruct_t jproc_signal_handlers[32] = { { NULL, NULL } };

static void jproc_signalHandler(int signum);

//------------------------------------------------------------------------------
//
int jproc_signal_setHandler(int signal_number, jproc_signal_handler_t handler, void *ctx)
{
  if(signal_number >= 32 || signal_number < 0)
  {
    return false;
  }

  jproc_signal_handlers[signal_number].handler = handler;
  jproc_signal_handlers[signal_number].ctx = ctx;

  signal(signal_number, jproc_signalHandler);

  return true;
}

//------------------------------------------------------------------------------
//
void jproc_signalHandler(int signum)
{
  if(signum >= 32 || signum < 0)
  {
    return;
  }

  if(jproc_signal_handlers[signum].handler)
  {
    jproc_signal_handlers[signum].handler(signum, jproc_signal_handlers[signum].ctx);
  }
}
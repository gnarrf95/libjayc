#include <jayc/jproc.h>
#include <jayc/jlog_stdio.h>
#include <stddef.h>

/* Values for program exit. */
#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

/* Interrupt to stop program. */
#define SIGNAL_INTERRUPT 2

/* Tells program loop to keep going. */
static int run_loop = 1;

/**
 * @brief Triggered by signal. Stops program loop.
 * 
 * @param signum  Signal caught.
 * @param ctx     User context.
 */
static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Caught signal [%d].", signum);
  run_loop = 0; /*< Stop loop. */
}

/**
 * @brief Gets called when jproc_exit() is called.
 * 
 * Automatically frees global jlog session.
 * Should be used to free any allocated
 * global memory.
 * 
 * @param exit_value  Value with which program will exit.
 * @param ctx         User context.
 */
static void exit_handler(int exit_value, void *ctx)
{
  JLOG_INFO("Exiting with value [%d].", exit_value);
}

int main()
{
  // Set exit handler at start of program (no context).
  jproc_exit_setHandler(&exit_handler, NULL);

  // Set global logger (will be freed at exit).
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_INFO);
  if(logger == NULL)
  {
    // Will cleanup before exiting.
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  // Create signal handler for SIGINT before program loop (no context).
  jproc_signal_setHandler(SIGNAL_INTERRUPT, &signal_handler, NULL);
  while(run_loop)
  {
    JLOG_INFO("Do stuff ...");
  }

  // When signal is caught, loop will stop and program will exit.
  jproc_exit(EXITVALUE_SUCCESS);
}
#include <jayc/jutil_thread.h>
#include <jayc/jutil_time.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define SIGNAL_INTERRUPT 2

#define THREAD_SLEEP 1000000000L

static int g_run = 1;

static jutil_thread_t *g_thread = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_thread)
  {
    jutil_thread_free(g_thread);
  }
}

static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Caught signal [%d], stopping loop.", signum);
  g_run = 0;
}

static int thread_function(void *ctx, jutil_thread_t *thread_session)
{
  JLOG_INFO("Hello, this is thread.");
  return 1;
}

int main()
{
  jproc_exit_setHandler(&exit_handler, NULL);
  jproc_signal_setHandler(SIGNAL_INTERRUPT, &signal_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  /* Initialize thread. */
  g_thread = jutil_thread_init(&thread_function, logger, THREAD_SLEEP, NULL);
  if(g_thread == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Start thread. */
  if(jutil_thread_start(g_thread) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Wait until signal arrives. */
  while(g_run)
  {
    jutil_time_sleep(1, 0, 0);
  }

  jutil_thread_free(g_thread);
  g_thread = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
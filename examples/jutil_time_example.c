#include <jayc/jutil_time.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define SIGNAL_INTERRUPT 2

int g_run = 1;

static jutil_time_timer_t *g_timer = NULL;
static jutil_time_stopWatch_t *g_stop_watch = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_timer)
  {
    jutil_time_timer_free(g_timer);
  }
  if(g_stop_watch)
  {
    jutil_time_stopWatch_free(g_stop_watch);
  }
}

static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Caught signal [%d], stopping loop.", signum);
  g_run = 0;
}

static int timer_handler(void *ctx)
{
  char time_buf[32] = { 0 };
  jutil_time_getCurrentTimeString(time_buf, sizeof(time_buf));

  /* Print elapsed time. */
  JLOG_INFO("[%s] : [%lu] milliseconds since timer started.",
    time_buf,
    jutil_time_stopWatch_check(g_stop_watch)
  );

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

  /* Initialize timer executes timer_handler() every 2 seconds. */
  g_timer = jutil_time_timer_init(NULL, timer_handler, 2, 0);
  if(g_timer == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Initialize stop_watch. */
  g_stop_watch = jutil_time_stopWatch_init();
  if(g_stop_watch == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Start timer and reset stopwatch. */
  if(jutil_time_timer_start(g_timer) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jutil_time_stopWatch_reset(g_stop_watch);

  /* Wait until signal is caught. */
  while(g_run)
  {
    /* Sleep for every second. */
    jutil_time_sleep(1, 0, 0);
  }

  /* After signal free everything. */
  jutil_time_timer_free(g_timer);
  g_timer = NULL;

  jutil_time_stopWatch_free(g_stop_watch);
  g_stop_watch = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
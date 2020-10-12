#include <jayc/jutil_cli.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static int g_run = 1;

static jutil_cli_t *g_cli = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_cli)
  {
    jutil_cli_free(g_cli);
  }
}

static int cli_handler(const char **args, size_t arg_size, void *ctx);

int main()
{
  jproc_exit_setHandler(&exit_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  g_cli = jutil_cli_init(&cli_handler, NULL, NULL);
  if(g_cli == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  while(g_run)
  {
    jutil_cli_run(g_cli);
  }

  jutil_cli_free(g_cli);
  g_cli = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}

int cli_handler(const char **args, size_t arg_size, void *ctx)
{
  if(arg_size == 0)
  {
    return 0;
  }

  size_t ctr;
  for(ctr = 0; ctr < arg_size; ctr++)
  {
    JLOG_INFO("CLI ARG [%lu] --> [%s].", ctr, args[ctr]);
  }

  if(strcmp(args[0], "exit") == 0)
  {
    g_run = 0;
  }

  return 0;
}
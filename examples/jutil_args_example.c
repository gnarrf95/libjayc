#include <jayc/jutil_args.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static char *optionHandler_a(const char **data, size_t data_size);
static char *optionHandler_b(const char **data, size_t data_size);
static char *optionHandler_c(const char **data, size_t data_size);

/* Define program description. */
static jutil_args_progDesc_t prog_desc =
{
  "jutil_args_example",
  "Shows how to use the jutil_args module.",
  "v1.0",
  "Manuel Nadji (https://github.com/gnarrf95)",
  "Copyright (c) 2020 by Manuel Nadji"
};

/* Define options and parameters. */
static jutil_args_option_t options[] =
{
  {
    "Option A",
    "A optional option without any parameters.",
    "op-a",
    'a',
    &optionHandler_a,
    0,
    0,
    0,
    JUTIL_ARGS_OPTIONPARAM_EMPTY
  },
  {
    "Option B",
    "A mandatory option without any parameters.",
    "op-b",
    0,
    &optionHandler_b,
    0,
    1,
    0,
    JUTIL_ARGS_OPTIONPARAM_EMPTY
  },
  {
    "Option C",
    "A optional option, which prints its arguments.",
    NULL,
    'c',
    &optionHandler_c,
    0,
    0,
    0,
    {
      {
        "Argument 1",
        "Gets printed."
      },
      {
        "Argument 2",
        "Also gets printed."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  }
};

/* Implement option handlers. */
char *optionHandler_a(const char **data, size_t data_size)
{
  JLOG_INFO("Option A was called.");
  return NULL;
}

char *optionHandler_b(const char **data, size_t data_size)
{
  JLOG_INFO("Option B was called.");
  return NULL;
}

char *optionHandler_c(const char **data, size_t data_size)
{
  if(data == NULL)
  {
    return jutil_args_error("Data array is NULL.");
  }
  if(data_size != 2)
  {
    return jutil_args_error("Invalid number of arguments [%lu].", data_size);
  }

  size_t ctr;
  for(ctr = 0; ctr < data_size; ctr++)
  {
    JLOG_INFO("Argument [%lu] is [%s].", ctr, data[ctr]);
  }
  return NULL;
}

/* Main function needs CLI arguments. */
int main(int argc, char *argv[])
{
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  int ret_check = jutil_args_process
  (
    &prog_desc,
    argc,
    argv,
    options,
    sizeof(options)/sizeof(jutil_args_option_t)
  );

  if(ret_check == 0)
  {
    JLOG_WARN("jutil_args_process() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  JLOG_INFO("SUCCESS !!!");
  jproc_exit(EXITVALUE_SUCCESS);
}
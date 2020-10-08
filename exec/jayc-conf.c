/**
 * @file jayc-conf.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief CLI tool to edit jconfig files.
 * 
 * 
 * 
 * @date 2020-10-03
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

/* Needed for getline() */
#define _POSIX_C_SOURCE 200809L

#include <jayc/jconfig.h>
#include <jayc/jproc.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jutil_args.h>
#include <jayc/jutil_cli.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

//==============================================================================
// Define constants.
//

/*
 * Exit values for jproc_exit().
 */
#define JAYCCONF_EXIT_SUCCESS 0
#define JAYCCONF_EXIT_FAILURE 1
#define JAYCCONF_EXIT_SIGINT  2

/*
 * File formats to read config.
 */
#define JAYCCONF_FORMAT_RAW 0

/*
 * CLI commands to edit config.
 */
#define JAYCCONF_CMD_LOAD   "lod"
#define JAYCCONF_CMD_SAVE   "sav"
#define JAYCCONF_CMD_SET    "set"
#define JAYCCONF_CMD_GET    "get"
#define JAYCCONF_CMD_DELETE "del"
#define JAYCCONF_CMD_DUMP   "dmp"
#define JAYCCONF_CMD_EXIT   "exit"
#define JAYCCONF_CMD_HELP   "help"

/*
 * States of CLI input parsing.
 */
#define JAYCCONF_CMDSTATE_CMD    0
#define JAYCCONF_CMDSTATE_PARAM1 1
#define JAYCCONF_CMDSTATE_PARAM2 2



//==============================================================================
// Define structures.
//

typedef struct __jaycConf_data
{
  char filename[128];
  int file_format;

  jconfig_t *config_data;
  jutil_cli_t *cli;

  int log_level;

  int run;
} jaycConf_data_t;



//==============================================================================
// Define log macros.
//

#define DEBUG(fmt, ...) JLOG_DEBUG(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) JLOG_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) JLOG_WARN(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) JLOG_ERROR(fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...)JLOG_CRITICAL(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) JLOG_FATAL(fmt, ##__VA_ARGS__)



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Initializes global data.
 * 
 */
static void jaycConf_initData();

/**
 * @brief Frees global data.
 * 
 * Used in @c #jaycConf_exitHandler .
 */
static void jaycConf_freeData();

/**
 * @brief Manages freeing memory and cleaning up, when program exits.
 * 
 * @param exit_value  Exit value for @c exit() .
 * @param ctx         Context pointer provided by user.
 */
static void jaycConf_exitHandler(int exit_value, void *ctx);

/**
 * @brief Handles SIGINT signal.
 * 
 * @param signal_number Signal caught (should be SIGINT).
 * @param ctx           Context pointer provided by user.
 */
static void jaycConf_signalHandler(int signal_number, void *ctx);

/**
 * @brief Handles filename argument.
 * 
 * @param data      Filename and fileformat in string array.
 * @param data_size Size of array (should be 2).
 * 
 * @return          @c NULL , if everything worked.
 * @return          Error message if something went wrong.
 */
static char *jaycConf_argFile(const char **data, size_t data_size);

/**
 * @brief Handles debug argument.
 * 
 * By default no debug logs are shown.
 * With argument "--debug" they can be enabled.
 * 
 * @param data      Empty array.
 * @param data_size Size of array (should be 0).
 * 
 * @return          @c NULL , if everything worked.
 * @return          Error message if something went wrong.
 */
static char *jaycConf_argDebug(const char **data, size_t data_size);

/**
 * @brief Loads config data from file.
 * 
 * @param file    File path.
 * @param format  File format constant.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jaycConf_loadConfig(const char *file, int format);

/**
 * @brief Saves config data to file.
 * 
 * @param file    File path.
 * @param format  File format constant.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jaycConf_saveConfig(const char *file, int format);

/**
 * @brief Prints all config data.
 * 
 * @param         If hierarchical data, prefix
 *                can be used to only search for
 *                keys, that start with prefix.
 *                If @c NULL , dump all keys.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jaycConf_dumpConfig(const char *prefix);

/**
 * @brief Handles CLI input.
 * 
 * @param args      Arguments read from CLI.
 * @param arg_size  Number of arguments read.
 * @param ctx       Context pointer (not used).
 * 
 * @return          Always @c 0 .
 */
static int jaycConf_cliHandler(const char **args, size_t arg_size, void *ctx);

/**
 * @brief Prints information about CLI commands.
 */
static void jaycConf_cli_printHelp();



//==============================================================================
// Define global data.
//

/**
 * @brief Program description for --help function.
 */
static jutil_args_progDesc_t prog_desc =
{
  "jayc-conf",

  "Program that can edit configurations and save/read " \
  "to/from files.",

  "v0.6-alpha",
  "Manuel Nadji (https://github.com/gnarrf95)",
  "Copyright (c) 2020 by Manuel Nadji"
};

/**
 * @brief Option structures for defining CLI options for jutil_args.
 */
static jutil_args_option_t jaycConf_argOptions[] =
{
  {
    "Config file",
    "File to open and format to parse.",
    "file",
    'f',
    &jaycConf_argFile,
    0,
    0,
    0,
    {
      {
        "filename",
        "File to read config from."
      },
      {
        "file-format",
        "Format which to parse."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  },
  {
    "Debug output",
    "Enable debug output.",
    "debug",
    0,
    &jaycConf_argDebug,
    0,
    0,
    0,
    JUTIL_ARGS_OPTIONPARAM_EMPTY
  }
};

/**
 * @brief Global data.
 */
static jaycConf_data_t g_data =
{
  { 0 },
  JAYCCONF_FORMAT_RAW,
  NULL,
  NULL,
  JLOG_LOGTYPE_INFO,
  true
};



//==============================================================================
// Implement main function.
//

int main(int argc, char *argv[])
{
  jaycConf_initData();
  if(jutil_args_process
    (
      &prog_desc,
      argc,
      argv,
      jaycConf_argOptions,
      sizeof(jaycConf_argOptions)/sizeof(jutil_args_option_t)
    ) == false)
  {
    jproc_exit(JAYCCONF_EXIT_FAILURE);
  }

  jlog_t *logger = jlog_stdio_session_init(g_data.log_level);
  if(logger == NULL)
  {
    jproc_exit(JAYCCONF_EXIT_FAILURE);
  }
  jlog_global_session_set(logger);

  if(strcmp(g_data.filename, ""))
  {
    if(jaycConf_loadConfig(g_data.filename, g_data.file_format) == false)
    {
      ERROR("jaycConf_loadConfig() failed.");
      jproc_exit(JAYCCONF_EXIT_FAILURE);
    }
  }

  g_data.cli = jutil_cli_init(&jaycConf_cliHandler, NULL);
  while(g_data.run)
  {
    jutil_cli_run(g_data.cli);
  }

  jproc_exit(JAYCCONF_EXIT_SUCCESS);
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
void jaycConf_initData()
{
  jproc_exit_setHandler(jaycConf_exitHandler, NULL);
  jproc_signal_setHandler(SIGINT, jaycConf_signalHandler, NULL);

  memset(g_data.filename, 0, sizeof(g_data.filename));

  g_data.config_data = jconfig_init();

  g_data.log_level = JLOG_LOGTYPE_INFO;
}

//------------------------------------------------------------------------------
//
void jaycConf_freeData()
{
  if(g_data.config_data)
  {
    jconfig_free(g_data.config_data);
  }
  if(g_data.cli)
  {
    jutil_cli_free(g_data.cli);
  }
}

//------------------------------------------------------------------------------
//
void jaycConf_exitHandler(int exit_value, void *ctx)
{
  jaycConf_freeData();
}

//------------------------------------------------------------------------------
//
void jaycConf_signalHandler(int signal_number, void *ctx)
{
  DEBUG("Signal [%d] caught.", signal_number);
  jproc_exit(JAYCCONF_EXIT_SIGINT);
}

//------------------------------------------------------------------------------
//
char *jaycConf_argFile(const char **data, size_t data_size)
{
  if(data_size != 2)
  {
    return jutil_args_error("[-f/--file] Invalid argument size [%lu].", data_size);
  }
  if(data == NULL)
  {
    return jutil_args_error("[-f/--file] Data array is NULL.");
  }
  if(data[0] == NULL || data[1] == NULL)
  {
    return jutil_args_error("[-f/--file] Argument string missing.");
  }
  if(strlen(data[0]) >= sizeof(g_data.filename))
  {
    return jutil_args_error("[-f/--file] Filename too long.");
  }

  memcpy(g_data.filename, data[0], strlen(data[0]));
  g_data.file_format = atoi(data[1]);

  return NULL;
}

//------------------------------------------------------------------------------
//
char *jaycConf_argDebug(const char **data, size_t data_size)
{
  if(data_size != 0)
  {
    return jutil_args_error("[--debug] Should have no arguments.");
  }

  g_data.log_level = JLOG_LOGTYPE_DEBUG;
  return NULL;
}

//------------------------------------------------------------------------------
//
int jaycConf_loadConfig(const char *file, int format)
{
  if(file == NULL)
  {
    return false;
  }

  switch(format)
  {
    case JAYCCONF_FORMAT_RAW:
    {
      return jconfig_raw_loadFromFile(g_data.config_data, file);
    }

    default:
    {
      return false;
    }
  }
}

//------------------------------------------------------------------------------
//
int jaycConf_saveConfig(const char *file, int format)
{
  if(file == NULL)
  {
    return false;
  }

  switch(format)
  {
    case JAYCCONF_FORMAT_RAW:
    {
      return jconfig_raw_saveToFile(g_data.config_data, file);
    }

    default:
    {
      return false;
    }
  }
}

//------------------------------------------------------------------------------
//
int jaycConf_dumpConfig(const char *prefix)
{
  jconfig_iterator_t *itr = NULL;

  while( (itr = jconfig_iterate(g_data.config_data, prefix, itr)) != NULL )
  {
    printf("\"%s\" = \"%s\"\n", jconfig_itr_getKey(itr), jconfig_itr_getData(itr));
  }
  printf("\n");
  return true;
}

//------------------------------------------------------------------------------
//
int jaycConf_cliHandler(const char **args, size_t arg_size, void *ctx)
{
  /*
   * Commands:
   * - lod <file> <format>  (Loads config from file)
   * - sav <file> <format>  (Saves config to file)
   * - set <key> <value>    (Sets value)
   * - get <key>            (Print key)
   * - del <key>            (Deletes key)
   * - dmp                  (Prints all config data)
   * - exit                 (Exit Program)
   * - help                 (Get information about commands)
   */

  if(arg_size == 0)
  {
    ERROR("arg_size is 0, something went wrong.");
    return 0;
  }

  if(strcmp(args[0], JAYCCONF_CMD_LOAD) == 0)
  {
    if(arg_size != 3)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    if(jaycConf_loadConfig(args[1], atoi(args[2])))
    {
      printf("OK\n\n");
      return 0;
    }

    printf("Could not load config.");
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_SAVE) == 0)
  {
    if(arg_size != 3)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    if(jaycConf_saveConfig(args[1], atoi(args[2])))
    {
      printf("OK\n\n");
      return 0;
    }

    printf("Could not load config.");
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_SET) == 0)
  {
    if(arg_size != 3)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    if(jconfig_datapoint_set(g_data.config_data, args[1], args[2]))
    {
      printf("OK\n\n");
      return 0;
    }

    printf("Could not set value.");
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_GET) == 0)
  {
    if(arg_size != 2)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    const char *value = jconfig_datapoint_get(g_data.config_data, args[1]);
    if(value == NULL)
    {
      INFO("Did not find key [%s].", args[1]);
      return 0;
    }

    printf("\"%s\" = \"%s\"\n\n", args[1], value);
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_DELETE) == 0)
  {
    if(arg_size != 2)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    if(jconfig_datapoint_delete(g_data.config_data, args[1]))
    {
      printf("OK\n\n");
      return 0;
    }

    printf("Could not delete key [%s].", args[1]);
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_DUMP) == 0)
  {
    if(arg_size > 2)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    const char *prefix = NULL;
    if(arg_size == 2)
    {
      prefix = args[1];
    }

    jaycConf_dumpConfig(prefix);
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_EXIT) == 0)
  {
    if(arg_size != 1)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    g_data.run = false;
    return 0;
  }
  else if(strcmp(args[0], JAYCCONF_CMD_HELP) == 0)
  {
    if(arg_size != 1)
    {
      INFO("Invalid number of arguments for command [%s].", args[0]);
      return 0;
    }

    jaycConf_cli_printHelp();
    return 0;
  }
  else
  {
    INFO("Invalid command [%s].", args[0]);
    return 0;
  }
}

//------------------------------------------------------------------------------
//
void jaycConf_cli_printHelp()
{
  /*
   * Commands:
   * - lod <file> <format>  (Loads config from file)
   * - sav <file> <format>  (Saves config to file)
   * - set <key> <value>    (Sets value)
   * - get <key>            (Print key)
   * - del <key>            (Deletes key)
   * - dmp                  (Prints all config data)
   * - exit                 (Exit Program)
   * - help                 (Get information about commands)
   */

  printf("## [CLI COMMANDS] ##\n\n");

  printf("# lod <file> <format>\n");
  printf("  Load configuration from file.\n");
  printf("  - file : File to read.\n");
  printf("  - format : File format to parse.\n");
  printf("\n");

  printf("# sav <file> <format>\n");
  printf("  Save configuration to file.\n");
  printf("  - file : File to write to.\n");
  printf("  - format : File format to parse.\n");
  printf("\n");

  printf("# set <key> <value>\n");
  printf("  Set key in config to value.\n");
  printf("  - key : Key of datapoint.\n");
  printf("  - value : Value to set datapoint to.\n");
  printf("\n");

  printf("# get <key>\n");
  printf("  Print key with value.\n");
  printf("  - key : Key of datapoint to print.\n");
  printf("\n");

  printf("# del <key>\n");
  printf("  Delete datapoint from configuration.\n");
  printf("  - key : Key of datapoint to delete.\n");
  printf("\n");

  printf("# dmp\n");
  printf("  Print whole configuration.\n");
  printf("\n");

  printf("# exit\n");
  printf("  Exit program.\n");
  printf("\n");

  printf("# help\n");
  printf("  Print this information.\n");
  printf("\n");
}
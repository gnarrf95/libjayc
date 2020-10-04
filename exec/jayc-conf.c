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
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
static int jaycConf_dumpConfig();

/**
 * @brief Processing CMD read from stdin.
 * 
 * @param cmd       CMD string.
 * @param cmd_size  CMD size.
 * 
 * @return          @c true , if successful.
 * @return          @c false , if error occured.
 */
static int jaycConf_processCMD(char *cmd);



//==============================================================================
// Define global data.
//

static jutil_args_progDesc_t prog_desc =
{
  "jayc-conf",

  "Program that can edit configurations and save/read " \
  "to/from files.",

  "v0.5-alpha",
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

  char *cmd_buf = NULL;
  size_t cmd_size = 0;
  while(g_data.run)
  {
    if(getline(&cmd_buf, &cmd_size, stdin) == -1)
    {
      ERROR("getline() failed.");
      free(cmd_buf);
      jproc_exit(JAYCCONF_EXIT_FAILURE);
    }

    char *cmd_str = (char *)malloc(sizeof(char) * cmd_size);
    if(cmd_str == NULL)
    {
      ERROR("malloc() failed.");
      free(cmd_buf);
      jproc_exit(JAYCCONF_EXIT_FAILURE);
    }

    memset(cmd_str, 0, cmd_size);
    if(sscanf(cmd_buf, "%[^\n]\n", cmd_str) != 1)
    {
      ERROR("sscanf() failed.");
      free(cmd_buf);
      free(cmd_str);
      jproc_exit(JAYCCONF_EXIT_FAILURE);
    }

    jaycConf_processCMD(cmd_str);

    // if(jaycConf_processCMD(cmd_str) == false)
    // {
    //   ERROR("jaycConf_processCMD() failed.");
    //   free(cmd_buf);
    //   free(cmd_str);
    //   jproc_exit(JAYCCONF_EXIT_FAILURE);
    // }

    free(cmd_buf);
    free(cmd_str);
    cmd_buf = NULL;
    cmd_size = 0;
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
  jlog_global_session_free();
  if(g_data.config_data)
  {
    jconfig_free(g_data.config_data);
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
int jaycConf_dumpConfig()
{
  jconfig_iterator_t *itr = NULL;

  while( (itr = jconfig_iterate(g_data.config_data, itr)) != NULL )
  {
    printf("\"%s\" = \"%s\"\n", jconfig_itr_getKey(itr), jconfig_itr_getData(itr));
  }
  printf("\n");
  return true;
}

//------------------------------------------------------------------------------
//
int jaycConf_processCMD(char *cmd)
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
   */

  char cmd_main[16] = { 0 };
  char cmd_par1[128] = { 0 };
  char cmd_par2[128] = { 0 };

  const char *delimiter = " ";
  int state = JAYCCONF_CMDSTATE_CMD;

  char *cmd_buf = strtok(cmd, delimiter);
  while(cmd_buf)
  {
    switch(state)
    {
      case JAYCCONF_CMDSTATE_CMD:
      {
        if(strlen(cmd_buf) >= sizeof(cmd_main))
        {
          INFO("Size of CMD name too long.");
          return false;
        }

        memcpy(cmd_main, cmd_buf, strlen(cmd_buf));
        break;
      }
      case JAYCCONF_CMDSTATE_PARAM1:
      {
        if(strlen(cmd_buf) >= sizeof(cmd_par1))
        {
          INFO("Size of parameter1 too long.");
          return false;
        }

        memcpy(cmd_par1, cmd_buf, strlen(cmd_buf));
        break;
      }
      case JAYCCONF_CMDSTATE_PARAM2:
      {
        if(strlen(cmd_buf) >= sizeof(cmd_par2))
        {
          INFO("Size of parameter2 too long.");
          return false;
        }

        memcpy(cmd_par2, cmd_buf, strlen(cmd_buf));
        break;
      }

      default:
      {
        ERROR("Invalid value for JAYCCONF_CMDSTATE [%d].", state);
        return false;
        // break;
      }
    }

    cmd_buf = strtok(NULL, delimiter);
    state++;
  }

  DEBUG("Got [%d] command values.", state);

  if(strlen(cmd_main) == 0)
  {
    INFO("No command name.");
    return false;
  }

  if(strcmp(cmd_main, JAYCCONF_CMD_LOAD) == 0)
  {
    if(strlen(cmd_par1) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }
    if(strlen(cmd_par2) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }

    return jaycConf_loadConfig(cmd_par1, atoi(cmd_par2));
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_SAVE) == 0)
  {
    if(strlen(cmd_par1) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }
    if(strlen(cmd_par2) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }

    return jaycConf_saveConfig(cmd_par1, atoi(cmd_par2));
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_SET) == 0)
  {
    if(strlen(cmd_par1) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }
    if(strlen(cmd_par2) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }

    return jconfig_datapoint_set(g_data.config_data, cmd_par1, cmd_par2);
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_GET) == 0)
  {
    if(strlen(cmd_par1) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }
    if(strlen(cmd_par2) != 0)
    {
      INFO("Too many arguments for command [%s].", cmd_main);
      return false;
    }

    const char *value = jconfig_datapoint_get(g_data.config_data, cmd_par1);
    if(value == NULL)
    {
      INFO("Did not find key [%s].", cmd_par1);
      return false;
    }

    printf("\"%s\" = \"%s\"\n\n", cmd_par1, value);
    return true;
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_DELETE) == 0)
  {
    if(strlen(cmd_par1) == 0)
    {
      INFO("Missing arguments for command [%s].", cmd_main);
      return false;
    }
    if(strlen(cmd_par2) != 0)
    {
      INFO("Too many arguments for command [%s].", cmd_main);
      return false;
    }

    return jconfig_datapoint_delete(g_data.config_data, cmd_par1);
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_DUMP) == 0)
  {
    return jaycConf_dumpConfig();
  }
  else if(strcmp(cmd_main, JAYCCONF_CMD_EXIT) == 0)
  {
    g_data.run = false;
    return true;
  }
  
  INFO("Invalid command [%s].", cmd_main);
  return false;
}
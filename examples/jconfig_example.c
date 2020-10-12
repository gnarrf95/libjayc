#include <jayc/jlog_stdio.h>
#include <jayc/jconfig.h>
#include <jayc/jproc.h>
#include <stddef.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define CONFIG_FILE_OLD "test_old.txt"
#define CONFIG_FILE_NEW "test_new.txt"

#define CONFIG_PREFIX "server.address"

static jconfig_t *g_config = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_config != NULL)
  {
    jconfig_free(g_config);
  }
}

int main()
{
  jproc_exit_setHandler(&exit_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  int ret_check;

  // Initialize config session.
  g_config = jconfig_init();
  if(g_config == NULL)
  {
    JLOG_ERROR("Could not initialize config.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Load config file.
  ret_check = jconfig_raw_loadFromFile(g_config, CONFIG_FILE_OLD);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not read config file.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Delete config point.
  ret_check = jconfig_datapoint_delete(g_config, "server.address.ip");
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not delete datapoint.");
  }

  // Change config point.
  ret_check = jconfig_datapoint_set(g_config, "server.address.port", "4444");
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not change config point.");
  }

  // Add config point.
  ret_check = jconfig_datapoint_set(g_config, "server.hashcode", "md5");
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not add config point.");
  }

  // Read config point.
  const char *cp = jconfig_datapoint_get(g_config, "server.address.port");
  if(cp == NULL)
  {
    JLOG_ERROR("Could not read datapoint.");
  }
  else
  {
    JLOG_INFO("[server.address.port] -> [%s].", cp);
  }

  // Print config keys that start with "server.address".
  jconfig_iterator_t *itr = jconfig_iterate(g_config, CONFIG_PREFIX, NULL);
  while(itr != NULL)
  {
    JLOG_INFO("[%s] : [%s].", jconfig_itr_getKey(itr), jconfig_itr_getData(itr));
    itr = jconfig_iterate(g_config, CONFIG_PREFIX, itr);
  }

  // Save config to file.
  ret_check = jconfig_raw_saveToFile(g_config, CONFIG_FILE_NEW);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not save config.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Clear config.
  jconfig_clear(g_config);

  // Free config memory.
  jconfig_free(g_config);
  g_config = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
#include <jayc/jutil_map.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static jutil_map_t *g_map = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_map)
  {
    jutil_map_free(g_map);
  }
}

int main()
{
  jproc_exit_setHandler(exit_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  /* Initialize map. */
  g_map = jutil_map_init();
  if(g_map == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  int value_1 = 45;
  int value_2 = 12;
  int value_3 = 0;

  /* Add node. */
  if(jutil_map_add(g_map, "value1", &value_1) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Add node. Set also creates if necessary. */
  if(jutil_map_set(g_map, "value2", &value_2) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Change value of node. */
  if(jutil_map_set(g_map, "value1", &value_3) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  int *value;

  /* Get value of node. */
  value = jutil_map_get(g_map, "value2");
  if(value == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("Value [%d].", *value);

  /* Remove node and print it. */
  value = jutil_map_remove(g_map, "value1");
  if(value == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("Value [%d].", *value);

  /* Check if map contains key. */
  if(jutil_map_contains(g_map, "value2") == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Iterate through map and print keys + values. */
  jutil_map_data_t *itr = jutil_map_iterate(g_map, NULL);

  while(itr != NULL)
  {
    JLOG_INFO("[%s] : [%d].", itr->index, *(int *)itr->data);
    
    itr = jutil_map_iterate(g_map, itr);
  }

  /* Print size of map. */
  JLOG_INFO("Size of map [%lu].", jutil_map_size(g_map));

  /* Free map. */
  jutil_map_free(g_map);
  g_map = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
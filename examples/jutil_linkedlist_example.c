#include <jayc/jutil_linkedlist.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static jutil_linkedlist_t *g_list = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_list)
  {
    jutil_linkedlist_free(&g_list);
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

  int value_1 = 45;
  int value_2 = 12;
  int value_3 = 0;
  int value_4 = -23;

  /* Add node at start. */
  if(jutil_linkedlist_push(&g_list, &value_1) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Add node at end. */
  if(jutil_linkedlist_append(&g_list, &value_2) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Add another node at beginning. */
  if(jutil_linkedlist_push(&g_list, &value_3) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Add another node at beginning. */
  if(jutil_linkedlist_push(&g_list, &value_4) == 0)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }

  jutil_linkedlist_t *itr;
  int *value;

  /* Iterate through list and print every node. */
  itr = g_list;

  while(itr != NULL)
  {
    value = jutil_linkedlist_getData(itr);
    if(value == NULL)
    {
      JLOG_INFO("Value [NULL].");
    }
    else
    {
      JLOG_INFO("Value [%d].", *value);
    }
    
    itr = jutil_linkedlist_iterate(itr);
  }

  /* Find node with value "0" and remove it. */
  itr = g_list;

  while(itr != NULL)
  {
    value = jutil_linkedlist_getData(itr);
    if(value)
    {
      if(*value == 0)
      {
        jutil_linkedlist_removeNode(&g_list, itr);
        break;
      }
    }
    
    itr = jutil_linkedlist_iterate(itr);
  }

  /* Remove node from end and print. */
  value = (int *)jutil_linkedlist_remove(&g_list);
  if(value == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("Value [%d].", *value);

  /* Remove node from start and print. */
  value = (int *)jutil_linkedlist_pop(&g_list);
  if(value == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("Value [%d].", *value);

  /* Get size of list. */
  JLOG_INFO("Size of list [%lu].", jutil_linkedlist_size(&g_list));

  /* Free list. Also sets it to NULL. */
  jutil_linkedlist_free(&g_list);
  jproc_exit(EXITVALUE_SUCCESS);
}
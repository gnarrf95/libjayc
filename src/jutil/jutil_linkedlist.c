/**
 * @file jutil_linkedlist.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jutil_linkedlist.
 * 
 * @date 2020-09-23
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jutil_linkedlist.h>
#include <stdlib.h>
#include <stdbool.h>

struct __jutil_linkedlist
{
  void *data;
  struct __jutil_linkedlist *next;
};

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_init()
{
  jutil_linkedlist_t *list = jutil_linkedlist_allocNode();
  
  return list;
}

//------------------------------------------------------------------------------
//
void jutil_linkedlist_free(jutil_linkedlist_t **list)
{
  if(list == NULL || *list == NULL)
  {
    return;
  }

  while(*list != NULL)
  {
    jutil_linkedlist_pop(list);
  }
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_allocNode()
{
  jutil_linkedlist_t *node = (jutil_linkedlist_t *)malloc(sizeof(jutil_linkedlist_t));
  if(node == NULL)
  {
    return NULL;
  }

  node->data = NULL;
  node->next = NULL;
  return node;
}

//------------------------------------------------------------------------------
//
void *jutil_linkedlist_getData(jutil_linkedlist_t *node)
{
  return node->data;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_iterate(jutil_linkedlist_t *head)
{
  if(head == NULL)
  {
    return NULL;
  }

  return head->next;
}

//------------------------------------------------------------------------------
//
size_t jutil_linkedlist_size(jutil_linkedlist_t **list)
{
  if(list == NULL || *list == NULL)
  {
    return 0;
  }

  jutil_linkedlist_t *itr = *list;
  
  size_t ret;
  for(ret = 0; itr != NULL; ret++)
  {
    itr = jutil_linkedlist_iterate(itr);
  }

  return ret;
}

//------------------------------------------------------------------------------
//
int jutil_linkedlist_push(jutil_linkedlist_t **list, void *data)
{
  if(list == NULL)
  {
    return false;
  }

  jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
  if(node == NULL)
  {
    return false;
  }
  node->data = data;

  if(*list)
  {
    node->next = *list;
  }

  *list = node;
  return true;
}

//------------------------------------------------------------------------------
//
void *jutil_linkedlist_pop(jutil_linkedlist_t **list)
{
  if(list == NULL || *list == NULL)
  {
    return NULL;
  }

  void *ret = (*list)->data;
  jutil_linkedlist_t *new_head = (*list)->next;

  free(*list);
  *list = new_head;

  return ret;
}

//------------------------------------------------------------------------------
//
void *jutil_linkedlist_removeNode(jutil_linkedlist_t **list, jutil_linkedlist_t *node)
{
  if(list == NULL || *list == NULL)
  {
    return NULL;
  }

  if(node == NULL)
  {
    return NULL;
  }

  void *ret = node->data;

  if(*list == node)
  {
    jutil_linkedlist_t *tmp = (*list)->next;
    free(*list);
    *list = tmp;
    
    return ret;
  }

  jutil_linkedlist_t *prev = NULL;
  jutil_linkedlist_t *itr = *list;
  while(itr != NULL)
  {
    if(itr == node)
    {
      prev->next = itr->next;
      free(itr);

      return ret;
    }

    prev = itr;
    itr = jutil_linkedlist_iterate(itr);
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
int jutil_linkedlist_append(jutil_linkedlist_t **list, void *data)
{
  if(list == NULL)
  {
    return false;
  }

  if(*list == NULL)
  {
    jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
    if(node == NULL)
    {
      return false;
    }

    node->data = data;
    *list = node;
    return true;
  }

  jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
  if(node == NULL)
  {
    return false;
  }
  node->data = data;

  jutil_linkedlist_t *itr = *list;
  while(itr->next != NULL)
  {
    itr = jutil_linkedlist_iterate(itr);
  }

  itr->next = node;
  return true;
}

//------------------------------------------------------------------------------
//
void *jutil_linkedlist_remove(jutil_linkedlist_t **list)
{
  if(list == NULL || *list == NULL)
  {
    return NULL;
  }

  jutil_linkedlist_t *itr = *list;
  while(itr->next != NULL)
  {
    itr = jutil_linkedlist_iterate(itr);
  }

  return jutil_linkedlist_removeNode(list, itr);
}
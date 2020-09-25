/**
 * @file jutil_linkedlist.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-23
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jutil_linkedlist.h>
#include <stdlib.h>

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
void jutil_linkedlist_free(jutil_linkedlist_t *list)
{
  if(list == NULL)
  {
    return;
  }

  while(list != NULL)
  {
    list = jutil_linkedlist_pop(list);
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
size_t jutil_linkedlist_size(jutil_linkedlist_t *list)
{
  if(list == NULL)
  {
    return 0;
  }

  jutil_linkedlist_t *itr = list;
  
  size_t ret;
  for(ret = 0; itr != NULL; ret++)
  {
    itr = jutil_linkedlist_iterate(itr);
  }

  return ret;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_push(jutil_linkedlist_t *list, void *data)
{
  if(list == NULL)
  {
    jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
    if(node == NULL)
    {
      return NULL;
    }

    node->data = data;
    return node;
  }

  jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
  if(node == NULL)
  {
    return NULL;
  }

  node->next = list;
  node->data = data;

  return node;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_pop(jutil_linkedlist_t *list)
{
  if(list == NULL)
  {
    return NULL;
  }

  jutil_linkedlist_t *new_head = list->next;
  free(list);

  return new_head;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_removeNode(jutil_linkedlist_t *list, jutil_linkedlist_t *node)
{
  if(list == NULL)
  {
    return NULL;
  }

  if(node == NULL)
  {
    return NULL;
  }

  jutil_linkedlist_t *iterator = list;
  jutil_linkedlist_t *previous = NULL;
  while(iterator != node)
  {
    previous = iterator;
    iterator = jutil_linkedlist_iterate(iterator);

    if(iterator == NULL)
    {
      return list;
    }
  }

  if(previous)
  {
    previous->next = iterator->next;
  }

  free(iterator);
  return list;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_append(jutil_linkedlist_t *list, void *data)
{
  if(list == NULL)
  {
    jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
    if(node == NULL)
    {
      return NULL;
    }

    node->data = data;
    return node;
  }

  jutil_linkedlist_t *node = jutil_linkedlist_allocNode();
  if(node == NULL)
  {
    return NULL;
  }
  node->data = data;

  jutil_linkedlist_t *iterator = list;
  while(iterator->next != NULL)
  {
    iterator = jutil_linkedlist_iterate(iterator);
  }

  iterator->next = node;
  return list;
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_linkedlist_remove(jutil_linkedlist_t *list)
{
  if(list == NULL)
  {
    return NULL;
  }

  jutil_linkedlist_t *iterator = list;
  while(iterator->next != NULL)
  {
    iterator = jutil_linkedlist_iterate(iterator);
  }

  return jutil_linkedlist_removeNode(list, iterator);
}
/**
 * @file jcon_linked_list.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements linked list, needed for jcon_server.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_linked_list.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
//
jcon_linked_list_node_t *jcon_linked_list_insert(jcon_linked_list_node_t *head, void *data)
{
  if(head == NULL)
  {
    head = (jcon_linked_list_node_t *)malloc(sizeof(jcon_linked_list_node_t));
    head->data = data;
    head->next = NULL;

    return head;
  }

  jcon_linked_list_node_t *iterator = head;

  while(iterator->next)
  {
    iterator = iterator->next;
  }

  jcon_linked_list_node_t *new_node = (jcon_linked_list_node_t *)malloc(sizeof(jcon_linked_list_node_t));
  new_node->data = data;
  new_node->next = NULL;

  iterator->next = new_node;

  return head;
}

//------------------------------------------------------------------------------
//
jcon_linked_list_node_t *jcon_linked_list_next(jcon_linked_list_node_t *head)
{
  if(head == NULL)
  {
    return NULL;
  }

  return head->next;
}

//------------------------------------------------------------------------------
//
jcon_linked_list_node_t *jcon_linked_list_delete(jcon_linked_list_node_t *head, jcon_linked_list_node_t *to_rem)
{
  if(head == NULL)
  {
    return NULL;
  }

  jcon_linked_list_node_t *iterator = head;
  jcon_linked_list_node_t *previous = NULL;

  while(iterator != to_rem)
  {
    if(iterator->next == NULL)
    {
      return NULL;
    }

    previous = iterator;
    iterator = iterator->next;
  }

  if(iterator == head)
  {
    head = head->next;
  }
  else
  {
    previous->next = iterator->next;
  }

  free(iterator);
  return previous->next;
}

//------------------------------------------------------------------------------
//
size_t jcon_linked_list_size(jcon_linked_list_node_t *head)
{
  jcon_linked_list_node_t *iterator;
  size_t ctr = 0;

  for(iterator = head; iterator != NULL; iterator = iterator->next)
  {
    ctr++;
  }

  return ctr;
}
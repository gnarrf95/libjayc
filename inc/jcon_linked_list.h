/**
 * @file jcon_linked_list.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Linked list, developed for jcon_server.
 * 
 * This linked list has reduced functionality,
 * because it only needs _insert_ and _delete_.
 * 
 * @todo Implement fully functional linked list.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_LINKED_LIST_H
#define INCLUDE_JCON_LINKED_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __jcon_linked_list_node
{
  void *data;
  struct node *next;
} jcon_linked_list_node_t;

/**
 * @brief Inserts node into a linked list.
 * 
 * @param head  List head, to add element to.
 *              If @c NULL , creates list head and returns it.
 * @param data  Data to store in element.
 * 
 * @return      Head of list.
 * @return      @c NULL , if error occured.
 */
jcon_linked_list_node_t *jcon_linked_list_insert(jcon_linked_list_node_t *head, void *data);

/**
 * @brief Removes node from linked list.
 * 
 * This function does not free any allocated data saved in node.
 * 
 * @param head    Head of list.
 * @param to_rem  Node to remove from list.
 */
void jcon_linked_list_delete(jcon_linked_list_node_t *head, jcon_linked_list_node_t *to_rem);

/**
 * @brief Returns the number of nodes in the list.
 * 
 * @param head  Head of list.
 * 
 * @return      Number of nodes in list.
 */
size_t jcon_linked_list_size(jcon_linked_list_node_t *head);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_LINKED_LIST_H */
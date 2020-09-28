/**
 * @file jutil_linkedlist.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Linked list, for storing unsorted elements.
 * 
 * This linked list is made for simple data storing.
 * There is no sorting, or indexing. The only way
 * to find elements, is to iterate through the list
 * and analyse each node.
 * 
 * @date 2020-09-23
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_LINKEDLIST_H
#define INCLUDE_JUTIL_LINKEDLIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Node object.
 * 
 * Holds data and pointer to next node.y
 * 
 * Used as double pointer, so list can
 * be passed as argument and changed.
 */
typedef struct __jutil_linkedlist jutil_linkedlist_t;

/**
 * @brief Initializes list. Returns empty node.
 * 
 * @todo Is this function even useful???
 * 
 * @return  Empty node.
 * @return  @c NULL , if error occured.
 */
jutil_linkedlist_t *jutil_linkedlist_init();

/**
 * @brief Frees all nodes from list.
 * 
 * @param list Head of linked list.
 */
void jutil_linkedlist_free(jutil_linkedlist_t **list);

/**
 * @brief Create a node.
 * 
 * Allocates memory for node, sets @c jutil_linkedlist_t#data
 * and @c jutil_linkedlist_t#next to @c NULL .
 * 
 * @return  Pointer to new node.
 * @return  @c NULL in case of error.
 */
jutil_linkedlist_t *jutil_linkedlist_allocNode();

/**
 * @brief Returns data saved in node.
 * 
 * @param node  Node, to request data from.
 * 
 * @return      Data pointer.
 * @return      @c NULL in case of error.
 */
void *jutil_linkedlist_getData(jutil_linkedlist_t *node);

/**
 * @brief Returns next node in list.
 * 
 * @param head  Pointer to current node.
 * 
 * @return      Pointer to next node in list.
 * @return      @c NULL , if head was last node or error occured.
 */
jutil_linkedlist_t *jutil_linkedlist_iterate(jutil_linkedlist_t *head);

/**
 * @brief Counts number of nodes in list.
 * 
 * @param list  List to count.
 * 
 * @return      Number of nodes in list.
 */
size_t jutil_linkedlist_size(jutil_linkedlist_t **list);

/**
 * @brief Put new node at first position of list.
 * 
 * @param list  List to edit.
 * @param data  Data pointer for new node.
 * 
 * @return      @c true , if successful.
 * @return      @c false in case of error.
 */
int jutil_linkedlist_push(jutil_linkedlist_t **list, void *data);

/**
 * @brief Remove first node of list.
 * 
 * @param list  List to edit.
 * 
 * @return      Data stored in first node.
 * @return      @c NULL , if list is empty or error occured.
 */
void *jutil_linkedlist_pop(jutil_linkedlist_t **list);

/**
 * @brief Iterates through list, until node is found and removes it.
 * 
 * If node is not found, returns original list.
 * 
 * @param list  List to edit.
 * @param node  Node to remove.
 * 
 * @return      Data of removed node.
 * @return      @c NULL , if node not found or error occured.
 */
void *jutil_linkedlist_removeNode(jutil_linkedlist_t **list, jutil_linkedlist_t *node);

/**
 * @brief Add node at end of list.
 * 
 * @param list  List to edit.
 * @param data  Data for new node.
 * 
 * @return      @c true , if successful.
 * @return      @c false in case of error.
 */
int jutil_linkedlist_append(jutil_linkedlist_t **list, void *data);

/**
 * @brief Remove last node of list.
 * 
 * @param list  List to edit.
 * 
 * @return      Data of last node.
 * @return      @c NULL , if list is empty or error occured.
 */
void *jutil_linkedlist_remove(jutil_linkedlist_t **list);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_LINKEDLIST_H */
/**
 * @file jutil_linkedlist.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-23
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_LINKEDLIST_H
#define INCLUDE_JUTIL_LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __jutil_linkedlist jutil_linkedlist_t;

jutil_linkedlist_t *jutil_linkedlist_init();
void jutil_linkedlist_free(jutil_linkedlist_t *list);
jutil_linkedlist_t *jutil_linkedlist_allocNode();

void *jutil_linkedlist_getData(jutil_linkedlist_t *node);

jutil_linkedlist_t *jutil_linkedlist_iterate(jutil_linkedlist_t *head);

jutil_linkedlist_t *jutil_linkedlist_push(jutil_linkedlist_t *list, void *data);
jutil_linkedlist_t *jutil_linkedlist_pop(jutil_linkedlist_t *list);

jutil_linkedlist_t *jutil_linkedlist_removeNode(jutil_linkedlist_t *list, jutil_linkedlist_t *node);

jutil_linkedlist_t *jutil_linkedlist_append(jutil_linkedlist_t *list, void *data);
jutil_linkedlist_t *jutil_linkedlist_remove(jutil_linkedlist_t *list);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_LINKEDLIST_H */
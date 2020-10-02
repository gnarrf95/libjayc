/**
 * @file jutil_map.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Simple string indexed map.
 * 
 * This is not a very efficient way to implement
 * a map, but it was easy to implement and is fairly
 * easy to use.
 * 
 * @date 2020-10-02
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_MAP_H
#define INCLUDE_JUTIL_MAP_H

#include <jayc/jutil_linkedlist.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JUTIL_MAP_SIZE_INDEX 128

typedef struct __jutil_map
{
  jutil_linkedlist_t *list;
} jutil_map_t;

typedef struct __jutil_map_data
{
  char index[JUTIL_MAP_SIZE_INDEX];
  void *data;
} jutil_map_data_t;

jutil_map_t *jutil_map_init();
void jutil_map_free(jutil_map_t *map);

int jutil_map_add(jutil_map_t *map, const char *index, void *data);
void *jutil_map_remove(jutil_map_t *map, const char *index);
int jutil_map_contains(jutil_map_t *map, const char *index);

void *jutil_map_get(jutil_map_t *map, const char *index);
int jutil_map_set(jutil_map_t *map, const char *index, void *data);

size_t jutil_map_size(jutil_map_t *map);
void jutil_map_clear(jutil_map_t *map);

jutil_linkedlist_t *jutil_map_getNode(jutil_map_t *map, const char *index);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_MAP_H */
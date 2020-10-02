/**
 * @file jutil_map.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements jutil_map.
 * 
 * @date 2020-10-02
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jutil_map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
//
jutil_map_t *jutil_map_init()
{
  jutil_map_t *map = (jutil_map_t *)malloc(sizeof(jutil_map_t));
  if(map == NULL)
  {
    return NULL;
  }

  map->list = NULL;

  return map;
}

//------------------------------------------------------------------------------
//
void jutil_map_free(jutil_map_t *map)
{
  if(map == NULL)
  {
    return;
  }

  jutil_map_clear(map);
  free(map);
}

//------------------------------------------------------------------------------
//
int jutil_map_add(jutil_map_t *map, const char *index, void *data)
{
  if(map == NULL)
  {
    return false;
  }
  if(index == NULL)
  {
    return false;
  }
  if(strlen(index) == 0)
  {
    return false;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return false;
  }
  if(jutil_map_contains(map, index))
  {
    return false;
  }

  jutil_map_data_t *map_data = (jutil_map_data_t *)malloc(sizeof(jutil_map_data_t));
  if(map_data == NULL)
  {
    return false;
  }

  memset(map_data->index, 0, sizeof(map_data->index));
  memcpy(map_data->index, index, strlen(index));
  map_data->data = data;

  if(jutil_linkedlist_push(&map->list, (void *)map_data) == false)
  {
    free(map_data);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void *jutil_map_remove(jutil_map_t *map, const char *index)
{
  if(map == NULL)
  {
    return NULL;
  }
  if(index == NULL)
  {
    return NULL;
  }
  if(strlen(index) == 0)
  {
    return NULL;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return NULL;
  }

  jutil_linkedlist_t *node = jutil_map_getNode(map, index);
  if(node == NULL)
  {
    return NULL;
  }

  jutil_map_data_t *map_data = jutil_linkedlist_removeNode(&map->list, node);
  if(map_data == NULL)
  {
    return NULL;
  }

  void *data = map_data->data;
  free(map_data);

  return data;
}

//------------------------------------------------------------------------------
//
int jutil_map_contains(jutil_map_t *map, const char *index)
{
  if(map == NULL)
  {
    return false;
  }
  if(index == NULL)
  {
    return false;
  }
  if(strlen(index) == 0)
  {
    return false;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return false;
  }

  if(jutil_map_getNode(map, index))
  {
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
//
void *jutil_map_get(jutil_map_t *map, const char *index)
{
  if(map == NULL)
  {
    return NULL;
  }
  if(index == NULL)
  {
    return NULL;
  }
  if(strlen(index) == 0)
  {
    return NULL;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return NULL;
  }

  jutil_linkedlist_t *node = jutil_map_getNode(map, index);
  if(node == NULL)
  {
    return NULL;
  }

  jutil_map_data_t *map_data = jutil_linkedlist_getData(node);
  if(map_data == NULL)
  {
    return NULL;
  }

  return map_data->data;
}

//------------------------------------------------------------------------------
//
int jutil_map_set(jutil_map_t *map, const char *index, void *data)
{
  if(map == NULL)
  {
    return false;
  }
  if(index == NULL)
  {
    return false;
  }
  if(strlen(index) == 0)
  {
    return false;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return false;
  }

  jutil_linkedlist_t *node = jutil_map_getNode(map, index);
  if(node == NULL)
  {
    return false;
  }

  jutil_map_data_t *map_data = jutil_linkedlist_getData(node);
  if(map_data == NULL)
  {
    return false;
  }

  map_data->data = data;
  return true;
}

//------------------------------------------------------------------------------
//
size_t jutil_map_size(jutil_map_t *map)
{
  if(map == NULL)
  {
    return 0;
  }

  if(map->list == NULL)
  {
    return 0;
  }

  return jutil_linkedlist_size(&map->list);
}

//------------------------------------------------------------------------------
//
void jutil_map_clear(jutil_map_t *map)
{
  if(map == NULL)
  {
    return;
  }

  while(map->list != NULL)
  {
    jutil_map_data_t *map_data = jutil_linkedlist_pop(&map->list);
    free(map_data);
  }
}

//------------------------------------------------------------------------------
//
jutil_linkedlist_t *jutil_map_getNode(jutil_map_t *map, const char *index)
{
  if(map == NULL)
  {
    return NULL;
  }
  if(index == NULL)
  {
    return NULL;
  }
  if(strlen(index) == 0)
  {
    return NULL;
  }
  if(strlen(index) >= JUTIL_MAP_SIZE_INDEX)
  {
    return NULL;
  }

  jutil_linkedlist_t *itr = map->list;

  while(itr != NULL)
  {
    jutil_map_data_t *map_data = jutil_linkedlist_getData(itr);
    if(map_data)
    {
      if(strcmp(map_data->index, index) == 0)
      {
        return itr;
      }
    }

    itr = jutil_linkedlist_iterate(itr);
  }

  return NULL;
}
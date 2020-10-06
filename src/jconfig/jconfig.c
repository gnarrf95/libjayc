/**
 * @file jconfig.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements jconfig component.
 * 
 * @date 2020-10-02
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

/* Needed for getline() */
#define _POSIX_C_SOURCE 200809L

#include <jayc/jconfig.h>
#include <jayc/jconfig_dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//------------------------------------------------------------------------------
//
jconfig_t *jconfig_init()
{
  jconfig_t *table = (jconfig_t *)malloc(sizeof(jconfig_t));
  if(table == NULL)
  {
    return NULL;
  }

  table->map = jutil_map_init();
  if(table->map == NULL)
  {
    free(table);
    return NULL;
  }

  return table;
}

//------------------------------------------------------------------------------
//
void jconfig_free(jconfig_t *table)
{
  if(table == NULL)
  {
    return;
  }

  jconfig_clear(table);
  jutil_map_free(table->map);
  free(table);
}

//------------------------------------------------------------------------------
//
int jconfig_datapoint_delete(jconfig_t *table, const char *key)
{
  if(table == NULL)
  {
    return false;
  }
  if(key == NULL)
  {
    return false;
  }
  if(strlen(key) == 0)
  {
    return false;
  }
  if(strlen(key) >= JUTIL_MAP_SIZE_INDEX)
  {
    return false;
  }

  void *data = jutil_map_remove(table->map, key);

  if(data == NULL)
  {
    return false;
  }

  free(data);
  return true;
}

//------------------------------------------------------------------------------
//
const char *jconfig_datapoint_get(jconfig_t *table, const char *key)
{
  if(table == NULL)
  {
    return NULL;
  }
  if(key == NULL)
  {
    return NULL;
  }
  if(strlen(key) == 0)
  {
    return NULL;
  }
  if(strlen(key) >= JUTIL_MAP_SIZE_INDEX)
  {
    return NULL;
  }

  return (const char *)jutil_map_get(table->map, key);
}

//------------------------------------------------------------------------------
//
int jconfig_datapoint_set(jconfig_t *table, const char *key, const char *value)
{
  if(table == NULL)
  {
    return false;
  }
  if(key == NULL)
  {
    return false;
  }
  if(strlen(key) == 0)
  {
    return false;
  }
  if(strlen(key) >= JUTIL_MAP_SIZE_INDEX)
  {
    return false;
  }
  if(value == NULL)
  {
    return false;
  }

  size_t data_size = strlen(value) + 1;
  char *data = (char *)malloc(sizeof(char) * data_size);
  if(data == NULL)
  {
    return false;
  }

  char *old_data = (char *)jutil_map_get(table->map, key);
  if(old_data)
  {
    free(old_data);
  }

  memset(data, 0, data_size);
  memcpy(data, value, data_size);

  if(jutil_map_set(table->map, key, (void *)data) == false)
  {
    free(data);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void jconfig_clear(jconfig_t *table)
{
  if(table == NULL)
  {
    return;
  }

  jutil_map_data_t *itr = NULL;

  while( (itr = jutil_map_iterate(table->map, itr)) != NULL )
  {
    free(itr->data);
  }

  jutil_map_clear(table->map);
}

//------------------------------------------------------------------------------
//
jconfig_iterator_t *jconfig_iterate(jconfig_t *table, jconfig_iterator_t *itr)
{
  if(table == NULL)
  {
    return NULL;
  }

  return (jconfig_iterator_t *)jutil_map_iterate(table->map, (jutil_map_data_t *)itr);
}

//------------------------------------------------------------------------------
//
const char *jconfig_itr_getKey(jconfig_iterator_t *itr)
{
  if(itr == NULL)
  {
    return NULL;
  }

  jutil_map_data_t *map_itr = itr;
  return (const char *)map_itr->index;
}

//------------------------------------------------------------------------------
//
const char *jconfig_itr_getData(jconfig_iterator_t *itr)
{
  if(itr == NULL)
  {
    return NULL;
  }

  jutil_map_data_t *map_itr = itr;
  return (const char *)map_itr->data;
}

//------------------------------------------------------------------------------
//
int jconfig_raw_saveToFile(jconfig_t *table, const char *filename)
{
  if(table == NULL)
  {
    return false;
  }

  FILE *fd = fopen(filename, "w");
  if(fd == NULL)
  {
    return false;
  }

  jutil_map_data_t *map_itr = NULL;

  while( (map_itr = jutil_map_iterate(table->map, map_itr)) )
  {
    if(map_itr->index == NULL)
    {
      fclose(fd);
      return false;
    }

    if(map_itr->data == NULL)
    {
      fclose(fd);
      return false;
    }

    fprintf(fd, "%s=%s\n", map_itr->index, (const char *)map_itr->data);
  }

  fclose(fd);
  return true;
}

//------------------------------------------------------------------------------
//
int jconfig_raw_loadFromFile(jconfig_t *table, const char *filename)
{
  if(table == NULL)
  {
    return false;
  }

  FILE *fd = fopen(filename, "r");
  if(fd == NULL)
  {
    return false;
  }

  jconfig_clear(table);

  char *lineptr = NULL;
  size_t line_size = 0;
  while(getline(&lineptr, &line_size, fd) > 0)
  {
    char key[2048] = { 0 };
    char data[2048] = { 0 };

    /* Empty lines are ignored. */
    if(lineptr[0] == '\n')
    {
      free(lineptr);
      continue;
    }

    if(sscanf(lineptr, "%2047[^=]=%2047[^\n]\n", key, data) != 2)
    {
      if(strlen(key) == 0)
      {
        free(lineptr);
        continue;
      }
    }

    free(lineptr);
    lineptr = NULL;
    line_size = 0;

    if(jconfig_datapoint_set(table, key, data) == false)
    {
      free(lineptr);
      fclose(fd);
      return false;
    }
  }

  free(lineptr);

  fclose(fd);
  return true;
}
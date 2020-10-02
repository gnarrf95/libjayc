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

/**
 * @brief Object pointer.
 */
typedef struct __jutil_map jutil_map_t;

/**
 * @brief Initializes map object.
 * 
 * @return  Map object pointer.
 * @return  @c NULL , if error occured.
 */
jutil_map_t *jutil_map_init();

/**
 * @brief Clears map and frees memory.
 * 
 * @param map Map object to clear.
 */
void jutil_map_free(jutil_map_t *map);

/**
 * @brief Add data to map index by @c index .
 * 
 * @param map   Map object.
 * @param index Index for new data.
 * @param data  Data to store.
 * 
 * @return      @c true , if successfully stored.
 * @return      @c false , if error occured.
 */
int jutil_map_add(jutil_map_t *map, const char *index, void *data);

/**
 * @brief Removes data from map.
 * 
 * @param map   Map object.
 * @param index Index of data to remove.
 * 
 * @return      Data pointer.
 * @return      @c NULL , if error occured.
 */
void *jutil_map_remove(jutil_map_t *map, const char *index);

/**
 * @brief Checks if index is in map.
 * 
 * @param map   Map object to check.
 * @param index Index to check.
 * 
 * @return      @c true , if index is in map.
 * @return      @c false , if index is not in map or error occured.
 */
int jutil_map_contains(jutil_map_t *map, const char *index);

/**
 * @brief Get data with index.
 * 
 * @param map   Map to search.
 * @param index Index to search for.
 * 
 * @return      Data stored with @c index.
 * @return      @c NULL , if @c index is not in map
 *              or error occured.
 */
void *jutil_map_get(jutil_map_t *map, const char *index);

/**
 * @brief Change data stored with index.
 * 
 * If index is not contained in map,
 * new entry is created.
 * 
 * @param map   Map object.
 * @param index Index for data.
 * @param data  New data to store.
 * 
 * @return      @c true , if data was stored.
 * @return      @c false , if error occured.
 */
int jutil_map_set(jutil_map_t *map, const char *index, void *data);

/**
 * @brief Check number of items in map.
 * 
 * @param map Map to check.
 * 
 * @return    Number of elements.
 * @return    @c 0 , if map empty or error occured.
 */
size_t jutil_map_size(jutil_map_t *map);

/**
 * @brief Remove all items from map.
 * 
 * <b>Note:</b>
 * Does not free memory of data.
 * 
 * @param map Map object to clear.
 */
void jutil_map_clear(jutil_map_t *map);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_MAP_H */
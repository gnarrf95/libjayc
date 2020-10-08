/**
 * @file jconfig.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief This is a concept for the jconfig interface.
 * 
 * <b>Note:</b>
 * Data in config is allocated and copied.
 * Data is freed when removed.
 * Do not free data manually!
 * 
 * @date 2020-09-23
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCONFIG_H
#define INCLUDE_JCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Config object.
 */
typedef struct __jconfig_table jconfig_t;

/**
 * @brief Reference to iterate through config data.
 */
typedef void jconfig_iterator_t;

/**
 * @brief Initializes config object.
 * 
 * @return  Config object pointer.
 * @return  @c NULL , if error occured.
 */
jconfig_t *jconfig_init();

/**
 * @brief Clears config table and frees memory.
 * 
 * @param table Config table to clear.
 */
void jconfig_free(jconfig_t *table);

/**
 * @brief Removes config point from table.
 * 
 * @param table Table to remove from.
 * @param key   Key to remove.
 * 
 * @return      @c true , if key was removed.
 * @return      @c false , if key was not found or error occured.
 */
int jconfig_datapoint_delete(jconfig_t *table, const char *key);

/**
 * @brief Returns data stored in key.
 * 
 * @param table Table to get data from.
 * @param key   Key to search for.
 * 
 * @return      Data string.
 * @return      @c NULL , if key not found or error occured.
 */
const char *jconfig_datapoint_get(jconfig_t *table, const char *key);

/**
 * @brief Sets data at key in config table.
 * 
 * If key is not found, datapoint is created.
 * 
 * @param table Config table object.
 * @param key   Key to set.
 * @param value Value to set key to.
 * 
 * @return      @c true , if key was set/created.
 * @return      @c false , if error occured.
 */
int jconfig_datapoint_set(jconfig_t *table, const char *key, const char *value);

/**
 * @brief Clears content from config.
 * 
 * @param table Table clear.
 */
void jconfig_clear(jconfig_t *table);

/**
 * @brief Iterates through all available datapoints.
 * 
 * @param table   Config table to iterate.
 * @param prefix  Only search for keys that start with prefix.
 *                (Used for hierachrcal keys)
 *                Ignored if @c NULL or @c "" .
 * @param itr     Iterator reference.
 *                If @c NULL , starts over.
 * 
 * @return        Next available element.
 * @return        @c NULL , if no more data.
 */
jconfig_iterator_t *jconfig_iterate(jconfig_t *table, const char *prefix, jconfig_iterator_t *itr);

/**
 * @brief Returns key at datapoint referenced by iterator.
 * 
 * @param itr Iterator reference.
 * 
 * @return    Key string.
 * @return    @c NULL , if error occured.
 */
const char *jconfig_itr_getKey(jconfig_iterator_t *itr);

/**
 * @brief Returns data at datapoint referenced by iterator.
 * 
 * @param itr Iterator reference.
 * 
 * @return    Data string.
 * @return    @c NULL , if error occured.
 */
const char *jconfig_itr_getData(jconfig_iterator_t *itr);

/**
 * @brief Saves config as raw key-value pair.
 * 
 * Saves config points as newline seperated
 * key-value pair in following format:
 * "<key-string>=<value-string>\n"
 * 
 * For example:
 * "server.address.ip=127.0.0.1\n"
 * "server.address.port=1234\n"
 * 
 * @param table     Config table to save.
 * @param filename  Path of file to save.
 * 
 * @return          @c true , if saved successfully.
 * @return          @c false , if error occured.
 */
int jconfig_raw_saveToFile(jconfig_t *table, const char *filename);

/**
 * @brief Loads config as raw key-value pair.
 * 
 * Loads config points as newline seperated
 * key-value pair in following format:
 * "<key-string>=<value-string>\n"
 * 
 * For example:
 * "server.address.ip=127.0.0.1\n"
 * "server.address.port=1234\n"
 * 
 * @param table     Config table to load.
 * @param filename  Path of file to load from.
 * 
 * @return          @c true , if loaded successfully.
 * @return          @c false , if error occured.
 */
int jconfig_raw_loadFromFile(jconfig_t *table, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCONFIG_H */
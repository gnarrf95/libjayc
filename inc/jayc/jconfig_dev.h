/**
 * @file jconfig_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Provides necessary definitions for additional functionality.
 * 
 * Should be used, to implement different types of
 * file formats to be parsed into and from jconfig data
 * structures.
 * 
 * @date 2020-10-03
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCONFIG_DEV_H
#define INCLUDE_JCONFIG_DEV_H

#include <jayc/jutil_map.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Config object.
 */
struct __jconfig_table
{
  /* Table: Should be a map, that can access datapoints with an string index.
     Index keys should be stored in the format "server.address.ip", to allow
     nested data. This should make it compatible with most config file
     formats. */
  
  jutil_map_t *map; /**< Config data table. */
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCONFIG_DEV_H */
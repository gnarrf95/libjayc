/**
 * @file jconfig.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief This is a concept for the jconfig interface.
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

typedef struct __jconfig_table jconfig_t;

struct __jconfig_table
{
  /* Table: Should be a map, that can access datapoints with an string index.
     Index keys should be stored in the format "server.address.ip", to allow
     nested data. This should make it compatible with most config file
     formats. */
  void *table;
  char *filename;
};

jconfig_t *jconfig_init(const char *filename);

void jconfig_free(jconfig_t *table);

const char *jconfig_datapoint_get(jconfig_t *table, const char *key);
int jconfig_datapoint_set(jconfig_t *table, const char *key, const char *value);
int jconfig_datapoint_delete(jconfig_t *table, const char *key);
int jconfig_datapoint_add(jconfig_t *table, const char *key, const char *value);

const char *jconfig_file_getName(jconfig_t *table);
int jconfig_file_setName(jconfig_t *table, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCONFIG_H */
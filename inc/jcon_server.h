/**
 * @file jcon_server.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief This is a interface for a server system.
 * 
 * @todo add details ...
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SERVER_H
#define INCLUDE_JCON_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef struct __jcon_server_session jcon_server_t;

void jcon_server_free(jcon_server_t *session);
const char *jcon_server_getConnectionType(jcon_server_t *session);
const char *jcon_server_getReferenceString(jcon_server_t *session);

int jcon_server_reset(jcon_server_t *session);
void jcon_server_close(jcon_server_t *session);

size_t jcon_server_getConnectionNumber(jcon_server_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_H */
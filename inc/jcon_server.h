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

#include <jcon_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief jcon_server session object, holds data and functions for operation.
 */
typedef struct __jcon_server_session jcon_server_t;

/**
 * @brief Frees session memory.
 * 
 * @param session Session object to destroy.
 */
void jcon_server_free(jcon_server_t *session);

/**
 * @brief Closes and restarts server.
 * 
 * @param session Session to reset.
 * 
 * @return        @c true , if reset was successful.
 * @return        @c false , if reset failed or error occured.
 */
int jcon_server_reset(jcon_server_t *session);

/**
 * @brief Closes server.
 * 
 * @param session Session to close.
 */
void jcon_server_close(jcon_server_t *session);

/**
 * @brief Checks, if server is open for connections.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if server is open.
 * @return        @c false , if server is down or error occured.
 */
int jcon_server_isOpen(jcon_server_t *session);

/**
 * @brief Get type of connection the server uses.
 * 
 * @param session Session to check.
 * 
 * @return        String representing type of connection.
 * @return        @c NULL in case of error.
 */
const char *jcon_server_getConnectionType(jcon_server_t *session);

/**
 * @brief Get string, that shows information about server connection.
 * 
 * @param session Session to check.
 * 
 * @return        String with server connection info.
 * @return        @c NULL in case of error.
 */
const char *jcon_server_getReferenceString(jcon_server_t *session);

/**
 * @brief Check, if there is a new client attempting to connect.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if new connection is available.
 * @return        @c false , if no connections or error occured.
 */
int jcon_server_newConnection(jcon_server_t *session);

/**
 * @brief Accept new connection and create jcon_client session for connection.
 * 
 * Before accepting a connection, it should be checked if a new
 * connection is available (using @c #jcon_server_newConnection() ).
 * But this depends on implemention of the server.
 * 
 * @param session Session, that accepts connection.
 * 
 * @return        jcon_client session, that is connected to new client.
 * @return        @c NULL in case of error.
 */
jcon_client_t *jcon_server_acceptConnection(jcon_server_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_H */
/**
 * @file jcon_system.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief System for threaded server.
 * 
 * @todo add details ...
 * 
 * @date 2020-09-24
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SYSTEM_H
#define INCLUDE_JCON_SYSTEM_H

#include <jayc/jcon_server.h>
#include <jayc/jcon_client.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Session object. Holds data for operation.
 */
typedef struct __jcon_system_session jcon_system_t;

/**
 * @brief Function that handles available data.
 * 
 * @param ctx     Context pointer provided by user.
 * @param client  jcon_client session, that has data available
 */
typedef void(*jcon_system_threadData_handler_t)(void *ctx, jcon_client_t *client);

/**
 * @brief Function that handles created connections.
 * 
 * @param ctx         Context pointer provided by user.
 * @param ref_string  Reference string of created client.
 */
typedef void(*jcon_system_threadCreate_handler_t)(void *ctx, const char *ref_string);

/**
 * @brief Function that handles closed connections.
 * 
 * @param ctx         Context pointer provided by user.
 * @param ref_string  Reference string of closed client.
 */
typedef void(*jcon_system_threadClose_handler_t)(void *ctx, const char *ref_string);

/**
 * @brief Initializes system and starts control thread.
 * 
 * @param server          jcon_server session to use.
 * @param data_handler    Handler to manage new data.
 * @param create_handler  Handler gets called, when connection is created.
 * @param close_handler   Handler gets called, when connection is closed.
 * @param logger          Logger to print debug and error messages.
 * @param ctx             Context pointer passed to handlers.
 * 
 * @return                jcon_system session object.
 * @return                @c NULL , if error occured.
 */
jcon_system_t *jcon_system_init
(
  jcon_server_t *server,
  jcon_system_threadData_handler_t data_handler,
  jcon_system_threadCreate_handler_t create_handler,
  jcon_system_threadClose_handler_t close_handler,
  jlog_t *logger,
  void *ctx
);

/**
 * @brief Stops everything and frees memory.
 * 
 * Closes all connections, stops control thread
 * and frees session memory.
 * 
 * Server not freed, has to be handled manually.
 * 
 * @param session jcon_system session to free.
 */
void jcon_system_free(jcon_system_t *session);

/**
 * @brief Get type of server connection.
 * 
 * @param session Session to check.
 * 
 * @return        String with connection type.
 * @return        @c NULL , if error occured.
 */
const char *jcon_system_getConnectionType(jcon_system_t *session);

/**
 * @brief Get reference string of server.
 * 
 * @param session Session to check.
 * 
 * @return        Reference string of server.
 * @return        @c NULL , if error occured.
 */
const char *jcon_system_getReferenceString(jcon_system_t *session);

/**
 * @brief Check if server is open.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if server is open.
 * @return        @c false , if server is closed
 *                or error occured.
 */
int jcon_system_isServerOpen(jcon_system_t *session);

/**
 * @brief Check if control thread is running.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if control thread is running.
 * @return        @c false , if control thread is stopped
 *                or error occured.
 */
int jcon_system_control_isRunning(jcon_system_t *session);

/**
 * @brief Get number of connections to server
 * 
 * @param session Session to check.
 * 
 * @return        Number of connections.
 * @return        @c 0 , if no connections
 *                or error occured.
 */
size_t jcon_system_getConnectionNumber(jcon_system_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SYSTEM_H */
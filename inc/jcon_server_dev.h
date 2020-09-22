/**
 * @file jcon_server_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definition for implementations of jcon_server.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SERVER_DEV_H
#define INCLUDE_JCON_SERVER_DEV_H

#include <jcon_server.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handler to destroy the session context.
 * 
 * Session carries its own functions and data used
 * for implementations. If the data is dynamically
 * allocated. To free this memory at the destruction
 * of the session, this handler can be provided.
 * 
 * @param ctx Session context to destroy.
 */
typedef void(*jcon_server_free_handler_t)(void *ctx);

/**
 * @brief Function to handle session reset calls.
 * 
 * Restarts server.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if reset was successful.
 * @return    @c false , if reset failed.
 */
typedef int(*jcon_server_reset_handler_t)(void *ctx);

/**
 * @brief Function to handle session close calls.
 * 
 * Closes server.
 * 
 * @param ctx Context pointer for session data.
 */
typedef void(*jcon_server_close_handler_t)(void *ctx);

/**
 * @brief Function to handle requests for connection state.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if connected.
 * @return    @c false , if not connected or error occured.
 */
typedef int(*jcon_server_isOpen_handler_t)(void *ctx);

/**
 * @brief Function to handle request for reference string.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    String with reference to server connection.
 * @return    NULL, if failed.
 */
typedef const char*(*jcon_server_getReferenceString_handler_t)(void *ctx);

/**
 * @brief Function to handle requests, if server has new connection available.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if new connection is available.
 * @return    @c false , if no new connection or error occured.
 */
typedef int(*jcon_server_newConnection_handler_t)(void *ctx);

/**
 * @brief Function to accept new connection.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    jcon_client session object, connected to new client.
 * @return    @c NULL , if no new connection or error occured.
 */
typedef jcon_client_t*(*jcon_server_acceptConnection_handler_t)(void *ctx);

/**
 * @brief jcon_server session object, holds data and functions for operation.
 */
struct __jcon_server_session
{
  jcon_server_free_handler_t session_free_handler;                      /**< Pointer to function, which frees context memory. */

  jcon_server_reset_handler_t function_reset;                           /**< Pointer to function, to reset server. */
  jcon_server_close_handler_t function_close;                           /**< Pointer to function, to close server. */

  jcon_server_isOpen_handler_t function_isOpen;                         /**< Pointer to function, to check wether the server is open. */
  jcon_server_getReferenceString_handler_t function_getReferenceString; /**< Pointer to function, which returns connection info. */

  jcon_server_newConnection_handler_t function_newConnection;           /**< Pointer to function, which checks if new connections are available. */
  jcon_server_acceptConnection_handler_t function_acceptConnection;     /**< Pointer to function, which accepts and returns new connection. */

  const char *connection_type;                                          /**< String to show, which type of connection the session is holding. */
  void *session_context;                                                /**< Context pointer, holds data for implementation. */
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_DEV_H */
/**
 * @file jcon_client_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definition for implementations of jcon_client.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_CLIENT_DEV_H
#define INCLUDE_JCON_CLIENT_DEV_H

#include <jayc/jcon_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function to handle session reset calls.
 *
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if reset was successful.
 * @return    @c false , if reset failed.
 */
typedef int(*jcon_client_reset_function_t)(void *ctx);

/**
 * @brief Function to handle session close calls.
 *
 * @param ctx Context pointer for session data.
 */
typedef void(*jcon_client_close_function_t)(void *ctx);

/**
 * @brief Function to handle request for reference string.
 * 
 * @param ctx Context pointer for session data.
 * 
 * @return    String with reference to client connection.
 * @return    NULL, if failed.
 */
typedef const char* (*jcon_client_getReferenceString_function_t)(void *ctx);

/**
 * @brief Function to handle request for connection state.
 *
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if connected.
 * @return    @c false , if not connected or error occured.
 */
typedef int(*jcon_client_isConnected_function_t)(void *ctx);

/**
 * @brief Function to handle requests if new data is available.
 *
 * @param ctx Context pointer for session data.
 * 
 * @return    @c true , if new data is available.
 * @return    @c false , if no data available or error occured.
 */
typedef int(*jcon_client_newData_function_t)(void *ctx);

/**
 * @brief Function to handle calls for data recieving.
 *
 * @param ctx       Context pointer for session data.
 * @param data_ptr  Pointer, in which data is stored.
 *                  If NULL, bytes will still be read (number given by
 *                  data_size), but nothing will be returned.
 * @param data_size Size (in bytes) of data to read.
 * 
 * @return          Size of data recieved.
 * @return          @c 0 , if no data recieved, or error occured.
 */
typedef size_t(*jcon_client_recvData_function_t)(void *ctx, void *data_ptr, size_t data_size);

/**
 * @brief Function to handle calls for data sending.
 *
 * @param ctx       Context pointer for session data.
 * @param data_ptr  Pointer to data to be sent.
 *                  If NULL, nothing will happen.
 * @param data_size Size of data_ptr in bytes.
 * 
 * @return          Size of data sended.
 * @return          @c 0 , if no data written or error occured.
 */
typedef size_t(*jcon_client_sendData_function_t)(void *ctx, void *data_ptr, size_t data_size);


/**
 * @brief Handler to destroy session. Session carries its own function to free the context memory.
 *
 * @param ctx Session context to free.
 */
typedef void(*jcon_client_session_free_handler_t)(void *ctx);

/**
 * @brief jcon_client session object, holds data and functions for operation.
 */
struct __jcon_client_session
{
  jcon_client_reset_function_t function_reset;                            /**< Pointer to function, to reset session. */
  jcon_client_close_function_t function_close;                            /**< Pointer to function, to close session. */
  jcon_client_getReferenceString_function_t function_getReferenceString;  /**< Pointer to function, to get connection information. */
  jcon_client_isConnected_function_t function_isConnected;                /**< Pointer to function, to check wether there is a connection. */

  jcon_client_newData_function_t function_newData;                        /**< Pointer to function, to check wether new data is available to read. */
  jcon_client_recvData_function_t function_recvData;                      /**< Pointer to function, with which to recieve data. */
  jcon_client_sendData_function_t function_sendData;                      /**< Pointer to function, with which to send data. */

  jcon_client_session_free_handler_t session_free_handler;                /**< Pointer to function, with which to free context memory. */

  const char *connection_type;                                            /**< String to show, which type of connection the session is holding. */
  void *session_context;                                                  /**< Context pointer, holds data for operation. */
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_DEV_H */
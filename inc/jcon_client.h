/**
 * @file jcon_client.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief A Connector interface, that can call customized implementations.
 * 
 * jcon_client is a customizable connector system. It provides the core
 * interface to use with any implementation.
 * 
 * @todo add more information ...
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_CLIENT_H
#define INCLUDE_JCON_CLIENT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief jcon_client session object, holds data and functions for operation.
 */
typedef struct __jcon_client_session jcon_client_t;

/**
 * @brief Frees session memory.
 *
 * @param session Session object to destroy.
 */
void jcon_client_session_free(jcon_client_t *session);

/**
 * @brief Reset connection of session.
 *
 * @param  session  Session to reset.
 * 
 * @return          @c true , if reset was successful.
 * @return          @c false , if reset failed.
 */
int jcon_client_reset(jcon_client_t *session);

/**
 * @brief Close connection of session.
 *
 * @param session Session to close.
 */
void jcon_client_close(jcon_client_t *session);

/**
 * @brief Get type of connection.
 *
 * @param session Session to check.
 * 
 * @return        String representing type of connection.
 * @return        @c NULL in case of error.
 */
const char *jcon_client_getConnectionType(jcon_client_t *session);

/**
 * @brief Get string that shows information about client connection.
 *
 * @param session Session to check.
 * 
 * @return        String with client connection info.
 * @return        @c NULL in case of error.
 */
const char *jcon_client_getReferenceString(jcon_client_t *session);

/**
 * @brief Check if session is connected.
 *
 * @param session Session to check.
 * 
 * @return        @c true , if session is connected.
 * @return        @c false , if session is not connected or error occured.
 */
int jcon_client_isConnected(jcon_client_t *session);

/**
 * @brief Check if there is new data available to read.
 *
 * @param session Session to check.
 * 
 * @return        @c true , if new data is available.
 * @return        @c false , if no data or error occured.
 */
int jcon_client_newData(jcon_client_t *session);

/**
 * @brief Recieve data from session.
 *
 * @param session   Session to recieve data from.
 * @param data_ptr  Pointer, in which data is stored.
 *                  If NULL, bytes will still be read (number given by
 *                  data_size), but nothing will be returned.
 * @param data_size Size (in bytes) of data to read.
 * 
 * @return          Size of data recieved.
 * @return          @c 0 , if no data recieved, or error occured.
 */
size_t jcon_client_recvData(jcon_client_t *session, void *data_ptr, size_t data_size);

/**
 * @brief Send data through session.
 * 
 * If return is not equal to @c data_size, something went wrong
 * in the transmittion and not all the data was sent.
 *
 * @param session   Session to send data through.
 * @param data_ptr  Pointer to data to be sent.
 *                  If NULL, nothing will happen.
 * @param data_size Size of data_ptr in bytes.
 * 
 * @return          Size of data sended.
 * @return          @c 0 , if no data written or error occured.
 */
size_t jcon_client_sendData(jcon_client_t *session, void *data_ptr, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_H */
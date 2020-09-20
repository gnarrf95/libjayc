#ifndef INCLUDE_JCON_CLIENT_H
#define INCLUDE_JCON_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*******************************************************************************
 * @brief jcon_client session object, holds data and functions for operation.
 */
typedef struct __jcon_client_session jcon_client_t;

/*******************************************************************************
 * @brief Frees session memory.
 *
 * @param session : Session object to Destroy.
 */
void jcon_client_session_free(jcon_client_t *session);

/*******************************************************************************
 * @brief Reset connection of session.
 *
 * @param session : Session to reset.
 * 
 * @return : 1/true if reset was successful, else 0/false.
 */
int jcon_client_reset(jcon_client_t *session);

/*******************************************************************************
 * @brief Close connection of session.
 *
 * @param session : Session to close.
 */
void jcon_client_close(jcon_client_t *session);

/*******************************************************************************
 * @brief Get type of connection.
 *
 * @param session : Session to check.
 * 
 * @return : String representing type of connection.
 */
const char *jcon_client_getConnectionType(jcon_client_t *session);

/*******************************************************************************
 * @brief Get string that shows information about client connection.
 *
 * @param session : Session to check.
 * 
 * @return : String with client connection info.
 */
const char *jcon_client_getReferenceString(jcon_client_t *session);

/*******************************************************************************
 * @brief Check if session is connected.
 *
 * @param session : Session to check.
 * 
 * @return : 1/true if connected, else 0/false.
 */
int jcon_client_isConnected(jcon_client_t *session);

/*******************************************************************************
 * @brief Check if there is new data available to read.
 *
 * @param session : Session to check.
 * 
 * @return : 1/true if new data is available, else 0/false.
 */
int jcon_client_newData(jcon_client_t *session);

/*******************************************************************************
 * @brief Recieve data from session.
 *
 * @param session : Session to recieve data from.
 * @param data_ptr : Pointer, in which data is stored.
 *                   If NULL, bytes will still be read (number given by data_size),
 *                   but nothing will be returned.
 * @param data_size : Size (in bytes) of data to read.
 * 
 * @return : Size of data recieved. If not equal to data_size,
 *           something went wrong in the transmittion. If 0, no data was recieved.
 *           Will return 0 in case of error.
 */
size_t jcon_client_recvData(jcon_client_t *session, void *data_ptr, size_t data_size);

/*******************************************************************************
 * @brief Send data through session.
 *
 * @param session : Session to send data through.
 * @param data_ptr : Pointer to data to be sent.
 *                   If NULL, nothing will happen.
 * @param data_size : Size of data_ptr in bytes.
 * 
 * @return : Size of data sended. If not equal to data_size, something went
 *           wrong in transmittion.
 *           Will return 0 in case of error.
 */
size_t jcon_client_sendData(jcon_client_t *session, void *data_ptr, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_H */
#ifndef INCLUDE_JCON_CLIENT_DEV_H
#define INCLUDE_JCON_CLIENT_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jcon_client.h>

/*******************************************************************************
 * @brief Function to handle session reset calls.
 *
 * @param ctx : Context pointer for session data.
 * 
 * @return : 1/true if reset was successful, else 0/false.
 */
typedef int(*jcon_client_reset_function_t)(void *ctx);

/*******************************************************************************
 * @brief Function to handle session close calls.
 *
 * @param ctx : Context pointer for session data.
 */
typedef void(*jcon_client_close_function_t)(void *ctx);

/*******************************************************************************
 * @brief Function to handle request for connection state.
 *
 * @param ctx : Context pointer for session data.
 * 
 * @return : 1/true if connected, else 0/false.
 */
typedef int(*jcon_client_isConnected_function_t)(void *ctx);

/*******************************************************************************
 * @brief Function to handle requests if new data is available.
 *
 * @param ctx : Context pointer for session data.
 * 
 * @return : 1/true if new data is available, else 0/false.
 */
typedef int(*jcon_client_newData_function_t)(void *ctx);

/*******************************************************************************
 * @brief Function to handle calls for data recieving.
 *
 * @param ctx : Context pointer for session data.
 * @param data_ptr : Pointer, in which data is stored.
 *                   If NULL, bytes will still be read (number given by data_size),
 *                   but nothing will be returned.
 * @param data_size : Size (in bytes) of data to read.
 * 
 * @return : Size of data recieved. If not equal to data_size,
 *           something went wrong in the transmittion. If 0, no data was recieved.
 *           Will return 0 in case of error.
 */
typedef size_t(*jcon_client_recvData_function_t)(void *ctx, void *data_ptr, size_t data_size);

/*******************************************************************************
 * @brief Function to handle calls for data sending.
 *
 * @param ctx : Context pointer for session data.
 * @param data_ptr : Pointer to data to be sent.
 *                   If NULL, nothing will happen.
 * @param data_size : Size of data_ptr in bytes.
 * 
 * @return : Size of data sended. If not equal to data_size, something went
 *           wrong in transmittion.
 *           Will return 0 in case of error.
 */
typedef size_t(*jcon_client_sendData_function_t)(void *ctx, void *data_ptr, size_t data_size);


/*******************************************************************************
 * @brief Handler to destroy session. Session carries its own function to free the context memory.
 *
 * @param ctx : Session context to free.
 */
typedef void(*jcon_client_session_free_handler_t)(void *ctx);

/*******************************************************************************
 * @brief jcon_client session object, holds data and functions for operation.
 *
 * @data function_reset : Pointer to function, to reset session.
 * @data function_close : Pointer to function, to close session.
 * @data function_isConnected : Pointer to function, to check wether there is a connection.
 * @data function_newData : Pointer to function, to check wether new data is available to read.
 * @data function_recvData : Pointer to function, with which to recieve data.
 * @data function_sendData : Pointer to function, with which to send data.
 * @data session_free_handler : Pointer to function, with which to free context memory.
 * @data connection_type : String to show, which type of connection the session is holding.
 * @data session_context : Context pointer, holds data for operation.
 */
struct __jcon_client_session
{
  jcon_client_reset_function_t function_reset;
  jcon_client_close_function_t function_close;
  jcon_client_isConnected_function_t function_isConnected;

  jcon_client_newData_function_t function_newData;
  jcon_client_recvData_function_t function_recvData;
  jcon_client_sendData_function_t function_sendData;

  jcon_client_session_free_handler_t session_free_handler;

  const char *connection_type;
  void *session_context;
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_DEV_H */
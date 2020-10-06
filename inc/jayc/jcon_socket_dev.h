/**
 * @file jcon_socket_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definition for implementations of jcon_socket.
 * 
 * @date 2020-10-05
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SOCKET_DEV_H
#define INCLUDE_JCON_SOCKET_DEV_H

#include <jayc/jcon_socket.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The connection type is not defined yet.
 * 
 * In case socket is not yet created or closed.
 */
#define JCON_SOCKET_CONNECTIONTYPE_NOTDEF 0

/**
 * @brief Socket operates as client.
 * 
 * Forbids session from using function
 * @c #jcon_socket_accept() .
 */
#define JCON_SOCKET_CONNECTIONTYPE_CLIENT 1

/**
 * @brief Socket operates as server.
 * 
 * Forbids session from using functions
 * @c #jcon_socket_recvData() and @c #jcon_socket_sendData() .
 */
#define JCON_SOCKET_CONNECTIONTYPE_SERVER 2

/**
 * @brief Handler function to connect socket.
 * 
 * @param session Session to connect.
 * 
 * @return        @c true , if successfully connected.
 * @return        @c false , if error occured.
 */
typedef int(*jcon_socket_connect_handler_t)(jcon_socket_t *session);

/**
 * @brief Handler function to bind socket.
 * 
 * @param session Session to bind.
 * 
 * @return        @c true , if successfully bound.
 * @return        @c false , if error occured.
 */
typedef int(*jcon_socket_bind_handler_t)(jcon_socket_t *session);

/**
 * @brief Handler function to do cleanup before socket descriptor is closed.
 * 
 * @param session Session to connect.
 */
typedef void(*jcon_socket_close_handler_t)(jcon_socket_t *session);

/**
 * @brief Handler function to accept connection.
 * 
 * @param session Session to accept connection.
 * 
 * @return        New connection session.
 * @return        @c NULL , if error occured.
 */
typedef jcon_socket_t *(*jcon_socket_accept_handler_t)(jcon_socket_t *session);

/**
 * @brief Handler function to free session memory.
 * 
 * @param session Session to free.
 */
typedef void(*jcon_socket_free_handler_t)(jcon_socket_t *session);

/**
 * @brief Session object.
 * 
 * Holds data for socket operation.
 */
struct __jcon_socket_session
{
  int file_descriptor;                              /**< File descriptor for socket. */
  int connection_type;                              /**< Enumeration with values:
                                                         * @c #JCON_SOCKET_CONNECTIONTYPE_NETDEF ,
                                                         * @c #JCON_SOCKET_CONNECTIONTYPE_CLIENT ,
                                                         * @c #JCON_SOCKET_CONNECTIONTYPE_SERVER . */
                                                      
  char *socket_type;                                /**< Tells what kind of socket it is. */
  char *referenceString;                            /**< Holds information about connection. */
  jlog_t *logger;                                   /**< Logger to use for debug and error output. */

  jcon_socket_connect_handler_t function_connect;   /**< Handler to be called by @c #jcon_socket_connect() . */
  jcon_socket_bind_handler_t function_bind;         /**< Handler to be called by @c #jcon_socket_bind() . */
  jcon_socket_close_handler_t function_close;       /**< Handler to be called by @c #jcon_socket_close() . */
  jcon_socket_accept_handler_t function_accept;     /**< Handler to be called by @c #jcon_socket_accept() . */
  jcon_socket_free_handler_t session_free_handler;  /**< Handler to be called by @c #jcon_socket_free() . */

  void *session_ctx;                                /**< Session context used for implementations. */
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SOCKET_DEV_H */
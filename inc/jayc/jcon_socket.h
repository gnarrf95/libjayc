/**
 * @file jcon_socket.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief General functions for socket operation.
 * 
 * @date 2020-10-05
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SOCKET_H
#define INCLUDE_JCON_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Session object.
 * 
 * Holds data for socket operation.
 */
typedef struct __jcon_socket_session jcon_socket_t;

/**
 * @brief Frees session memory.
 * 
 * @param session Session object to free.
 */
void jcon_socket_free(jcon_socket_t *session);

/**
 * @brief Connect to server.
 * 
 * Creates socket and connects to a server.
 * This will mark the session as a client,
 * so no server functions ( @c #jcon_socket_accept() )
 * can be called using this session.
 * 
 * @param session Session to connect.
 * 
 * @return        @c true , if connection was established.
 * @return        @c false , if connection failed.
 */
int jcon_socket_connect(jcon_socket_t *session);

/**
 * @brief Binds socket to address.
 * 
 * This will mark the session as a server,
 * therefore no client functions
 * ( @c #jcon_socket_recvData() and @c #jcon_socket_sendData() )
 * can be called using this session.
 * 
 * @param session Session to bind.
 * 
 * @return        @c true , if socket was bound to address.
 * @return        @c false , if binding failed.
 */
int jcon_socket_bind(jcon_socket_t *session);

/**
 * @brief Closes socket.
 * 
 * This will close the socket and
 * unmark the session.
 * Afterwards it can be created as a client
 * or server again.
 * 
 * @param session Session to close.
 */
void jcon_socket_close(jcon_socket_t *session);

/**
 * @brief Checks wether input is available on socket.
 * 
 * Checks for input using @c poll() .
 * 
 * <b>For clients:</b>
 * @c true as return says, that data is available
 * to read.
 * 
 * <b>For servers:</b>
 * @c true means, that a new client is requesting
 * a connection.
 * 
 * Also detects disconnects and automatically
 * closes the session.
 * 
 * @param session Session to check.
 * @param timeout Timeout for @c poll() in milliseconds.
 *                Stops blocking, if no new input was
 *                detected in time.
 * 
 * @return        @c true , if new input is available.
 * @return        @c false , if @c poll() timed out,
 *                or error occured.
 */
int jcon_socket_pollForInput(jcon_socket_t *session, int timeout);

/**
 * @brief Accepts connection request.
 * 
 * <b>Server function</b>
 * 
 * If new connection is available, accepts the connection
 * and returns client connection as new session.
 * 
 * @param session Server session, to accept connection.
 * 
 * @return        Session object of client connection.
 * @return        @c NULL , if no new connection was
 *                available or error occured.
 */
jcon_socket_t *jcon_socket_accept(jcon_socket_t *session);

/**
 * @brief Recieve data from socket.
 * 
 * <b>Client function</b>
 * 
 * If new data is available (check using @c #jcon_tcp_pollForInput() ),
 * data can be read using this function.
 * 
 * Data is read from socket using buffer, and if data_ptr is not
 * @c NULL , is copied in data_ptr.
 * If size of data read exceeds the buffer size (data_size), it is
 * trimmed to fit in data_ptr. This is done to prevent buffer
 * overflow.
 * 
 * If data_ptr is @c NULL , the data is still read, but it is
 * discarded afterwards.
 * This can be used to skip offsets in binary data.
 * 
 * If EOF is recieved, the session is automatically closed.
 * 
 * @param session   Session to read from.
 * @param data_ptr  Buffer to hold data.
 * @param data_size Buffer size.
 * 
 * @return          Size of data read.
 * @return          @c 0 , if no data read or error occured.
 */
size_t jcon_socket_recvData(jcon_socket_t *session, void *data_ptr, size_t data_size);

/**
 * @brief Send data via socket.
 * 
 * <b>Client function</b>
 * 
 * If socket is connected, data can be sent using this function.
 * The data in data_ptr is written, but only so many bytes as data_size
 * states.
 * 
 * This function detects a disconnect and automatically closes
 * the session.
 * 
 * @param session   Session to send to.
 * @param data_ptr  Buffer with data to read.
 * @param data_size Buffer size.
 * 
 * @return          Size of data sent.
 * @return          @c 0 , if no data sent or error occured.
 */
size_t jcon_socket_sendData(jcon_socket_t *session, void *data_ptr, size_t data_size);

/**
 * @brief Checks if the session is connected.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if session connected.
 * @return        @c false , if session closed or error occured.
 */
int jcon_socket_isConnected(jcon_socket_t *session);

/**
 * @brief Returns what type of socket the session is holding.
 * 
 * <b>Example:</b> "TCP", "UNIX"
 * 
 * @param session Session to check.
 * 
 * @return        String with socket type.
 * @return        @c NULL , if error occured.
 */
const char *jcon_socket_getSocketType(jcon_socket_t *session);

/**
 * @brief Returns string with connection information.
 * 
 * <b>Example:</b> "TCP:127.0.0.1:8080"
 * <b>Example:</b> "UNIX:/tmp/test.uds"
 * 
 * @param session Session to check.
 * 
 * @return        String with connection info.
 * @return        @c NULL , if error occured.
 */
const char *jcon_socket_getReferenceString(jcon_socket_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SOCKET_H */
/**
 * @file jcon_tcp.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Provides interface to handle TCP sockets.
 * 
 * Abstracts usage of TCP sockets.
 * Manages error handling and other background tasks.
 * 
 * Provides client and server functionality.
 * 
 * @date 2020-09-27
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_TCP_H
#define INCLUDE_JCON_TCP_H

#include <jayc/jlog.h>
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
typedef struct __jcon_tcp_session jcon_tcp_t;

/**
 * @brief Simple initializer. Only essential information needed.
 * 
 * TCP/IP stack provides a lot of options and flags to
 * initialize and manage a socket.
 * This function only requires the bare minimum to create
 * a connection.
 * 
 * @param address IP/DNS address of server to connect to.
 * @param port    Port to connect to.
 * @param logger  Logger to use in session.
 * 
 * @return        Session object.
 * @return        @c NULL , if error occured.
 */
jcon_tcp_t *jcon_tcp_simple_init(const char *address, uint16_t port, jlog_t *logger);

/**
 * @brief Frees session memory.
 * 
 * @param session Session object to free.
 */
void jcon_tcp_free(jcon_tcp_t *session);

/**
 * @brief Connect to server.
 * 
 * Creates socket and connects to a server.
 * This will mark the session as a client,
 * so no server functions ( @c #jcon_tcp_accept() )
 * can be called using this session.
 * 
 * @param session Session to connect.
 * 
 * @return        @c true , if connection was established.
 * @return        @c false , if connection failed.
 */
int jcon_tcp_connect(jcon_tcp_t *session);

/**
 * @brief Binds socket to address.
 * 
 * This will mark the session as a server,
 * therefore no client functions
 * ( @c #jcon_tcp_recvData() and @c #jcon_tcp_sendData() )
 * can be called using this session.
 * 
 * @param session Session to bind.
 * 
 * @return        @c true , if socket was bound to address.
 * @return        @c false , if binding failed.
 */
int jcon_tcp_bind(jcon_tcp_t *session);

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
void jcon_tcp_close(jcon_tcp_t *session);

/**
 * @brief Shuts socket down.
 * 
 * Will shutdown socket and wait until
 * other side disconnects. Then closes
 * socket. This should avoid the @c TIME_WAIT
 * issue.
 * 
 * @param session Session to close.
 */
void jcon_tcp_shutdown(jcon_tcp_t *session);

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
 * @param timeout Timeout for @c poll() . Stops blocking,
 *                if no new input was detected in time.
 * 
 * @return        @c true , if new input is available.
 * @return        @c false , if @c poll() timed out,
 *                or error occured.
 */
int jcon_tcp_pollForInput(jcon_tcp_t *session, int timeout);

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
jcon_tcp_t *jcon_tcp_accept(jcon_tcp_t *session);

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
 * This can be used o skip offsets in binary data.
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
size_t jcon_tcp_recvData(jcon_tcp_t *session, void *data_ptr, size_t data_size);

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
size_t jcon_tcp_sendData(jcon_tcp_t *session, void *data_ptr, size_t data_size);

/**
 * @brief Checks if the session is connected.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if session connected.
 * @return        @c false , if session closed or error occured.
 */
int jcon_tcp_isConnected(jcon_tcp_t *session);

/**
 * @brief Returns string with address of socket.
 * 
 * <b>Example:</b> "TCP:127.0.0.1:8080"
 * 
 * @param session Session to check.
 * 
 * @return        String with connection info.
 * @return        @c NULL , if error occured.
 */
const char *jcon_tcp_getReferenceString(jcon_tcp_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_TCP_H */
/**
 * @file jcon_client_tcp.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief TCP socket connector, implemented using jcon_client.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_CLIENT_TCP_H
#define INCLUDE_JCON_CLIENT_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jcon_client.h>

#include <jlog.h>
#include <netinet/in.h>

/**
 * @brief Initialize client with IP and port.
 * 
 * Creates new socket. Uses DNS resolution, so also supports DNS names.
 * 
 * @param address IP address or DNS name of target server.
 * @param port    Port, to which to connect.
 * @param logger  jlog logger to use. If @c NULL , uses global logger.
 * 
 * @return        jcon_client session object.
 * @return        @c NULL , if an error occured.
 */
jcon_client_t *jcon_client_tcp_session_init(char *address, uint16_t port, jlog_t *logger);

/**
 * @brief Initializes client with already existing socket.
 * 
 * For example when accepting connection from server.
 * Can be used to handle server connections.
 * 
 * @param file_descriptor File descriptor of socket.
 * @param socket_address  Address struct of connection.
 * @param logger          jlog logger to use. If @c NULL , uses global logger.
 * 
 * @return                jcon_client session object.
 * @return                @c NULL , if an error occured.
 */
jcon_client_t *jcon_client_tcp_session_clone(int file_descriptor, struct sockaddr_in socket_address, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_TCP_H */
/**
 * @file jcon_client_tcp.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief TCP socket connector, implemented using jcon_client.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @see jcon_client.h
 * 
 */

#ifndef INCLUDE_JCON_CLIENT_TCP_H
#define INCLUDE_JCON_CLIENT_TCP_H

#include <jayc/jcon_client.h>
#include <jayc/jcon_tcp.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Initialize client from existing jcon_tcp client session.
 * 
 * When jcon_tcp server session accepts new connection,
 * it returns a new session, that can be used to create a
 * new client.
 * 
 * @param tcp_session jcon_tcp session to use.
 * @param logger      Logger to use.
 * 
 * @return            jcon_client session for new connection.
 * @return            @c NULL , if error occured.
 */
jcon_client_t *jcon_client_tcp_session_tcpClone(jcon_tcp_t *tcp_session, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_TCP_H */
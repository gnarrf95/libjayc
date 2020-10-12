/**
 * @file jcon_server_tcp.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief TCP implementation of jcon_server.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SERVER_TCP_H
#define INCLUDE_JCON_SERVER_TCP_H

#include <jayc/jcon_server.h>
#include <jayc/jlog.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize server with IP and port.
 * 
 * Creates server socket.
 * 
 * @param address IP address, the server will be open to.
 * @param port    Port, the server will be open to.
 * @param logger  jlog logger to use. If @c NULL , uses global logger.
 * 
 * @return        jcon_server session object.
 * @return        @c NULL , if an error occured.
 */
jcon_server_t *jcon_server_tcp_session_init(char *address, uint16_t port, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_TCP_H */
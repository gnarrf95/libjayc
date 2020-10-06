/**
 * @file jcon_socketTCP.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief TCP variant of jcon_socket.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SOCKETTCP_H
#define INCLUDE_JCON_SOCKETTCP_H

#include <jayc/jcon_socket.h>
#include <jayc/jcon_socket_dev.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

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
jcon_socket_t *jcon_socketTCP_simpleInit(const char *address, uint16_t port, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SOCKETTCP_H */
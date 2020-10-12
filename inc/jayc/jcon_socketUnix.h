/**
 * @file jcon_socketUnix.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Unix variant of jcon_socket.
 * 
 * @date 2020-10-06
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SOCKETUNIX_H
#define INCLUDE_JCON_SOCKETUNIX_H

#include <jayc/jcon_socket.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Simple initializer. Only essential information needed.
 * 
 * Unix socket stack provides a lot of options and flags to
 * initialize and manage a socket.
 * This function only requires the bare minimum to create
 * a connection.
 * 
 * @param filepath  Path to UDS file.
 * @param logger    Logger to use in session.
 * 
 * @return          Session object.
 * @return          @c NULL , if error occured.
 */
jcon_socket_t *jcon_socketUnix_simple_init(const char *filepath, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SOCKETUnix_H */
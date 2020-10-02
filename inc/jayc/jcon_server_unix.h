/**
 * @file jcon_server_unix.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Unix implementation of jcon_server.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SERVER_UNIX_H
#define INCLUDE_JCON_SERVER_UNIX_H

#include <jayc/jcon_server.h>
#include <jayc/jlog.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize server with uds file path.
 * 
 * Creates server socket.
 * 
 * @param filepath  File path, the server will be open to.
 * @param logger    jlog logger to use. If @c NULL , uses global logger.
 * 
 * @return          jcon_server session object.
 * @return          @c NULL , if an error occured.
 */
jcon_server_t *jcon_server_unix_session_init(char *filepath, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_UNIX_H */
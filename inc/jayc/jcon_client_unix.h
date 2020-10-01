/**
 * @file jcon_client_unix.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief UNIX socket connector, implemented using jcon_client.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @see jcon_client.h
 * 
 */

#ifndef INCLUDE_JCON_CLIENT_UNIX_H
#define INCLUDE_JCON_CLIENT_UNIX_H

#include <jayc/jcon_client.h>
#include <jayc/jcon_unix.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize client with uds file path.
 * 
 * Creates new socket. 
 * 
 * @param filepath  Path to uds file of server.
 * @param logger    jlog logger to use. If @c NULL , uses global logger.
 * 
 * @return          jcon_client session object.
 * @return          @c NULL , if an error occured.
 */
jcon_client_t *jcon_client_unix_session_init(char *filepath, jlog_t *logger);

/**
 * @brief Initialize client from existing jcon_unix client session.
 * 
 * When jcon_unix server session accepts new connection,
 * it returns a new session, that can be used to create a
 * new client.
 * 
 * @param unix_session  jcon_unix session to use.
 * @param logger        Logger to use.
 * 
 * @return              jcon_client session for new connection.
 * @return              @c NULL , if error occured.
 */
jcon_client_t *jcon_client_unix_session_unixClone(jcon_unix_t *unix_session, jlog_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_CLIENT_UNIX_H */
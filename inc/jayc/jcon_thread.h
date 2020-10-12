/**
 * @file jcon_thread.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief This system automates jcon_client handling in a thread.
 * 
 * A jcon_thread system initializes with a jcon_client session and creates
 * a thread, in which the client continousely checks for new data.
 * If new data is available, a @c #jcon_thread_data_handler_t function
 * (defined at initialization) gets called, so the user can decide,
 * how the data gets read and processed.
 * 
 * There are also handlers for when the thread gets started
 * ( @c #jcon_thread_create_handler_t ) and gets closed
 * ( @c #jcon_thread_close_handler_t ).
 * The @c create_ and @c close_handler also provide information, about
 * how the client was created (initialized, cloned) and closed
 * (disconnected, externally closed).
 * 
 * The session holds a session context, that is provided at initialization
 * and passed into all headers, to use in implementation.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @todo Is @c #jcon_thread_create_handler_t needed?
 * 
 */

#ifndef INCLUDE_JCON_THREAD_H
#define INCLUDE_JCON_THREAD_H

#include <jayc/jcon_client_dev.h>
#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Define object types.
//

/**
 * @brief jcon_thread session object.
 */
typedef struct __jcon_thread_session jcon_thread_t;



//==============================================================================
// Define create and close types.
//

/**
 * @brief A new connection was initialized.
 */
#define JCON_THREAD_CREATETYPE_INIT 0

/**
 * @brief A connection was cloned.
 * 
 * For example, when a server recieved a connection.
 */
#define JCON_THREAD_CREATETYPE_CLONE 1

/**
 * @brief The client lost connection.
 */
#define JCON_THREAD_CLOSETYPE_DISCONNECT 0

/**
 * @brief The client got manually closed.
 */
#define JCON_THREAD_CLOSETYPE_EXTERN 1



//==============================================================================
// Define handler types.
//

/**
 * @brief Handles how data is read, when available.
 * 
 * Gets called when data is available to read from client
 * ( @c #jcon_client_newData() ). Data needs to be read inside function.
 * 
 * @param ctx     Session context provided at initialization.
 * @param client  jcon_client with available data.
 * 
 * @see @c #jcon_client_recvData()
 */
typedef void(*jcon_thread_data_handler_t)(void *ctx, jcon_client_t *client);

/**
 * @brief Handler, that gets called when thread is created.
 * 
 * @param ctx               Session context provided at initialization.
 * @param create_type       How the client was created.
 * @param reference_string  Shows connection information of client.
 * 
 * @see @c #JCON_THREAD_CREATETYPE_INIT
 * @see @c #JCON_THREAD_CREATETYPE_CLONE
 * 
 * @todo Is this really useful?
 */
typedef void (*jcon_thread_create_handler_t)(void *ctx, int create_type, const char *reference_string);

/**
 * @brief Handler, that gets called when client session is closed.
 * 
 * @param ctx               Session context provided at initialization.
 * @param close_type        How the client was closed.
 * @param reference_string  Shows connection information of client.
 * 
 * @see @c #JCON_THREAD_CLOSETYPE_INIT
 * @see @c #JCON_THREAD_CLOSETYPE_CLONE
 */
typedef void (*jcon_thread_close_handler_t)(void *ctx, int close_type, const char *reference_string);



//==============================================================================
// Define interface functions.
//

/**
 * @brief Initializes and starts the jcon_thread.
 * 
 * @param client          jcon_client to use.
 * @param data_handler    Handles new available data.
 * @param create_handler  Gets called, when jcon_thread is created.
 * @param close_handler   Gets called, when jcon_client is closed.
 * @param logger          Logger to use for debug messages.
 * @param ctx             Session context passed to handlers.
 * 
 * @return                New jcon_thread session object.
 * @return                @c NULL, if failed.
 */
jcon_thread_t *jcon_thread_init
(
  jcon_client_t *client,
  jcon_thread_data_handler_t data_handler,
  jcon_thread_create_handler_t create_handler,
  jcon_thread_close_handler_t close_handler,
  jlog_t *logger,
  void *ctx
);

/**
 * @brief Stops thread and frees session memory.
 * 
 * <b>Note:</b>
 * Function does not free memory of client.
 * Client must be handled manually!
 * 
 * @param session Session object to free.
 */
void jcon_thread_free(jcon_thread_t *session);

/**
 * @brief Checks if thread is still running.
 * 
 * @param session Session to check.
 * @return        @c true , if thread is running.
 * @return        @c false , if thread is not running.
 */
int jcon_thread_isRunning(jcon_thread_t *session);

/**
 * @brief Returns connection type of jcon_client.
 * 
 * @param session Session to check.
 * 
 * @return        Connection type of jcon_client.
 * @return        @c NULL , if failed.
 */
const char *jcon_thread_getConnectionType(jcon_thread_t *session);

/**
 * @brief Returns reference string of jcon_client.
 * 
 * @param session Session to check.
 * 
 * @return        Reference string of jcon_client.
 * @return        @c NULL , if failed.
 */
const char *jcon_thread_getReferenceString(jcon_thread_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_THREAD_H */
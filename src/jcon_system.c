/**
 * @file jcon_system.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-24
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_system.h>
#include <jcon_server.h>
#include <jcon_thread.h>
#include <jutil_thread.h>
#include <jutil_linkedlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

//==============================================================================
// Define constants.
//

#define JCON_SYSTEM_LOOPSLEEP_DEFAULT 100000000



//==============================================================================
// Declare data structures.
//

/**
 * @brief Structure for linked list.
 */
typedef struct __jcon_system_connectionPair
{
  jcon_thread_t *thread;  /**< jcon_thread session. */
  jcon_client_t *client;  /**< jcon_client session used for jcon_thread. */
} jcon_system_connection_t;

/**
 * @brief Session object. Holds data for operation.
 */
struct __jcon_system_session
{
  jcon_server_t *server;                              /**< Server to handle. */
  jutil_linkedlist_t *connections;                    /**< List of connections. */
  jutil_thread_t *control_thread;                     /**< Thread to control server and connection list. */

  jcon_system_threadData_handler_t data_handler;      /**< Handler to manage, when data is available through a client. */
  jcon_system_threadCreate_handler_t create_handler;  /**< Handler to manage, when new client is connected. */
  jcon_system_threadClose_handler_t close_handler;    /**< Handler to manage, when client disconnects. */

  jlog_t *logger;                                     /**< Logger for debug and error messages. */
  void *session_context;                              /**< Session context to pass to handlers. */
};



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Loop function for control thread.
 * 
 * Manages new connections and disconnects.
 * 
 * @param ctx             jcon_system session object.
 * @param thread_handler  jcon_thread session. Used for mutex.
 * 
 * @return                @c true , if thread should continue.
 * @return                @c false , if thread should stop.
 */
static int jcon_system_control_function(void *ctx, jutil_thread_t *thread_handler);

/**
 * @brief Restarts server.
 * 
 * @param session System session.
 * 
 * @return        @c true , if restart was successful.
 * @return        @c false , if error occured.
 */
static int jcon_system_resetServer(jcon_system_t *session);


/**
 * @brief Stops server.
 * 
 * Currently not used.
 * Server should be stopped outside of system.
 * 
 * @param session System session.
 */
// static void jcon_system_closeServer(jcon_system_t *session);

/**
 * @brief Manages new connections.
 * 
 * Checks if new connection to server are available
 * and if, accepts connection and adds them to
 * linked list.
 * 
 * @param session System session.
 */
static void jcon_system_checkForConnections(jcon_system_t *session);

/**
 * @brief Manages disconnects.
 * 
 * Checks if clients have disconnected
 * and if, closes sessions and removes them
 * from linked list.
 * 
 * @param session System session.
 */
static void jcon_system_cleanupConnections(jcon_system_t *session);

/**
 * @brief Frees all connections and removes them.
 * 
 * @param session System session.
 */
static void jcon_system_clearConnections(jcon_system_t *session);

/**
 * @brief Adds new connecton to linked list.
 * 
 * @param session System session.
 * @param client  Client for new connection.
 * 
 * @return        @c true , if client was added to list.
 * @return        @c false , if error occured.
 */
static int jcon_system_addConnection(jcon_system_t *session, jcon_client_t *client);

/**
 * @brief Closes connection and removes it from list.
 * 
 * @param session         System session.
 * @param connection_node Node in linked list of connection.
 */
static void jcon_system_freeConnection(jcon_system_t *session, jutil_linkedlist_t *connection_node);

/**
 * @brief Handler for jcon_thread.
 * 
 * Currently not used.
 * 
 * @param ctx               Context pointer provided by user.
 * @param create_type       Either @c #JCON_THREAD_CREATETYPE_INIT
 *                          or @c #JCON_THREAD_CREATETYPE_CLONE .
 * @param reference_string  Reference string of the client created.
 */
static void jcon_system_connectionThread_create(void *ctx, int create_type, const char *reference_string);

/**
 * @brief Handler for jcon_thread.
 * 
 * Currently not used.
 * 
 * @param ctx               Context pointer provided by user.
 * @param create_type       Either @c #JCON_THREAD_CLOSETYPE_DISCONNECT
 *                          or @c #JCON_THREAD_CLOSETYPE_EXTERN .
 * @param reference_string  Reference string of the client closed.
 */
static void jcon_system_connectionThread_close(void *ctx, int close_type, const char *reference_string);

/**
 * @brief Logs debug and error messages.
 * 
 * Uses logger from @c ctx , or if logger is @c NULL , uses global logger.
 * 
 * @param session   System session.
 * @param log_type  Type of log message.
 * @param file      Source code file, where message was logged.
 * @param function  Function in which message was logged.
 * @param line      Line, where message was logged.
 * @param fmt       Format string for stdarg.h .
 */
static void jcon_system_log(jcon_system_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG /* Allows to disable debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_system_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jcon_system_t *jcon_system_init
(
  jcon_server_t *server,
  jcon_system_threadData_handler_t data_handler,
  jcon_system_threadCreate_handler_t create_handler,
  jcon_system_threadClose_handler_t close_handler,
  jlog_t *logger,
  void *ctx
)
{
  if(server == NULL)
  {
    ERROR(NULL,"Server is NULL.");
    return NULL;
  }

  jcon_system_t *session = (jcon_system_t *)malloc(sizeof(jcon_system_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }

  session->server = server;
  session->connections = NULL;
  session->data_handler = data_handler;
  session->create_handler = create_handler;
  session->close_handler = close_handler;
  session->logger = logger;
  session->session_context = ctx;

  session->control_thread = jutil_thread_init
  (
    jcon_system_control_function,
    logger,
    JCON_SYSTEM_LOOPSLEEP_DEFAULT,
    session
  );
  if(session->control_thread == NULL)
  {
    ERROR(session, "jutil_thread_init() failed. Destroying session.");
    free(session);
    return NULL;
  }

  if(jcon_system_isServerOpen(session) == false)
  {
    if(jcon_system_resetServer(session) == false)
    {
      ERROR(session, "jcon_system_resetServer() failed. Destroying session.");
      jutil_thread_free(session->control_thread);
      free(session);
      return NULL;
    }
  }

  if(jutil_thread_start(session->control_thread) == false)
  {
    ERROR(session, "jutil_thread_start() failed. Destroying session.");
    jutil_thread_free(session->control_thread);
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_system_free(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_thread_free(session->control_thread);
  jcon_system_clearConnections(session);

  free(session);
}

//------------------------------------------------------------------------------
//
const char *jcon_system_getConnectionType(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return NULL;
  }

  return jcon_server_getConnectionType(session->server);
}

//------------------------------------------------------------------------------
//
const char *jcon_system_getReferenceString(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return NULL;
  }

  return jcon_server_getReferenceString(session->server);
}

//------------------------------------------------------------------------------
//
int jcon_system_isServerOpen(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return false;
  }

  return jcon_server_isOpen(session->server);
}

//------------------------------------------------------------------------------
//
int jcon_system_control_isRunning(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return jutil_thread_isRunning(session->control_thread);
}

//------------------------------------------------------------------------------
//
size_t jcon_system_getConnectionNumber(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return 0;
  }

  size_t ret;

  jutil_thread_lockMutex(session->control_thread);
  ret = jutil_linkedlist_size(&session->connections);
  jutil_thread_unlockMutex(session->control_thread);

  return ret;
}



//==============================================================================
// Implement functions for control thread.
//

//------------------------------------------------------------------------------
//
int jcon_system_control_function(void *ctx, jutil_thread_t *thread_handler)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  if(thread_handler == NULL)
  {
    ERROR(NULL, "thread_handler is NULL.");
    return false;
  }

  jcon_system_t *session = (jcon_system_t *)ctx;
  int ret = true;

  /* Check for closed connections. */
  jutil_thread_lockMutex(thread_handler);
  jcon_system_cleanupConnections(session);
  jutil_thread_unlockMutex(thread_handler);

  /* Check for new connections. */
  jutil_thread_lockMutex(thread_handler);
  jcon_system_checkForConnections(session);
  jutil_thread_unlockMutex(thread_handler);

  return ret;
}



//==============================================================================
// Implement functions for server control.
//

//------------------------------------------------------------------------------
//
int jcon_system_resetServer(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return false;
  }

  int ret;
  jutil_thread_lockMutex(session->control_thread);
  ret = jcon_server_reset(session->server);
  jutil_thread_unlockMutex(session->control_thread);

  if(ret == false)
  {
    ERROR(session, "jcon_server_reset() failed.");
  }

  return ret;
}

/*
//------------------------------------------------------------------------------
//
void jcon_system_closeServer(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(session->server == NULL)
  {
    ERROR(NULL, "Server is NULL.");
    return;
  }

  jcon_server_close(session->server);
}
*/



//==============================================================================
// Implement functions for server connections.
//

//------------------------------------------------------------------------------
//
void jcon_system_checkForConnections(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(jcon_server_newConnection(session->server))
  {
    jcon_client_t *new_client = jcon_server_acceptConnection(session->server);
    if(new_client)
    {
      if(jcon_system_addConnection(session, new_client) == false)
      {
        ERROR(session, "jcon_system_addConnection() failed.");
      }
    }
    else
    {
      ERROR(session, "jcon_server_acceptConnection() failed.");
    }
  }
}

//------------------------------------------------------------------------------
//
void jcon_system_cleanupConnections(jcon_system_t *session)
{

  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_linkedlist_t *itr = session->connections;

  while(itr != NULL)
  {
    jcon_system_connection_t *connection = (jcon_system_connection_t *)jutil_linkedlist_getData(itr);
    if(connection)
    {
      if(jcon_thread_isRunning(connection->thread) == false)
      {
        jcon_system_freeConnection(session, itr);
        break;
      }
      else
      {
        itr = jutil_linkedlist_iterate(itr);
      }
    }
    else
    {
      jutil_linkedlist_removeNode(&session->connections, itr);
      break;
    }
  }
}

//------------------------------------------------------------------------------
//
void jcon_system_clearConnections(jcon_system_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_linkedlist_t *itr = session->connections;
  while(itr != NULL)
  {
    DEBUG(session, "Destroying node [%p].", itr);
    jutil_linkedlist_t *tmp = itr;
    itr = jutil_linkedlist_iterate(itr);
    jcon_system_freeConnection(session, tmp);
  }
}

//------------------------------------------------------------------------------
//
int jcon_system_addConnection(jcon_system_t *session, jcon_client_t *client)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  if(client == NULL)
  {
    ERROR(session, "Client is NULL.");
    return false;
  }

  jcon_system_connection_t *new_connection = (jcon_system_connection_t *)malloc(sizeof(jcon_system_connection_t));
  if(new_connection == NULL)
  {
    ERROR(session, "malloc() failed.");
    return false;
  }

  new_connection->client = client;
  new_connection->thread = jcon_thread_init
  (
    client,
    session->data_handler,
    jcon_system_connectionThread_create,
    jcon_system_connectionThread_close,
    session->logger,
    NULL
  );
  if(new_connection->thread == NULL)
  {
    ERROR(session, "jcon_thread_init() failed.");
    free(new_connection);
    return false;
  }

  if(jutil_linkedlist_append(&session->connections, (void *)new_connection) == false)
  {
    ERROR(session, "jutil_linkedlist_append() failed.");
    jcon_thread_free(new_connection->thread);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void jcon_system_freeConnection(jcon_system_t *session, jutil_linkedlist_t *connection_node)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  if(connection_node == NULL)
  {
    ERROR(session, "Connection Node is NULL.");
    return;
  }

  jcon_system_connection_t *connection = (jcon_system_connection_t *)jutil_linkedlist_getData(connection_node);
  if(connection)
  {
    if(connection->thread)
    {
      jcon_thread_free(connection->thread);
    }
    if(connection->client)
    {
      DEBUG(session, "Freeing client [%s].", jcon_client_getReferenceString(connection->client));
      jcon_client_session_free(connection->client);
    }

    free(connection);
  }

  if(jutil_linkedlist_removeNode(&session->connections, connection_node) == NULL)
  {
    ERROR(session, "jutil_linkedlist_removeNode() failed.");
  }
}



//==============================================================================
// Implement handlers for jcon_thread.
//

//------------------------------------------------------------------------------
//
void jcon_system_connectionThread_create(void *ctx, int create_type, const char *reference_string)
{
}

//------------------------------------------------------------------------------
//
void jcon_system_connectionThread_close(void *ctx, int close_type, const char *reference_string)
{
}



//==============================================================================
// Implement log function.
//

//------------------------------------------------------------------------------
//
void jcon_system_log(jcon_system_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session)
  {
    if(session->logger && session->server)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_system_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_system_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
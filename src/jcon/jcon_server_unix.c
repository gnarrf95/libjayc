/**
 * @file jcon_server_unix.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jcon_server_unix.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_server_unix.h>
#include <jayc/jcon_server_dev.h>
#include <jayc/jcon_client_unix.h>
#include <jayc/jcon_socketUnix.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

//==============================================================================
// Define constants and defaults.
//

/**
 * @brief Connection type, to return for @c #jcon_client_getConnectionType() .
 */
#define JCON_SERVER_UNIX_CONNECTIONTYPE "UNIX"

/**
 * @brief Default value for polling timeout.
 * 
 * When checking, if new data is available, function @c poll()
 * is used. This value tells the function, how long to
 * wait for new data in milliseconds.
 */
#define JCON_SERVER_UNIX_POLL_TIMEOUT_DEFAULT 10



//==============================================================================
// Declare handlers and internal functions.
//

/**
 * @brief Function for context free handler.
 * 
 * Will free context data.
 * 
 * @param ctx Session context to free.
 */
static void jcon_server_unix_session_free(void *ctx);

/**
 * @brief Function for reset handler.
 * 
 * Restarts the server.
 * 
 * @param ctx Context pointer with socket data.
 * 
 * @return    @c true , if reset was successful.
 * @return    @c false , if reset failed.
 */
static int jcon_server_unix_reset(void *ctx);

/**
 * @brief Function for close handler.
 * 
 * Closes server socket.
 * 
 * @param ctx Context pointer with socket data.
 */
static void jcon_server_unix_close(void *ctx);

/**
 * @brief Checks if socket is open.
 * 
 * @param ctx Context pointer with socket data.
 * 
 * @return    @c true , if socket is open.
 * @return    @c false , if socket closed or error occured.
 */
static int jcon_server_unix_isOpen(void *ctx);

/**
 * @brief Returnes @c jcon_server_unix_context_t#reference_string .
 * 
 * @param ctx Context of session to ask from.
 * 
 * @return    Session string.
 * @return    @c NULL , if error occured.
 */
static const char *jcon_server_unix_getReferenceString(void *ctx);

/**
 * @brief Checks, if new connection is available.
 * 
 * Polls socket, to check if new connections are available.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    @c true , if new connection is available.
 * @return    @c false , if no connection or error occured.
 */
static int jcon_server_unix_newConnection(void *ctx);

/**
 * @brief Accepts connection and creates jcon_client.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    New jcon_client session object.
 * @return    @c NULL , if error occured.
 */
static jcon_client_t *jcon_server_unix_acceptConnection(void *ctx);

/**
 * @brief Logs debug and error messages.
 * 
 * Uses logger from @c ctx , or if logger is @c NULL , uses global logger.
 * 
 * @param ctx       Session for info about connection.
 * @param log_type  Type of log message.
 * @param file      Source code file, where message was logged.
 * @param function  Function in which message was logged.
 * @param line      Line, where message was logged.
 * @param fmt       Format string for stdarg.h .
 */
static void jcon_server_unix_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG
  #define DEBUG(ctx, fmt, ...)
#else
  #define DEBUG(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(ctx, fmt, ...) jcon_server_unix_log(ctx, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Define context structure.
//

/**
 * @brief Data for jcon_server_unix object.
 */
typedef struct __jcon_server_unix_context
{
  jcon_socket_t *server;                 /**< jcon_unix session object. */
  int poll_timeout;                   /**< Timeout for asking for new data in milliseconds. */
  jlog_t *logger;                     /**< Logger for debug and error messages. */
} jcon_server_unix_context_t;



//==============================================================================
// Implement handlers and internal functions.
//

//------------------------------------------------------------------------------
//
jcon_server_t *jcon_server_unix_session_init(char *filepath, jlog_t *logger)
{
  jcon_server_t *session = (jcon_server_t *)malloc(sizeof(jcon_server_t));
  if(session == NULL)
  {
    ERROR(NULL, "<UNIX:%s> malloc() failed.", filepath);
    return NULL;
  }

  session->session_free_handler = &jcon_server_unix_session_free;
  session->function_reset = &jcon_server_unix_reset;
  session->function_close = &jcon_server_unix_close;
  session->function_isOpen = &jcon_server_unix_isOpen;
  session->function_getReferenceString = &jcon_server_unix_getReferenceString;
  session->function_newConnection = &jcon_server_unix_newConnection;
  session->function_acceptConnection = &jcon_server_unix_acceptConnection;
  session->connection_type = JCON_SERVER_UNIX_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_server_unix_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<UNIX:%s> malloc() failed. Destroying session.", filepath);
    free(session);
    return NULL;
  }

  jcon_server_unix_context_t *ctx = (jcon_server_unix_context_t *)session->session_context;

  ctx->poll_timeout = JCON_SERVER_UNIX_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;

  ctx->server = jcon_socketUnix_simple_init(filepath, logger);
  if(ctx->server == NULL)
  {
    ERROR(NULL, "<UNIX:%s> jcon_socketUnix_simple_init() failed. Destroying context and session.", filepath);
    free(ctx);
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_server_unix_session_free(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  if(jcon_server_unix_isOpen(ctx) == false)
  {
    jcon_server_unix_close(ctx);
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;
  jcon_socket_free(session_context->server);

  free(ctx);
}

//------------------------------------------------------------------------------
//
int jcon_server_unix_reset(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  if(jcon_server_unix_isOpen(ctx) == false)
  {
    jcon_server_unix_close(ctx);
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  return jcon_socket_bind(session_context->server);
}

//------------------------------------------------------------------------------
//
void jcon_server_unix_close(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  if(jcon_server_unix_isOpen(ctx) == false)
  {
    DEBUG(ctx, "Server already closed.");
    return;
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  jcon_socket_close(session_context->server);
}


//------------------------------------------------------------------------------
//
int jcon_server_unix_isOpen(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  return jcon_socket_isConnected(session_context->server);
}

//------------------------------------------------------------------------------
//
const char *jcon_server_unix_getReferenceString(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  return jcon_socket_getReferenceString(session_context->server);
}

//------------------------------------------------------------------------------
//
int jcon_server_unix_newConnection(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  return jcon_socket_pollForInput(session_context->server, session_context->poll_timeout);
}

//------------------------------------------------------------------------------
//
jcon_client_t *jcon_server_unix_acceptConnection(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

  jcon_socket_t *new_connection = jcon_socket_accept(session_context->server);
  if(new_connection == NULL)
  {
    ERROR(ctx, "jcon_unix_accept() failed.");
    return NULL;
  }

  jcon_client_t *new_client = jcon_client_unix_session_unixClone(new_connection, session_context->logger);
  if(new_client == NULL)
  {
    ERROR(ctx, "jcon_client_unix_session_unixClone() failed.");
    jcon_socket_free(new_connection);
    return NULL;
  }

  return new_client;
}

//------------------------------------------------------------------------------
//
void jcon_server_unix_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(ctx)
  {
    jcon_server_unix_context_t *session_context = (jcon_server_unix_context_t *)ctx;

    if(session_context->logger)
    {
      jlog_log_message_m(session_context->logger, log_type, file, function, line, "<%s> %s", jcon_server_unix_getReferenceString(ctx), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_server_unix_getReferenceString(ctx), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
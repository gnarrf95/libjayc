/**
 * @file jcon_client_unix.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jcon_client_unix.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_client_unix.h>
#include <jayc/jcon_client_dev.h>
#include <jayc/jcon_unix.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/un.h>

//==============================================================================
// Define constants and defaults.
//

/**
 * @brief Connection type, to return for @c #jcon_client_getConnectionType() .
 */
#define JCON_CLIENT_UNIX_CONNECTIONTYPE "UNIX"

/**
 * @brief Default value for polling timeout.
 * 
 * When checking, if new data is available, function @c poll()
 * is used. This value tells the function, how long to
 * wait for new data in milliseconds.
 */
#define JCON_CLIENT_UNIX_POLL_TIMEOUT_DEFAULT 10



//==============================================================================
// Declare handlers and internal functions.
//

/**
 * @brief Function for context free handler.
 * 
 * Will close socket, if connected and free context data.
 * 
 * @param ctx Context to free.
 */
static void jcon_client_unix_session_free(void *ctx);

/**
 * @brief Resets socket.
 * 
 * Not usable, if initialized via @c #jcon_client_unix_session_clone() .
 * 
 * @param ctx Context of session to reset.
 * 
 * @return    @c true , if reset was successful.
 * @return    @c false , if reset failed.
 */
static int jcon_client_unix_reset(void *ctx);

/**
 * @brief Closes connection.
 * 
 * @param ctx Context of session to close.
 */
static void jcon_client_unix_close(void *ctx);

/**
 * @brief Checks, wether client is connected.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    @c true , if client is connected.
 * @return    @c false , if client is not connected or error occured.
 */
static int jcon_client_unix_isConnected(void *ctx);

/**
 * @brief Returnes @c jcon_client_unix_context_t#reference_string .
 * 
 * @param ctx Context of session to ask from.
 * 
 * @return    Session string.
 * @return    @c NULL , if error occured.
 */
static const char *jcon_client_unix_getReferenceString(void *ctx);

/**
 * @brief Checks, if new data is available to read.
 * 
 * @param ctx Context of session to check.
 * 
 * @return    @c true , if new data is available.
 * @return    @c false , if no new data or error occured.
 */
static int jcon_client_unix_newData(void *ctx);

/**
 * @brief Recieves data from socket.
 * 
 * @param ctx       Context of session to read from.
 * @param data_ptr  Pointer, in which data is stored.
 *                  If NULL, bytes will still be read (number given by
 *                  data_size), but nothing will be returned.
 * @param data_size Size (in bytes) of data to read.
 * 
 * @return          Size of data recieved.
 * @return          @c 0 , if no data recieved, or error occured.
 */
static size_t jcon_client_unix_recvData(void *ctx, void *data_ptr, size_t data_size);

/**
 * @brief Send data through socket.
 *
 * @param ctx       Context of session to send through.
 * @param data_ptr  Pointer to data to be sent.
 *                  If NULL, nothing will happen.
 * @param data_size Size of data_ptr in bytes.
 * 
 * @return          Size of data sended.
 * @return          @c 0 , if no data written or error occured.
 */
static size_t jcon_client_unix_sendData(void *ctx, void *data_ptr, size_t data_size);

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
static void jcon_client_unix_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG
  #define DEBUG(ctx, fmt, ...)
#else
  #define DEBUG(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(ctx, fmt, ...) jcon_client_unix_log(ctx, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Define context structure.
//

/**
 * @brief Data for jcon_client_unix object.
 */
typedef struct __jcon_client_unix_context
{
  jcon_socket_t *connection;            /**< jcon_unix session object. */
  int poll_timeout;                   /**< Timeout for asking for new data in milliseconds. */
  jlog_t *logger;                     /**< Logger for debug and error messages. */
} jcon_client_unix_context_t;



//==============================================================================
// Implement handlers and internal functions.
//

//------------------------------------------------------------------------------
//
jcon_client_t *jcon_client_unix_session_init(char *filepath, jlog_t *logger)
{
  jcon_client_t *session = (jcon_client_t *)malloc(sizeof(jcon_client_t));
  if(session == NULL)
  {
    ERROR(NULL, "<UNIX:%s> malloc() failed.", filepath);
    return NULL;
  }

  session->function_reset = &jcon_client_unix_reset;
  session->function_close = &jcon_client_unix_close;
  session->function_getReferenceString = &jcon_client_unix_getReferenceString;
  session->function_isConnected = &jcon_client_unix_isConnected;
  session->function_newData = &jcon_client_unix_newData;
  session->function_recvData = &jcon_client_unix_recvData;
  session->function_sendData = &jcon_client_unix_sendData;
  session->session_free_handler = &jcon_client_unix_session_free;
  session->connection_type = JCON_CLIENT_UNIX_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_client_unix_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<UNIX:%s> malloc() failed. Destroying session.", filepath);
    free(session);
    return NULL;
  }

  jcon_client_unix_context_t *ctx = (jcon_client_unix_context_t *)session->session_context;

  ctx->poll_timeout = JCON_CLIENT_UNIX_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;

  ctx->connection = jcon_socketUnix_simple_init(filepath, logger);
  if(ctx->connection == NULL)
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
jcon_client_t *jcon_client_unix_session_unixClone(jcon_socket_t *unix_session, jlog_t *logger)
{
  if(unix_session == NULL)
  {
    ERROR(NULL, "<UNIX> unix_session is NULL.");
    return NULL;
  }

  jcon_client_t *session = (jcon_client_t *)malloc(sizeof(jcon_client_t));
  if(session == NULL)
  {
    ERROR(NULL, "<UNIX> malloc() failed.");
    return NULL;
  }

  session->function_reset = &jcon_client_unix_reset;
  session->function_close = &jcon_client_unix_close;
  session->function_getReferenceString = &jcon_client_unix_getReferenceString;
  session->function_isConnected = &jcon_client_unix_isConnected;
  session->function_newData = &jcon_client_unix_newData;
  session->function_recvData = &jcon_client_unix_recvData;
  session->function_sendData = &jcon_client_unix_sendData;
  session->session_free_handler = &jcon_client_unix_session_free;
  session->connection_type = JCON_CLIENT_UNIX_CONNECTIONTYPE;
  session->session_context = malloc(sizeof(jcon_client_unix_context_t));
  if(session->session_context == NULL)
  {
    ERROR(NULL, "<unix> malloc() failed. Destroying session.");
    free(session);
    return NULL;
  }

  jcon_client_unix_context_t *ctx = (jcon_client_unix_context_t *)session->session_context;

  ctx->poll_timeout = JCON_CLIENT_UNIX_POLL_TIMEOUT_DEFAULT;
  ctx->logger = logger;
  ctx->connection = unix_session;

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_client_unix_session_free(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_client_unix_close(ctx);
  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  jcon_socket_free(session_context->connection);
  free(ctx);
}

//------------------------------------------------------------------------------
//
int jcon_client_unix_reset(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  if(jcon_client_isConnected(ctx))
  {
    jcon_client_unix_close(ctx);
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  if(jcon_socket_connect(session_context->connection) == false)
  {
    ERROR(ctx, "jcon_unix_connect() failed.");
    return false;
  }
  
  return true;
}

//------------------------------------------------------------------------------
//
void jcon_client_unix_close(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  if(jcon_client_unix_isConnected(ctx) == false)
  {
    DEBUG(ctx, "Client not connected.");
    return;
  }

  jcon_socket_close(session_context->connection);
}

//------------------------------------------------------------------------------
//
int jcon_client_unix_isConnected(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  return jcon_socket_isConnected(session_context->connection);
}

//------------------------------------------------------------------------------
//
const char *jcon_client_unix_getReferenceString(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return NULL;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  return jcon_socket_getReferenceString(session_context->connection);
}

//------------------------------------------------------------------------------
//
int jcon_client_unix_newData(void *ctx)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return false;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  return jcon_socket_pollForInput(session_context->connection, session_context->poll_timeout);
}

//------------------------------------------------------------------------------
//
size_t jcon_client_unix_recvData(void *ctx, void *data_ptr, size_t data_size)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

  return jcon_socket_recvData(session_context->connection, data_ptr, data_size);
}

//------------------------------------------------------------------------------
//
size_t jcon_client_unix_sendData(void *ctx, void *data_ptr, size_t data_size)
{
  if(ctx == NULL)
  {
    ERROR(NULL, "Context is NULL.");
    return 0;
  }

  jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;
  
  return jcon_socket_sendData(session_context->connection, data_ptr, data_size);
}

//------------------------------------------------------------------------------
//
void jcon_client_unix_log(void *ctx, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(ctx)
  {
    jcon_client_unix_context_t *session_context = (jcon_client_unix_context_t *)ctx;

    if(session_context->logger)
    {
      jlog_log_message_m(session_context->logger, log_type, file, function, line, "<%s> %s", jcon_client_unix_getReferenceString(ctx), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_client_unix_getReferenceString(ctx), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
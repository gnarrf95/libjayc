/**
 * @file jcon_thread.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements jcon_thread.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jcon_thread.h>
#include <jutil_thread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

//==============================================================================
// Define constants and defaults.
//

#define JCON_THREAD_LOOPSLEEP_DEFAULT 100000000



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Loop function that manages client.
 * 
 * Gets executed every loop iteration.
 * Checks if data is available.
 * Stops executing if client disconnects.
 * 
 * @param session_ptr     Pointer to jcon_thread session.
 * @param thread_handler  jutil_thread handler. Used to manage mutex.
 * 
 * @return                @c true , if thread should continue.
 * @return                @c false , if thread should stop.
 */
static int jcon_thread_run_function(void *session_ptr, jutil_thread_t *thread_handler);

/**
 * @brief Logs debug and error messages.
 * 
 * Uses logger from @c session , or if logger is @c NULL , uses global logger.
 * 
 * @param session     Session for info about connection.
 * @param log_type    Type of log message.
 * @param file        Source code file, where message was logged.
 * @param function    Function in which message was logged.
 * @param line        Line, where message was logged.
 * @param fmt         Format string for stdarg.h .
 */
static void jcon_thread_log(jcon_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);



//==============================================================================
// Define log macros.
//

#ifdef JCON_NO_DEBUG /* Allows to disable debug messages at compile time. */
  #define DEBUG(session, fmt, ...)
#else
  #define DEBUG(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(session, fmt, ...) jcon_thread_log(session, JLOG_LOGTYPE_FATAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)



//==============================================================================
// Define context structure.
//

/**
 * @brief Holds data necessary for jcon_thread runtime.
 */
struct __jcon_thread_session
{
  jcon_client_t *client;                        /**< Client for connection. */
  jutil_thread_t *thread;                       /**< Thread session. */

  jcon_thread_data_handler_t data_handler;      /**< Handler to call, when data is available. */
  jcon_thread_create_handler_t create_handler;  /**< Handler to call, when thread is created. */
  jcon_thread_close_handler_t close_handler;    /**< Handler to call, when thread is closed. */

  jlog_t *logger;                               /**< Logger for debug and error messages. */
  void *session_context;                        /**< Context pointer to pass to handlers. */
};



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
jcon_thread_t *jcon_thread_init
(
  jcon_client_t *client,
  jcon_thread_data_handler_t data_handler,
  jcon_thread_create_handler_t create_handler,
  jcon_thread_close_handler_t close_handler,
  jlog_t *logger,
  void *ctx
)
{
  if(client == NULL)
  {
    ERROR(NULL, "Client is NULL.");
    return NULL;
  }

  jcon_thread_t *session = (jcon_thread_t *)malloc(sizeof(jcon_thread_t));
  if(session == NULL)
  {
    ERROR(NULL, "malloc() failed.");
    return NULL;
  }
  
  session->client = client;
  session->data_handler = data_handler;
  session->create_handler = create_handler;
  session->close_handler = close_handler;

  session->logger = logger;
  session->session_context = ctx;

  session->thread = jutil_thread_init
  (
    jcon_thread_run_function,
    logger,
    JCON_THREAD_LOOPSLEEP_DEFAULT,
    session
  );
  if(session->thread == NULL)
  {
    ERROR(session, "jutil_thread_init() failed. Destroying session.");
    free(session);
    return NULL;
  }

  if(jutil_thread_start(session->thread) == false)
  {
    ERROR(session, "jutil_thread_start() failed. Destroying session.");
    jutil_thread_free(session->thread);
    free(session);
    return NULL;
  }

  return session;
}

//------------------------------------------------------------------------------
//
void jcon_thread_free(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return;
  }

  jutil_thread_free(session->thread);
  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_thread_isRunning(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return jutil_thread_isRunning(session->thread);
}

//------------------------------------------------------------------------------
//
const char *jcon_thread_getConnectionType(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return NULL;
  }

  return jcon_client_getConnectionType(session->client);
}

//------------------------------------------------------------------------------
//
const char *jcon_thread_getReferenceString(jcon_thread_t *session)
{
  if(session == NULL)
  {
    ERROR(NULL, "Session is NULL.");
    return false;
  }

  return jcon_client_getReferenceString(session->client);
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
int jcon_thread_run_function(void *session_ptr, jutil_thread_t *thread_handler)
{
  if(session_ptr == NULL)
  {
    ERROR(NULL, "session_ptr is NULL.");
    return false;
  }

  if(thread_handler == NULL)
  {
    ERROR(NULL, "thread_handler is NULL.");
    return false;
  }

  jcon_thread_t *session = (jcon_thread_t *)session_ptr;
  int ret = true;
  
  /* Check for new data */
  jutil_thread_lockMutex(thread_handler);
  if(jcon_client_newData(session->client))
  {
    DEBUG(session, "New data available.");
    if(session->data_handler)
    {
      session->data_handler(
        session->session_context,
        session->client
      );
    }
  }
  jutil_thread_unlockMutex(thread_handler);

  /* Check connection state */
  jutil_thread_lockMutex(thread_handler);
  if(jcon_client_isConnected(session->client) == false)
  {
    DEBUG(session, "Client disconnect.");
    if(session->close_handler)
    {
      session->close_handler(
        session->session_context,
        JCON_THREAD_CLOSETYPE_DISCONNECT,
        jcon_thread_getReferenceString(session)
      );
    }

    ret = false;
  }
  jutil_thread_unlockMutex(thread_handler);

  return ret;
}

//------------------------------------------------------------------------------
//
void jcon_thread_log(jcon_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(session && session->client)
  {
    if(session->logger)
    {
      jlog_log_message_m(session->logger, log_type, file, function, line, "<%s> %s", jcon_thread_getReferenceString(session), buf);
    }
    else
    {
      jlog_global_log_message_m(log_type, file, function, line, "<%s> %s", jcon_thread_getReferenceString(session), buf);
    }
  }
  else
  {
    jlog_global_log_message_m(log_type, file, function, line, buf);
  }
}
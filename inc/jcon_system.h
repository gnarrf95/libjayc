/**
 * @file jcon_system.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-24
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SYSTEM_H
#define INCLUDE_JCON_SYSTEM_H

#include <jcon_server.h>
#include <jcon_thread.h>
#include <jutil_linkedlist.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*jcon_system_threadData_handler_t)(void *ctx, jcon_client_t *client);
typedef void(*jcon_system_threadCreate_handler_t)(void *ctx, const char *ref_string);
typedef void(*jcon_system_threadClose_handler_t)(void *ctx, const char *ref_string);

typedef struct __jcon_system_session jcon_system_t;
typedef struct __jcon_system_connectionPair jcon_system_connection_t;

struct __jcon_system_session
{
  jcon_server_t *server;
  jutil_linkedlist_t *connections;

  pthread_t control_thread;
  pthread_mutex_t mutex;
  long loop_sleep;
  int control_run;

  jcon_system_threadData_handler_t data_handler;
  jcon_system_threadCreate_handler_t create_handler;
  jcon_system_threadClose_handler_t close_handler;

  jlog_t *logger;
  void *session_context;
};

struct __jcon_system_connectionPair
{
  jcon_thread_t *thread;
  jcon_client_t *client;
};

jcon_system_t *jcon_system_init
(
  jcon_server_t *server,
  jcon_system_threadData_handler_t data_handler,
  jcon_system_threadCreate_handler_t create_handler,
  jcon_system_threadClose_handler_t close_handler,
  jlog_t *logger,
  void *ctx
);

void jcon_system_free(jcon_system_t *session);

const char *jcon_system_getConnectionType(jcon_system_t *session);
const char *jcon_system_getReferenceString(jcon_system_t *session);

int jcon_system_isServerOpen(jcon_system_t *session);
int jcon_system_resetServer(jcon_system_t *session);
void jcon_system_closeServer(jcon_system_t *session);

int jcon_system_control_isRunning(jcon_system_t *session);
int jcon_system_control_start(jcon_system_t *session);
void jcon_system_control_stop(jcon_system_t *session);

size_t jcon_system_getConnectionNumber(jcon_system_t *session);
void jcon_system_clearConnections(jcon_system_t *session);

int jcon_system_addConnection(jcon_system_t *session, jcon_client_t *client);
void jcon_system_freeConnection(jcon_system_t *session, jutil_linkedlist_t *connection_node);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SYSTEM_H */
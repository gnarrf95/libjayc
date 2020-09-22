/**
 * @file jcon_server_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definition for implementations of jcon_server.
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JCON_SERVER_DEV_H
#define INCLUDE_JCON_SERVER_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jcon_server.h>
#include <jcon_thread.h>
#include <jcon_client.h>
#include <jcon_linked_list.h>
#include <pthread.h>

void jcon_server_log(jcon_server_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

typedef void(*jcon_server_free_handler_t)(void *ctx);

typedef int(*jcon_server_reset_handler_t)(void *ctx);

typedef void(*jcon_server_close_handler_t)(void *ctx);

typedef int(*jcon_server_isRunning_handler_t)(void *ctx);

typedef const char*(*jcon_server_getReferenceString_handler_t)(void *ctx);

typedef int(*jcon_server_listen_handler_t)(void *ctx);
typedef jcon_client_t*(*jcon_server_accept_handler_t)(void *ctx);

struct __jcon_server_session
{
  pthread_t control_thread;
  pthread_mutex_t mutex;
  jcon_linked_list_node_t *connections;
  int run;

  jlog_t *logger;
  const char *connection_type;
  void *implementation_context;
  void *session_context;

  jcon_client_session_free_handler_t server_free_handler;
  jcon_server_reset_handler_t server_reset_handler;
  jcon_server_close_handler_t server_close_handler;
  jcon_server_isRunning_handler_t server_isRunning_handler;
  jcon_server_getReferenceString_handler_t server_getReferenceString_handler;
  jcon_server_listen_handler_t server_listen_handler;
  jcon_server_accept_handler_t server_accept_handler;

  jcon_thread_data_handler_t data_handler;
  jcon_thread_create_handler_t create_handler;
  jcon_thread_close_handler_t close_handler;
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JCON_SERVER_DEV_H */
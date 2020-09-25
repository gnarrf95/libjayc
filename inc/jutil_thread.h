/**
 * @file jutil_thread.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Abstraction for pthread library.
 * 
 * @date 2020-09-25
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_THREAD_H
#define INCLUDE_JUTIL_THREAD_H

#include <pthread.h>
#include <jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JUTIL_THREAD_STATE_STOPPED 0  /**< pthread can be created. */
#define JUTIL_THREAD_STATE_INIT 1     /**< pthread is initializing. */
#define JUTIL_THREAD_STATE_RUNNING 2  /**< pthread is running, can be stopped and joined. */
#define JUTIL_THREAD_STATE_FINISHED 3 /**< pthread is finished, waiting to be joined. */

typedef struct __jutil_thread_session jutil_thread_t;

/**
 * @brief User defined loop function.
 * 
 * This function will be passed to session at initialization.
 * Tells thread what to do each iteration.
 * If return is 1, tells handler that the thread is finished.
 * 
 * @param ctx Context used by implementation.
 * 
 * @return    @c 0 , if thread should continue.
 * @return    @c 1 , if thread should stop.
 */
typedef int(*jutil_thread_loop_function_t)(void *ctx, void *mutex_ptr);

struct __jutil_thread_session
{
  pthread_t thread;
  pthread_mutex_t mutex;
  jutil_thread_loop_function_t loop_function;
  long loop_sleep;
  jlog_t *logger;

  int thread_state;
  int run_signal; /**< If set to 0, the thread will stop. */
  
  void *ctx;
};

void *jutil_thread_pthread_handler(void *ctx);

void jutil_thread_manage(jutil_thread_t *session);

jutil_thread_t *jutil_thread_init(jutil_thread_loop_function_t function, jlog_t *logger, long sleep_ns, void *ctx);
void jutil_thread_free(jutil_thread_t *session);

int jutil_thread_start(jutil_thread_t *session);
void jutil_thread_stop(jutil_thread_t *session);

void *jutil_thread_getMutexPtr(jutil_thread_t *session);

void jutil_thread_lockMutex(void *mutex_ptr);
void jutil_thread_unlockMutex(void *mutex_ptr);

int jutil_thread_isRunning(jutil_thread_t *session);

int jutil_thread_pthread_create(jutil_thread_t *session);
void jutil_thread_pthread_join(jutil_thread_t *session);
int jutil_thread_pmutex_init(jutil_thread_t *session);
void jutil_thread_pmutex_destroy(jutil_thread_t *session);
void jutil_thread_pmutex_lock(pthread_mutex_t *mutex);
void jutil_thread_pmutex_unlock(pthread_mutex_t *mutex);

void jutil_thread_log(jutil_thread_t *session, int log_type, const char *file, const char *function, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_THREAD_H */
/**
 * @file jutil_thread.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Abstraction for pthread library.
 * 
 * Manages error handling and other background tasks.
 * 
 * Provides easy to use functionality to check
 * state of a thread and control it from outside.
 * 
 * Provides simple function callback interface and
 * mutex handling.
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

/**
 * @brief Holds data for thread runtime and operation.
 * 
 */
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
 * @return    @c 1 , if thread should continue.
 * @return    @c 0 , if thread should stop.
 */
typedef int(*jutil_thread_loop_function_t)(void *ctx, jutil_thread_t *thread_session);

/**
 * @brief Initializes session.
 * 
 * @param function  Function, that will be called in thread loop.
 * @param logger    Logger to use for debug and error messages.
 * @param sleep_ns  How long thread should sleep between loop executions.
 *                  In nanoseconds.
 * @param ctx       Context passed to loop function.
 * 
 * @return          Session object.
 * @return          @c NULL in case of error.
 */
jutil_thread_t *jutil_thread_init(jutil_thread_loop_function_t function, jlog_t *logger, long sleep_ns, void *ctx);

/**
 * @brief Frees memory of session.
 * 
 * Stoppes thread, if still running and frees memory.
 * 
 * @param session Session to free.
 */
void jutil_thread_free(jutil_thread_t *session);

/**
 * @brief Manages background tasks.
 * 
 * Performs cleanup for thread, if necessary.
 * This function should be called periodically.
 * 
 * @param session Session to manage.
 */
void jutil_thread_manage(jutil_thread_t *session);

/**
 * @brief Starts the thread.
 * 
 * @param session Thread to start.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
int jutil_thread_start(jutil_thread_t *session);

/**
 * @brief Safely shuts down thread.
 * 
 * @param session Thread to shutdown.
 */
void jutil_thread_stop(jutil_thread_t *session);

/**
 * @brief Locks the mutex.
 * 
 * Should be used when using data, that
 * is accessed in multiple threads.
 * Mutex should afterwards be unlocked with
 * @c #jcon_thread_unlockMutex() .
 * 
 * @param session Session to lock.
 */
void jutil_thread_lockMutex(jutil_thread_t *session);

/**
 * @brief Unlocks the mutex.
 * 
 * @param session Session to unlock.
 */
void jutil_thread_unlockMutex(jutil_thread_t *session);

/**
 * @brief Check if thread is currently running.
 * 
 * @param session Session to check.
 * 
 * @return        @c true , if thread is running.
 * @return        @c false , if thread is stopped.
 */
int jutil_thread_isRunning(jutil_thread_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_THREAD_H */
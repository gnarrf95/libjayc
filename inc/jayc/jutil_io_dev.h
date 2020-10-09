/**
 * @file jutil_io_dev.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Necessary definitions for implementations of jutil_io.
 * 
 * @date 2020-10-10
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_DEV_IO_H
#define INCLUDE_JUTIL_DEV_IO_H

#include <jayc/jutil_io.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handles print calls.
 * 
 * @param ctx     Session context.
 * @param string  String to print.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
typedef int(*jutil_io_printHandler_t)(void *ctx, const char *string);

/**
 * @brief Handles print line calls.
 * 
 * Prints string and adds newline.
 * 
 * @param ctx     Session context.
 * @param string  String to print.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
typedef int(*jutil_io_printLnHandler_t)(void *ctx, const char *string);

/**
 * @brief Will read string into buffer.
 * 
 * @param ctx       Session context.
 * @param buffer    Buffer for input.
 * @param buf_size  Maximum size of buffer.
 * 
 * @return          Number of bytes read from input.
 */
typedef size_t(*jutil_io_readHandler_t)(void *ctx, char *buffer, size_t buf_size);

/**
 * @brief Will read whole line.
 * 
 * Similar design to @c getline() .
 * @c line_buf points to a string buffer and
 * @c line_size points to a variable telling the
 * size of the buffer. If both @c *line_buf is
 * @c NULL and @c *line_size is @c 0 ,
 * function will allocate a buffer big
 * enough for input and set variable pointed
 * to by @c *line_size to the size of the
 * allocated buffer.
 * 
 * If the buffer gets allocated by the function,
 * the buffer needs to be freed manually.
 * 
 * Newline character will automatically be
 * removed from buffer.
 * 
 * @param ctx       Session context.
 * @param buf_ptr   Pointer to string buffer.
 * @param buf_size  Pointer to string size.
 * 
 * @return          @c true , if successful.
 * @return          @c false , if error occured.
 */
typedef int(*jutil_io_readLnHandler_t)(void *ctx, char **buf_ptr, size_t *buf_size);

/**
 * @brief Frees session context.
 * 
 * @param ctx Session context to free.
 */
typedef void(*jutil_io_freeHandler_t)(void *ctx);

struct __jutil_io_session
{
  jutil_io_printHandler_t function_print;
  jutil_io_printLnHandler_t function_printLine;
  jutil_io_readHandler_t function_read;
  jutil_io_readLnHandler_t function_readLine;

  jutil_io_freeHandler_t session_free_handler;

  void *session_ctx;
};

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_DEV_IO_H */
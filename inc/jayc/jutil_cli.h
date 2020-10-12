/**
 * @file jutil_cli.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Interface for managing CLI input.
 * 
 * @date 2020-10-07
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_CLI_H
#define INCLUDE_JUTIL_CLI_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handler that gets called when CLI input arrives.
 * 
 * @param args      Array of arguments.
 * @param arg_size  Size of array.
 * 
 * @return          NOT USED YET ...
 */
typedef int(*jutil_cli_cmdHandler_t)(const char **args, size_t arg_size, void *ctx);

/**
 * @brief Function to get data for command parsing.
 * 
 * Inspired by @c getline() function.
 * Compared to @c getline() , this function
 * must return string without newline character.
 * 
 * @c *buf_ptr should be set to @c NULL before call.
 * Function should allocate buffer for string and
 * set @c *buf_size accordingly.
 * 
 * @param ctx       Context pointer passed by @c #jutil_cli_run() .
 * @param buf_ptr   Pointer to empty buffer, allocated and set by function.
 * @param buf_size  Size of buffer, set by function.
 * 
 * @return          @c true , if function was successful.
 * @return          @c false , if no input or error occured.
 */
typedef int(*jutil_cli_getInputFunction_t)(void *ctx, char **buf_ptr, size_t *buf_size);

/**
 * @brief Session object.
 */
typedef struct __jutil_cli_session jutil_cli_t;

/**
 * @brief Initializes session.
 * 
 * @param handler         Handler function to be called, when input is recieved.
 * @param input function  Function to get input.
 *                        If @c NULL , gets line input from stdin.
 * @param ctx             Session context pointer.
 * 
 * @return                New session object.
 * @return                @c NULL , if error occured.
 */
jutil_cli_t *jutil_cli_init(jutil_cli_cmdHandler_t handler, jutil_cli_getInputFunction_t input_function, void *ctx);

/**
 * @brief Frees memory of session.
 * 
 * @param session Session object to free.
 */
void jutil_cli_free(jutil_cli_t *session);

/**
 * @brief Read input and parse to handler.
 * 
 * @param session Session object.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
int jutil_cli_run(jutil_cli_t *session);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_CLI_H */
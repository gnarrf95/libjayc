/**
 * @file jproc.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Component for process control.
 * 
 * This component is used to manage the running process.
 * It includes custom exit functions to free global memory
 * and signal catching.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JPROC_H
#define INCLUDE_JPROC_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Define handler types.
//

/**
 * @brief Handles when @c #jproc_exit() is called.
 * 
 * When program has memory, that has to freed,
 * it can be done in this function. So when
 * the program exits by using the @c #jproc_exit()
 * function, this function will be called,
 * before the program is terminated.
 * 
 * @param exit_value  Value to be passed to @c exit() .
 * @param ctx         Context pointer provided by user.
 */
typedef void(*jproc_exit_handler_t)(int exit_value, void *ctx);

/**
 * @brief Handler for catching signals.
 * 
 * When the process recieves a signal
 * (SIGINT, etc.) this handler will be called.
 * 
 * @param signal_number Value of the signal recieved.
 * @param ctx           Context pointer provided by user.
 */
typedef void(*jproc_signal_handler_t)(int signal_number, void *ctx);



//==============================================================================
// Define interface functions.
//

/**
 * @brief Exits the program.
 * 
 * Before exiting, if available, calls handler
 * provided by user.
 * 
 * @param exit_value  Value to pass to @c exit() . 
 */
void jproc_exit(int exit_value);

/**
 * @brief Sets the handler to be called in @c #jproc_exit() .
 * 
 * @param handler Handler function to call.
 * @param ctx     Context pointer to pass to handler.
 * 
 * @return        @c true , if successful.
 * @return        @c false , if error occured.
 */
int jproc_exit_setHandler(jproc_exit_handler_t handler, void *ctx);

/**
 * @brief Adds handler to be called when signal is recieved.
 * 
 * @param signal_number Signal to call handler.
 * @param handler       Handler function.
 * @param ctx           Context pointer to pass to handler.
 * 
 * @return              @c true , if successful.
 * @return              @c false , if error occured.
 */
int jproc_signal_setHandler(int signal_number, jproc_signal_handler_t handler, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JPROC_H */
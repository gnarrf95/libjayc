/**
 * @file jutil_args.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Interface for parsing command line arguments.
 * 
 * CLI options get defined in @c #jutil_args_option_t .
 * Should be created in array and then passed to
 * @c #jutil_args_process() along with size of the
 * array and the CLI variables from @c main() .
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_ARGS_H
#define INCLUDE_JUTIL_ARGS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of parameters a option can have.
 */
#define JUTIL_ARGS_OPTIONPARAM_MAXSIZE 16

/**
 * @brief End of parameter definition.
 */
#define JUTIL_ARGS_OPTIONPARAM_END {NULL,NULL}

/**
 * @brief Defines, that option does not have any parameters.
 */
#define JUTIL_ARGS_OPTIONPARAM_EMPTY {{NULL,NULL}}

/**
 * @brief Function gets called when option is found.
 * 
 * Should perform action with certain argument.
 * If input has an error, function @c #jutil_args_error()
 * should be called and string should be returned.
 * 
 * @param data      Data array provided by command line.
 * @param data_size Size of data array.
 * 
 * @return          @c NULL , if processed correctly.
 * @return          Error string, when input invalid.
 */
typedef char *(*jutil_args_optionHandler_t)(const char **data, size_t data_size);

/**
 * @brief Gives general program information for help command.
 */
typedef struct __jutil_args_progDesc
{
  const char *prog_name;      /**< Name of the program. */
  const char *description;    /**< Detailed description of program. */
  const char *version_string; /**< Version of the program. */

  const char *developer_info; /**< Name and contact of developer. */
  const char *copyright_info; /**< Short copyright text.
                                   Recommended format:
                                   "Copyright (c) 2020 by <developer name>" */
} jutil_args_progDesc_t;

/**
 * @brief Describes parameter for option.
 */
typedef struct __jutil_args_optionParam
{
  const char *name;         /**< Name of the parameter. */
  const char *description;  /**< Description displayed in help. */
} jutil_args_optionParam_t;

/**
 * @brief Describes a command line option.
 * 
 * The system should be provided with an array of these
 * options, to process the CLI input.
 * 
 * If the order of the options is important,
 * then the option array should be provided
 * in order.
 */
typedef struct __jutil_args_option
{
  const char *name;                       /**< Short description of option (few words only). */
  const char *description;                /**< Describes use of option in help. */
  const char *tag_long;                   /**< Long version of option. F.ex. "--option". */
  const char tag_short;                   /**< Short version of option. F.ex. "-o" .*/

  jutil_args_optionHandler_t handler;     /**< Handler to be called with string, when found. */

  int no_tag;                             /**< For arguments without a tag.
                                               If @c true , @c #tag_long , @c #tag_short and
                                               @c #data_required are ignored.
                                               NOT IMPLEMENTED YET! */

  // int data_required;                      /**< How many space seperated arguments should follow
  //                                              this option. */
  int mandatory;                          /**< If @c true , error if not found. */
  int ctr_processed;                      /**< Used by library, set to @c 0 at initialization. */

  const jutil_args_optionParam_t params
    [JUTIL_ARGS_OPTIONPARAM_MAXSIZE];     /**< Parameters required for option.
                                               Last element must be followed
                                               with @c #JUTIL_ARGS_OPTIONPARAM_END. */
} jutil_args_option_t;

/**
 * @brief Input error.
 * 
 * Should be called in @c #jutil_args_optionHandler_t() .
 * When data recieved by handler is not valid, a
 * error message should be created using this function.
 * 
 * The output of this function should be returned by handler.
 * 
 * <b>Note:</b>
 * This function allocates memory for the return, however this
 * is freed by the jutil_args system. Do not free the memory
 * manually!
 * 
 * @param fmt Format string for stdarg.h .
 * 
 * @return    Error string according to input.
 * @return    @c NULL , if error occured.
 */
char *jutil_args_error(const char *fmt, ...);

/**
 * @brief Iterates through the options and processes input.
 * 
 * @param prog_desc   Description of program.
 * @param argc        Number of CLI arguments.
 * @param argv        CLI argument array.
 * @param options     Array of options.
 * @param opt_number  Size of array.
 * 
 * @return            @c true , if processing was successful.
 * @return            @c false , if error occured.
 */
int jutil_args_process(jutil_args_progDesc_t *prog_desc, int argc, char *argv[], jutil_args_option_t *options, size_t opt_number);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_ARGS_H */
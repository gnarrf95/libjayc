/**
 * @file jutil_args.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functionality for jutil_args.
 * 
 * @date 2020-10-02
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jutil_args.h>
#include <jayc/jutil_linkedlist.h>
#include <jayc/jproc.h>
#include <jayc/jlog.h>
#include <jayc/jinfo.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//==============================================================================
// Define structures.
//

/**
 * @brief Created at parsing, afterwards executes options.
 */
typedef struct __jutil_args_processed_option
{
  const char *name;                           /**< Name of the option. */
  char *tag_string;                           /**< Holds tags. Used for error messages.
                                                   "-f/--file"
                                                   "-f"
                                                   "--file" */
                                                   
  jutil_args_optionHandler_t handler;         /**< Handler of option. */

  char *args[JUTIL_ARGS_OPTIONPARAM_MAXSIZE]; /**< Arguments from CLI. */
  size_t arg_size;                            /**< Number of arguments. */
} processedOption_t;

/**
 * @brief Context for parsing process.
 */
typedef struct __jutil_args_context
{
  jutil_args_progDesc_t *prog_desc;           /**< Program description for help message. */

  int argc;                                   /**< Number of input strings. */
  char **argv;                                /**< Input strings. */
  int counter;                                /**< Counter pointing to currently
                                                   processed input string. */

  jutil_args_option_t *options;               /**< Array with options. */
  size_t opt_number;                          /**< Size of option array. */

  jutil_linkedlist_t *processed_list;         /**< Linked list with processed
                                                   options to execute. */
} processContext_t;



//==============================================================================
// Declare internal functions.
//

/**
 * @brief Gets size of option parameter array.
 * 
 * @param params  Parameters to count.
 * 
 * @return        Number of parameters in array.
 */
static size_t optionParam_getSize(const jutil_args_optionParam_t *params);

/**
 * @brief Checks for correctness of option.
 * 
 * @param option  Option to check.
 * 
 * @return        @c true , if option is correct.
 * @return        @c false , if option is invalid.
 */
static int validateOption(jutil_args_option_t *option);

/**
 * @brief Iterates through input and parses it.
 * 
 * @param ctx   Process context.
 * 
 * @return      @c true , if parsing was successful.
 * @return      @c false , if error occured.
 */
static int parseInput(processContext_t *ctx);

/**
 * @brief Processes short tags.
 * 
 * Extracts tag character from CLI argument
 * finds the appropriate option definition
 * and handles data parsing.
 * 
 * Also handles stacked tags (f.ex. "-fdx").
 * 
 * @param ctx Parser context.
 * 
 * @return    @c true , if successful.
 * @return    @c false , if error occured.
 */
static int processShortTag(processContext_t *ctx);

/**
 * @brief Processes long tags.
 * 
 * Extracts tag string from CLI argument
 * finds the appropriate option definition
 * and handles data parsing.
 * 
 * @param ctx Parser context.
 * 
 * @return    @c true , if successful.
 * @return    @c false , if error occured.
 */
static int processLongTag(processContext_t *ctx);

/**
 * @brief Processes option with current counter.
 * 
 * Processes input at current counter with option.
 * Creates processedOption struct and adds it to list.
 * 
 * @param ctx     Processing context.
 * @param option  Option for current input counter.
 * 
 * @return        @c true , if option processed successfully.
 * @return        @c false , if error occured.
 */
static int processOption(processContext_t *ctx, jutil_args_option_t *option);

/**
 * @brief Free struct of processed option.
 * 
 * @param option  Processed option struct.
 */
static void freeProcessedOption(processedOption_t *proc_option);

/**
 * @brief Frees all processed options from linked list.
 * 
 * @param ctx Process context.
 */
static void clearProcessedOptions(processContext_t *ctx);

/**
 * @brief Gets option for tag from array.
 * 
 * Iterates through option array and returns
 * option, that has a matching tag.
 * 
 * @param ctx Parser context.
 * @param tag Tag to look for.
 * 
 * @return    Option pointer matching tag.
 * @return    @c NULL , if no matching option was found.
 */
static jutil_args_option_t *getShortOption(processContext_t *ctx, char tag);

/**
 * @brief Gets option for tag from array.
 * 
 * Iterates through option array and returns
 * option, that has a matching tag.
 * 
 * @param ctx Parser context.
 * @param tag Tag to look for.
 * 
 * @return    Option pointer matching tag.
 * @return    @c NULL , if no matching option was found.
 */
static jutil_args_option_t *getLongOption(processContext_t *ctx, char *tag);

/**
 * @brief Executes handlers with data.
 * 
 * Uses linked list created by process functions.
 * 
 * @param ctx Parser context.
 * 
 * @return    @c true , if successful.
 * @return    @c false , if error occured.
 */
static int executeOptions(processContext_t *ctx);

/**
 * @brief Prints information about how to use arguments.
 * 
 * @param ctx     Parser context.
 * @param output  (stdout/stderr).
 */
static void printUsage(processContext_t *ctx, FILE *output);

/**
 * @brief Prints usage error.
 * 
 * If arguments were not provided in a valid way,
 * prints error message and usage info.
 * 
 * @param ctx Parser context.
 * @param fmt Format string for stdarg.h .
 */
static void printError(processContext_t *ctx, const char *fmt, ...);

/**
 * @brief Prints usage info with additional description.
 * 
 * @param ctx Parser context.
 */
static void printHelp(processContext_t *ctx);

/**
 * @brief prints info about library and program versions.
 * 
 * @param ctx Parser context.
 */
static void printVersionInfo(processContext_t *ctx);

/**
 * @brief Allocate memory and create a string with option tags.
 * 
 * String showing which tags a option has.
 * "-f/--file"
 * "-f"
 * "--file"
 * 
 * @param option  Option from which to create tag string.
 * 
 * @return        Allocated string.
 * @return        @c NULL , if error occured.
 */
static char *createTagString(jutil_args_option_t *option);



//==============================================================================
// Define log macros.
//

#ifdef JUTIL_NO_DEBUG /* Allow to disable debug messages at compile time. */
  #define DEBUG(fmt, ...)
#else
  #define DEBUG(fmt, ...) JLOG_DEBUG(fmt, ##__VA_ARGS__)
#endif
#define INFO(fmt, ...) JLOG_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) JLOG_WARN(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) JLOG_ERROR(fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...)JLOG_CRITICAL(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) JLOG_FATAL(fmt, ##__VA_ARGS__)



//==============================================================================
// Implement interface functions.
//

//------------------------------------------------------------------------------
//
char *jutil_args_error(const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  size_t ret_size = strlen(buf) + 1;
  char *ret = (char *)malloc(sizeof(char) * ret_size);

  memset(ret, 0, ret_size);
  memcpy(ret, buf, ret_size);

  return ret;
}

//------------------------------------------------------------------------------
//
int jutil_args_process(jutil_args_progDesc_t *prog_desc, int argc, char *argv[], jutil_args_option_t *options, size_t opt_number)
{
  /* Check if options available. */
  if(opt_number == 0)
  {
    DEBUG("No options available.");
    return true;
  }

  int check;

  /* Validate options. */
  check = true;
  size_t ctr;
  for(ctr = 0; ctr < opt_number; ctr++)
  {
    if(validateOption(&options[ctr]) == false)
    {
      DEBUG("Option [%lu] invalid.", ctr);
      check = false;
    }
  }

  if(check == false)
  {
    return false;
  }

  /* Create context. */
  DEBUG("Processing [%lu] options.", opt_number);
  processContext_t ctx =
  {
    prog_desc,
    argc,
    argv,
    0,
    options,
    opt_number,
    NULL
  };

  /* Parse input. */
  if(ctx.argc == 1)
  {
    DEBUG("No CLI input available.");
  }
  else
  {
    if(parseInput(&ctx) == false)
    {
      DEBUG("Input parsing failed.");
      return false;
    }
  }

  /* Check if mandatory options were processed. */
  check = true;
  for(ctr = 0; ctr < opt_number; ctr++)
  {
    if(options[ctr].mandatory == true && options[ctr].ctr_processed == 0)
    {
      if(options[ctr].tag_long == NULL)
      {
        printError(&ctx, "Missing tag [-%c].", options[ctr].tag_short);
      }
      else
      {
        printError(&ctx, "Missing tag [--%s].", options[ctr].tag_long);
      }
      check = false;
    }
  }
  
  if(check == false)
  {
    DEBUG("Missing mandatory options.");
    clearProcessedOptions(&ctx);
    return false;
  }

  /* Execute options handlers. */
  return executeOptions(&ctx);
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
size_t optionParam_getSize(const jutil_args_optionParam_t *params)
{
  if(params == NULL)
  {
    return 0;
  }

  /* Check for JUTIL_ARGS_OPTIONPARAM_EMPTY */
  if(params[0].name == NULL && params[0].description == NULL)
  {
    return 0;
  }

  size_t ctr;
  for(ctr = 0; ctr < JUTIL_ARGS_OPTIONPARAM_MAXSIZE; ctr++)
  {
    /* Check for JUTIL_ARGS_OPTIONPARAM_END */
    if(params[ctr].name == NULL && params[ctr].description == NULL)
    {
      return ctr;
    }
  }

  return JUTIL_ARGS_OPTIONPARAM_MAXSIZE;
}

//------------------------------------------------------------------------------
//
int validateOption(jutil_args_option_t *option)
{
  if(option->name == NULL)
  {
    ERROR("Option needs a name.");
    return false;
  }

  if(option->tag_long == NULL && option->tag_short == 0)
  {
    ERROR("Options without tags not implemented yet.");
    return false;
  }

  if(option->handler == NULL)
  {
    ERROR("Option needs a handler.");
    return false;
  }

  if(option->tag_short == 'h' || option->tag_short == 'v')
  {
    ERROR("Tag [-%c] already used by jutil_args.", option->tag_short);
    return false;
  }

  if(option->tag_long)
  {
    if(strcmp(option->tag_long, "help") == 0 || strcmp(option->tag_long, "version") == 0)
    {
      ERROR("Tag [--%s] already used by jutil_args.", option->tag_long);
      return false;
    }
  }

  size_t size_param = optionParam_getSize(option->params);
  size_t ctr_param;
  for
  (
    ctr_param = 0;
    ctr_param < size_param;
    ctr_param++
  )
  {
    if(option->params[ctr_param].name == NULL)
    {
      ERROR("Option parameter needs a name.");
      return false;
    }
  }

  option->ctr_processed = 0;

  return true;
}

//------------------------------------------------------------------------------
//
int parseInput(processContext_t *ctx)
{
  int check = true;

  for(ctx->counter = 1; ctx->counter < ctx->argc; ctx->counter++)
  {
    char *arg_string = ctx->argv[ctx->counter];

    if(arg_string[0] != '-')
    {
      /* Options without tags not implemented yet. */
      printError(ctx, "Invalid tag [%s].", arg_string);
      check = false;
      break;
    }

    if(arg_string[1] == '-')
    {
      /* Process long tag options. */
      if(processLongTag(ctx) == false)
      {
        DEBUG("Failed processing long tag [ctr = %lu].", ctx->counter);
        check = false;
        break;
      }
    }
    else
    {
      /* Process short tag options. */
      if(processShortTag(ctx) == false)
      {
        DEBUG("Failed processing short tag [ctr = %lu].", ctx->counter);
        check = false;
        break;
      }
    }
  }

  if(check == false)
  {
    clearProcessedOptions(ctx);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
int processShortTag(processContext_t *ctx)
{
  char *tag_string = ctx->argv[ctx->counter];
  size_t tag_number = strlen(tag_string) - 1;

  size_t ctr;

  /* Iterate through tag characters (allows for stacked tags). */
  for(ctr = 1; ctr < tag_number+1; ctr++)
  {
    if(tag_string[ctr] == 'h')
    {
      printHelp(ctx);
      return false;
    }
    if(tag_string[ctr] == 'v')
    {
      printVersionInfo(ctx);
      return false;
    }

    jutil_args_option_t *option = getShortOption(ctx, tag_string[ctr]);
    if(option == NULL)
    {
      printError(ctx, "Invalid tag [-%c].", tag_string[ctr]);
      return false;
    }

    if(tag_number > 1 && optionParam_getSize(option->params))
    {
      printError(ctx, "Tags requiring arguments cannot be stacked (-%c).", tag_string[ctr]);
      return false;
    }

    if(processOption(ctx, option) == false)
    {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------
//
int processLongTag(processContext_t *ctx)
{
  size_t tag_size = strlen(ctx->argv[ctx->counter]) - 2;
  
  char *tag = (char *)malloc(sizeof(char) * tag_size+1);
  if(tag == NULL)
  {
    ERROR("malloc() failed.");
    return false;
  }

  memset(tag, 0, tag_size+1);
  memcpy(tag, &(ctx->argv[ctx->counter][2]), tag_size);

  /* Check if help-tag recieved. */
  if(strcmp(tag, "help") == 0)
  {
    printHelp(ctx);
    free(tag);
    return false;
  }

  /* Check if version-tag recieved. */
  if(strcmp(tag, "version") == 0)
  {
    printVersionInfo(ctx);
    free(tag);
    return false;
  }

  /* Get option and process it. */
  jutil_args_option_t *option = getLongOption(ctx, tag);
  if(option == NULL)
  {
    printError(ctx, "Invalid tag [--%s].", tag);
    free(tag);
    return false;
  }
  free(tag);

  return processOption(ctx, option);
}

//------------------------------------------------------------------------------
//
int processOption(processContext_t *ctx, jutil_args_option_t *option)
{
  /* Create pointer for processed option. */
  processedOption_t *processed_option = (processedOption_t *)malloc(sizeof(processedOption_t));
  if(processed_option == NULL)
  {
    ERROR("malloc() failed [%s].", option->name);
    return false;
  }

  /* Copy data. */
  processed_option->name = option->name;
  processed_option->handler = option->handler;

  /* Read arguments if necessary. */
  processed_option->arg_size = optionParam_getSize(option->params);

  int check = true;
  size_t ctr;
  for(ctr = 0; ctr < processed_option->arg_size; ctr++)
  {
    ctx->counter++;
    if(ctx->counter >= ctx->argc)
    {
      printError(ctx, "[%s] Missing arguments.", processed_option->tag_string);
      check = false;
    }

    processed_option->args[ctr] = ctx->argv[ctx->counter];
  }

  if(check == false)
  {
    free(processed_option);
    return false;
  }

  /* Create tag_string. */
  processed_option->tag_string = createTagString(option);
  if(processed_option->tag_string == NULL)
  {
    free(processed_option);
    return false;
  }

  if(jutil_linkedlist_append(&ctx->processed_list, (void *)processed_option) == false)
  {
    ERROR("jutil_linkedlist_push() failed.");
    freeProcessedOption(processed_option);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void freeProcessedOption(processedOption_t *proc_option)
{
  free(proc_option->tag_string);
  free(proc_option);
}

//------------------------------------------------------------------------------
//
void clearProcessedOptions(processContext_t *ctx)
{
  jutil_linkedlist_t *proc_opts = ctx->processed_list;

  while(proc_opts != NULL)
  {
    processedOption_t *opt = (processedOption_t *)jutil_linkedlist_pop(&proc_opts);
    if(opt)
    {
      freeProcessedOption(opt);
    }
  }
}

//------------------------------------------------------------------------------
//
jutil_args_option_t *getShortOption(processContext_t *ctx, char tag)
{
  size_t ctr;
  for(ctr = 0; ctr < ctx->opt_number; ctr++)
  {
    if(ctx->options[ctr].tag_short == tag)
    {
      return &(ctx->options[ctr]);
    }
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
jutil_args_option_t *getLongOption(processContext_t *ctx, char *tag)
{
  size_t ctr;
  for(ctr = 0; ctr < ctx->opt_number; ctr++)
  {
    if(ctx->options[ctr].tag_long == NULL)
    {
      continue;
    }

    if(strcmp(ctx->options[ctr].tag_long, tag) == 0)
    {
      return &(ctx->options[ctr]);
    }
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
int executeOptions(processContext_t *ctx)
{
  int fail = false;

  jutil_linkedlist_t *opt_itr = ctx->processed_list;

  while(opt_itr != NULL)
  {
    processedOption_t *opt = (processedOption_t *)jutil_linkedlist_getData(opt_itr);

    char *ret_handler = opt->handler((const char **)opt->args, opt->arg_size);
    if(ret_handler)
    {
      printError(ctx, "[%s] %s", opt->tag_string, ret_handler);
      free(ret_handler);
      fail = true;
      break;
    }

    opt_itr = jutil_linkedlist_iterate(opt_itr);
  }

  clearProcessedOptions(ctx);
  if(fail)
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
//
void printHelp(processContext_t *ctx)
{
  printf("## [PROGRAM] ##\n");
  printf("  %s (%s)\n\n", ctx->prog_desc->prog_name, ctx->prog_desc->version_string);

  printf("## [DESCRIPTION] ##\n");
  printf("  %s\n\n", ctx->prog_desc->description);

  printUsage(ctx, stdout);
  printf("\n");

  printf("## [OPTIONS] ##\n\n");
  size_t ctr_op;
  for(ctr_op = 0; ctr_op < ctx->opt_number; ctr_op++)
  {
    jutil_args_option_t op = ctx->options[ctr_op];
    size_t size_par = optionParam_getSize(op.params);
    size_t ctr_par;

    /* Option description. */
    printf("# %s", op.name);
    if(!op.mandatory)
    {
      printf(" (OPTIONAL)");
    }
    printf(" :\n");

    printf("  %s\n\n", op.description);

    /* Option syntax. */
    if(op.tag_short == 0 && op.tag_long == NULL)
    {
      // Not yet supported.
    }
    else if(op.tag_long == NULL)
    {
      printf("  USAGE: -%c", op.tag_short);
    }
    else if(op.tag_short == 0)
    {
      printf("  USAGE: --%s", op.tag_long);
    }
    else
    {
      printf("  USAGE: --%s/-%c", op.tag_long, op.tag_short);
    }

    for(ctr_par = 0; ctr_par < size_par; ctr_par++)
    {
      printf(" <%s>", op.params[ctr_par].name);
    }
    printf("\n\n");

    /* Parameter description. */
    if(size_par)
    {
      printf("  PARAMETERS:\n");
      for(ctr_par = 0; ctr_par < size_par; ctr_par++)
      {
        printf("  * %s : %s\n", op.params[ctr_par].name, op.params[ctr_par].description);
      }
      printf("\n");
    }
  }

  printf("## [COPYRIGHT] ##\n");
  printf("  DEVELOPER: %s\n", ctx->prog_desc->developer_info);
  printf("  %s\n\n", ctx->prog_desc->copyright_info);
}

//------------------------------------------------------------------------------
//
void printUsage(processContext_t *ctx, FILE *output)
{
  /* Program called. */
  fprintf(output, "  USAGE: %s ", ctx->argv[0]);

  size_t ctr;

  for(ctr = 0; ctr < ctx->opt_number; ctr++)
  {
    jutil_args_option_t op = ctx->options[ctr];

    if(op.tag_short == 0 && op.tag_long == NULL)
    {
      // Not yet supported.
      continue;
    }

    if(!op.mandatory)
    {
      fprintf(output, "[");
    }

    if(op.tag_short == 0)
    {
      fprintf(output, "--%s", op.tag_long);
    }
    else
    {
      fprintf(output, "-%c", op.tag_short);
    }

    if(optionParam_getSize(op.params))
    {
      fprintf(output, " ...");
    }

    if(!op.mandatory)
    {
      fprintf(output, "]");
    }
    fprintf(output, " ");
  }

  fprintf(output, "\n");
}

//------------------------------------------------------------------------------
//
void printError(processContext_t *ctx, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  fprintf(stderr, "[ ERROR ] %s\n", buf);
  printUsage(ctx, stderr);
  fprintf(stderr, "\n  Use [-h / --help], to get more info.\n");

}

//------------------------------------------------------------------------------
//
void printVersionInfo(processContext_t *ctx)
{
  printf("## [PROGRAM VERSION] ##\n");
  printf("  %s %s\n\n", ctx->prog_desc->prog_name, ctx->prog_desc->version_string);

  printf("## [LIBRARY VERSION] ##\n");
  printf("  %s\n", jinfo_build_version());
  printf("  BUILT WITH %s ON %s\n\n", jinfo_build_compiler(), jinfo_build_platform());
}

//------------------------------------------------------------------------------
//
char *createTagString(jutil_args_option_t *option)
{
  size_t str_len;

  if(option->tag_long && option->tag_short)
  {
    str_len = strlen(option->tag_long) + 5;
  }
  else if(option->tag_long)
  {
    str_len = strlen(option->tag_long) + 2;
  }
  else if(option->tag_short)
  {
    str_len = 2;
  }
  else
  {
    ERROR("Option without tag not supported.");
    return NULL;
  }

  char *tag_str = (char *)malloc(sizeof(char) * (str_len + 1));
  if(tag_str == NULL)
  {
    ERROR("malloc() failed.");
    return NULL;
  }

  if(option->tag_long && option->tag_short)
  {
    sprintf(tag_str, "-%c/--%s", option->tag_short, option->tag_long);
  }
  else if(option->tag_long)
  {
    sprintf(tag_str, "--%s", option->tag_long);
  }
  else
  {
    sprintf(tag_str, "-%c", option->tag_short);
  }

  return tag_str;
}
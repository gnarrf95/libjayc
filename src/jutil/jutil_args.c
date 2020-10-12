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
#include <jayc/jproc.h>
#include <jayc/jlog.h>
#include <jayc/jinfo.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//==============================================================================
// Define context structure and log macros.
//

typedef struct __jutil_args_context
{
  jutil_args_progDesc_t *prog_desc;

  int argc;
  char **argv;
  int counter;

  jutil_args_option_t *options;
  size_t opt_number;
} jutil_args_ctx_t;

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
// Declare internal funcions.
//

/**
 * @brief Gets size of option parameter array.
 * 
 * @param params  Parameters to count.
 * 
 * @return        Number of parameters in array.
 */
static size_t jutil_args_optionParam_getSize(const jutil_args_optionParam_t *params);

/**
 * @brief Checks for correctness of option.
 * 
 * @param option  Option to check.
 * 
 * @return        @c true , if option is correct.
 * @return        @c false , if option is invalid.
 */
static int jutil_args_validateOption(jutil_args_option_t option);

/**
 * @brief Processes short tags.
 * 
 * Extracts tag character from CLI argument
 * finds the appropriate option definition
 * and handles data parsing.
 * 
 * @param ctx Parser context.
 * 
 * @return    @c true , if successful.
 * @return    @c false , if error occured.
 */
static int jutil_args_processShortTag(jutil_args_ctx_t *ctx);

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
static int jutil_args_processLongTag(jutil_args_ctx_t *ctx);

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
static jutil_args_option_t *jutil_args_getShortOption(jutil_args_ctx_t *ctx, char tag);

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
static jutil_args_option_t *jutil_args_getLongOption(jutil_args_ctx_t *ctx, char *tag);

/**
 * @brief Prints information about how to use arguments.
 * 
 * @param ctx     Parser context.
 * @param output  (stdout/stderr).
 */
static void jutil_args_printUsage(jutil_args_ctx_t *ctx, FILE *output);

/**
 * @brief Prints usage error.
 * 
 * If arguments were not provided in a valid way,
 * prints error message and usage info.
 * 
 * @param ctx Parser context.
 * @param fmt Format string for stdarg.h .
 */
static void jutil_args_printError(jutil_args_ctx_t *ctx, const char *fmt, ...);

/**
 * @brief Prints usage info with additional description.
 * 
 * @param ctx Parser context.
 */
static void jutil_args_printHelp(jutil_args_ctx_t *ctx);

/**
 * @brief prints info about library and program versions.
 * 
 * @param ctx Parser context.
 */
static void jutil_args_printVersionInfo(jutil_args_ctx_t *ctx);



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

  DEBUG("Processing [%lu] options.", opt_number);

  size_t ctr;

  /* Check if all options are valid. */
  for(ctr = 0; ctr < opt_number; ctr++)
  {
    if(jutil_args_validateOption(options[ctr]) == false)
    {
      DEBUG("Invalid option [ctr = %d].", ctr);
      return false;
    }
  }

  /* Context to pass to sub-functions. */
  jutil_args_ctx_t context =
  {
    prog_desc,
    argc,
    argv,
    0,
    options,
    opt_number
  };
  /* Check if CLI arguments available. */
  if(argc == 1)
  {
    DEBUG("No CLI arguments available.");
  }
  else
  {
    /* Iterate through all options and process them. */
    for(context.counter = 1; context.counter < context.argc; context.counter++)
    {
      char *arg = context.argv[context.counter];

      if(arg[0] == '-')
      {
        if(arg[1] == '-')
        {
          /* Process long tag options. */
          if(jutil_args_processLongTag(&context) == false)
          {
            DEBUG("Failed processing long tag [ctr = %d].", context.counter);
            return false;
          }
        }
        else
        {
        /* Process long tag options. */
          if(jutil_args_processShortTag(&context) == false)
          {
            DEBUG("Failed processing short tag [ctr = %d].", context.counter);
            return false;
          }
        }
      }
      else
      {
        /* Options without tags not implemented yet. */
        jutil_args_printError(&context, "Invalid tag [%s].", arg);
        return false;
      }
    }
  }

  /* Check if mandatory options were processed. */
  bool ret = true;
  for(ctr = 0; ctr < opt_number; ctr++)
  {
    if(options[ctr].mandatory == true && options[ctr].ctr_processed == 0)
    {
      if(options[ctr].tag_long == NULL)
      {
        jutil_args_printError(&context, "Missing tag [-%c].", options[ctr].tag_short);
      }
      else
      {
        jutil_args_printError(&context, "Missing tag [--%s].", options[ctr].tag_long);
      }
      ret = false;
    }
  }

  return ret;
}



//==============================================================================
// Implement internal functions.
//

//------------------------------------------------------------------------------
//
size_t jutil_args_optionParam_getSize(const jutil_args_optionParam_t *params)
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
int jutil_args_validateOption(jutil_args_option_t option)
{
  if(option.name == NULL)
  {
    ERROR("Option needs a name.");
    return false;
  }

  if(option.tag_long == NULL && option.tag_short == 0)
  {
    ERROR("Options without tags not implemented yet.");
    return false;
  }

  if(option.handler == NULL)
  {
    ERROR("Option needs a handler.");
    return false;
  }

  if(option.tag_short == 'h' || option.tag_short == 'v')
  {
    ERROR("Tag [-%c] already used by jutil_args.", option.tag_short);
    return false;
  }

  if(option.tag_long)
  {
    if(strcmp(option.tag_long, "help") == 0 || strcmp(option.tag_long, "version") == 0)
    {
      ERROR("Tag [--%s] already used by jutil_args.", option.tag_long);
      return false;
    }
  }

  size_t size_param = jutil_args_optionParam_getSize(option.params);
  size_t ctr_param;
  for
  (
    ctr_param = 0;
    ctr_param < size_param;
    ctr_param++
  )
  {
    if(option.params[ctr_param].name == NULL)
    {
      ERROR("Option parameter needs a name.");
      return false;
    }
  }

  /* For processing ctr_processed needs to be 0. */
  option.ctr_processed = 0;

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_args_processShortTag(jutil_args_ctx_t *ctx)
{
  /* Check validity of tag. */
  if(strlen(ctx->argv[ctx->counter]) > 2)
  {
    jutil_args_printError(ctx, "Invalid tag [%s].", ctx->argv[ctx->counter]);
    return false;
  }

  char tag = ctx->argv[ctx->counter][1];

  /* Check if help-tag recieved. */
  if(tag == 'h')
  {
    jutil_args_printHelp(ctx);
    return false;
  }

  /* Check if version-tag recieved. */
  if(tag == 'v')
  {
    jutil_args_printVersionInfo(ctx);
    return false;
  }

  jutil_args_option_t *option = jutil_args_getShortOption(ctx, tag);
  if(option == NULL)
  {
    jutil_args_printError(ctx, "Invalid tag [-%c].", tag);
    return false;
  }

  char **data = NULL;
  size_t data_size = jutil_args_optionParam_getSize(option->params);

  /* Read additional arguments for option. */
  if(data_size)
  {
    data = (char **)malloc(sizeof(char *) * data_size);
    if(data == NULL)
    {
      ERROR("malloc failed() [tag = -%c].", tag);
      return false;
    }

    size_t ctr_data;
    for(ctr_data = 0; ctr_data < data_size; ctr_data++)
    {
      ctx->counter++;
      if(ctx->counter >= ctx->argc)
      {
        jutil_args_printError(ctx, "Missing arguments for tag [-%c].", tag);
        free(data);
        return false;
      }

      data[ctr_data] = ctx->argv[ctx->counter];
    }
  }

  /* Call option handler. */
  char *ret_handler = option->handler((const char **)data, data_size);
  free(data);
  if(ret_handler)
  {
    jutil_args_printError(ctx, (const char *)ret_handler);
    free(ret_handler);
    return false;
  }

  option->ctr_processed++;
  return true;
}

//------------------------------------------------------------------------------
//
int jutil_args_processLongTag(jutil_args_ctx_t *ctx)
{
  size_t tag_size = strlen(ctx->argv[ctx->counter]) - 2;
  if(tag_size > 128)
  {
    jutil_args_printError(ctx, "Invalid tag [%s].", ctx->argv[ctx->counter]);
    return false;
  }

  char tag[128] = { 0 };
  memcpy(tag, &(ctx->argv[ctx->counter][2]), tag_size);

  /* Check if help-tag recieved. */
  if(strcmp(tag, "help") == 0)
  {
    jutil_args_printHelp(ctx);
    return false;
  }

  /* Check if version-tag recieved. */
  if(strcmp(tag, "version") == 0)
  {
    jutil_args_printVersionInfo(ctx);
    return false;
  }

  jutil_args_option_t *option = jutil_args_getLongOption(ctx, tag);
  if(option == NULL)
  {
    jutil_args_printError(ctx, "Invalid tag [--%s].", tag);
    return false;
  }

  char **data = NULL;
  size_t data_size = jutil_args_optionParam_getSize(option->params);

  /* Read additional arguments for option. */
  if(data_size)
  {
    data = (char **)malloc(sizeof(char *) * data_size);
    if(data == NULL)
    {
      ERROR("malloc failed() [tag = --%s].", tag);
      return false;
    }

    size_t ctr_data;
    for(ctr_data = 0; ctr_data < data_size; ctr_data++)
    {
      ctx->counter++;
      if(ctx->counter >= ctx->argc)
      {
        jutil_args_printError(ctx, "Missing arguments for tag [--%s].", tag);
        return false;
      }

      data[ctr_data] = ctx->argv[ctx->counter];
    }
  }

  /* Call option handler. */
  char *ret_handler = option->handler((const char **)data, data_size);
  free(data);
  if(ret_handler)
  {
    jutil_args_printError(ctx, (const char *)ret_handler);
    free(ret_handler);
    return false;
  }

  option->ctr_processed++;
  return true;
}

//------------------------------------------------------------------------------
//
jutil_args_option_t *jutil_args_getShortOption(jutil_args_ctx_t *ctx, char tag)
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
jutil_args_option_t *jutil_args_getLongOption(jutil_args_ctx_t *ctx, char *tag)
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
void jutil_args_printHelp(jutil_args_ctx_t *ctx)
{
  printf("## [Program] ##\n");
  printf("%s (%s)\n\n", ctx->prog_desc->prog_name, ctx->prog_desc->version_string);

  printf("## [Description] ##\n");
  printf("%s\n\n", ctx->prog_desc->description);

  jutil_args_printUsage(ctx, stdout);
  printf("\n");

  printf("## [Options] ##\n\n");
  size_t ctr_op;
  for(ctr_op = 0; ctr_op < ctx->opt_number; ctr_op++)
  {
    jutil_args_option_t op = ctx->options[ctr_op];
    size_t size_par = jutil_args_optionParam_getSize(op.params);
    size_t ctr_par;

    /* Option description. */
    printf("# %s", op.name);
    if(!op.mandatory)
    {
      printf(" (optional)");
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
      printf("  Usage: -%c", op.tag_short);
    }
    else if(op.tag_short == 0)
    {
      printf("  Usage: --%s", op.tag_long);
    }
    else
    {
      printf("  Usage: --%s/-%c", op.tag_long, op.tag_short);
    }

    for(ctr_par = 0; ctr_par < size_par; ctr_par++)
    {
      printf(" <%s>", op.params[ctr_par].name);
    }
    printf("\n\n");

    /* Parameter description. */
    if(size_par)
    {
      printf("  Parameters:\n");
      for(ctr_par = 0; ctr_par < size_par; ctr_par++)
      {
        printf("  - %s : %s\n", op.params[ctr_par].name, op.params[ctr_par].description);
      }
      printf("\n");
    }
  }

  printf("## [Copyright Notest] ##\n");
  printf("Developer: %s\n", ctx->prog_desc->developer_info);
  printf("%s\n\n", ctx->prog_desc->copyright_info);
}

//------------------------------------------------------------------------------
//
void jutil_args_printUsage(jutil_args_ctx_t *ctx, FILE *output)
{
  /* Program called. */
  fprintf(output, "Usage: %s ", ctx->argv[0]);

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

    if(jutil_args_optionParam_getSize(op.params))
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
void jutil_args_printError(jutil_args_ctx_t *ctx, const char *fmt, ...)
{
  va_list args;
  char buf[2048];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  fprintf(stderr, "[ ERROR ] %s\n", buf);
  jutil_args_printUsage(ctx, stderr);
  fprintf(stderr, "\nUse [-h / --help], to get more info.\n");

}

//------------------------------------------------------------------------------
//
void jutil_args_printVersionInfo(jutil_args_ctx_t *ctx)
{
  printf("## [Program Version] ##\n");
  printf("%s %s\n\n", ctx->prog_desc->prog_name, ctx->prog_desc->version_string);

  printf("## [Library Version] ##\n");
  printf("%s\n", jinfo_build_version());
  printf("Build with %s on %s\n\n", jinfo_build_compiler(), jinfo_build_platform());
}
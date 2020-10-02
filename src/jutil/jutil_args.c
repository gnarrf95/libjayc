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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct __jutil_args_context
{
  char *prog_name;

  int argc;
  char **argv;
  int counter;

  jutil_args_option_t *options;
  size_t opt_number;
} jutil_args_ctx_t;

/**
 * @brief Checks for correctness of option.
 * 
 * @param option  Option to check.
 * 
 * @return        @c true , if option is correct.
 * @return        @c false , if option is invalid.
 */
static int jutil_args_validateOption(jutil_args_option_t option);

static int jutil_args_processShortTag(jutil_args_ctx_t *ctx);
static int jutil_args_processLongTag(jutil_args_ctx_t *ctx);

static jutil_args_option_t *jutil_args_getShortOption(jutil_args_ctx_t *ctx, char tag);
static jutil_args_option_t *jutil_args_getLongOption(jutil_args_ctx_t *ctx, char *tag);

static void jutil_args_printUsage(jutil_args_ctx_t *ctx, FILE *output);
static void jutil_args_printError(jutil_args_ctx_t *ctx, const char *fmt, ...);
static void jutil_args_printHelp(jutil_args_ctx_t *ctx);

#ifdef JUTIL_NO_DEBUG /* Allow to disable debug messages at compile time. */
  #define DEBUG(fmt, ...)
#else
  #define DEBUG(fmt, ...) JLOG_DEBUG(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#endif
#define INFO(fmt, ...) JLOG_INFO(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) JLOG_WARN(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) JLOG_ERROR(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...)JLOG_CRITICAL(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) JLOG_FATAL(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

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
int jutil_args_process(int argc, char *argv[], jutil_args_option_t *options, size_t opt_number)
{
  /* Check if options available. */
  if(opt_number == 0)
  {
    DEBUG("No options available.");
    return false;
  }

  /* Check if CLI arguments available. */
  if(argc == 1)
  {
    DEBUG("No CLI arguments available.");
    return false;
  }

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
    argv[0],
    argc,
    argv,
    0,
    options,
    opt_number
  };

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

  jutil_args_option_t *option = jutil_args_getShortOption(ctx, tag);
  if(option == NULL)
  {
    jutil_args_printError(ctx, "Invalid tag [-%c].", tag);
    return false;
  }

  char **data = NULL;
  size_t data_size = 0;

  /* Read additional arguments for option. */
  if(option->data_required)
  {
    data_size = option->data_required;
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
    return true;
  }

  jutil_args_option_t *option = jutil_args_getLongOption(ctx, tag);
  if(option == NULL)
  {
    jutil_args_printError(ctx, "Invalid tag [--%s].", tag);
    return false;
  }

  char **data = NULL;
  size_t data_size = 0;

  /* Read additional arguments for option. */
  if(option->data_required)
  {
    data_size = option->data_required;
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
  jutil_args_printUsage(ctx, stdout);
}

//------------------------------------------------------------------------------
//
void jutil_args_printUsage(jutil_args_ctx_t *ctx, FILE *output)
{
  char *prog_name = ctx->argv[0];
  
  fprintf(output, "Usage: %s\n", prog_name);

  size_t ctr;
  for(ctr = 0; ctr < ctx->opt_number; ctr++)
  {
    jutil_args_option_t op = ctx->options[ctr];

    fprintf(output, "       [");

    if(op.tag_long && op.tag_short)
    {
      fprintf(output, "-%c/--%s]", op.tag_short, op.tag_long);
    }
    else if(op.tag_long)
    {
      fprintf(output, "--%s]", op.tag_long);
    }
    else
    {
      fprintf(output, "-%c]", op.tag_short);
    }

    size_t ctr_args;
    for(ctr_args = 0; ctr_args < op.data_required; ctr_args++)
    {
      fprintf(output, " <value-%lu>", ctr_args+1);
    }

    fprintf(output, "\n");
  }
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
/**
 * @file jsys_test.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief This is a test daemon system.
 * 
 * Based on jcon_system with jcon_server_tcp
 * it handles tcp connections, recieved messages
 * get generated into a hash and returned to client.
 * 
 * @date 2020-09-29
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#define _POSIX_C_SOURCE 199309L /* needed for nanosleep() */

#include <jayc/jlog.h>
#include <jayc/jlog_stdio.h>
#include <jayc/jlog_syslog.h>
#include <jayc/jcon_system.h>
#include <jayc/jcon_server_tcp.h>
#include <jayc/jcon_client.h>
#include <jayc/jutil_crypto.h>
#include <jayc/jutil_args.h>
#include <jayc/jproc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>

#include <stdio.h>

#define JSYS_DEFAULT_IP       "127.0.0.1"
#define JSYS_DEFAULT_PORT     1234
#define JSYS_DEFAULT_HASHCODE 1

#define JSYS_HASHCODE_NONE    0
#define JSYS_HASHCODE_MD5     1
#define JSYS_HASHCODE_SHA256  2
#define JSYS_HASHCODE_SHA512  3

typedef struct __jsys_data
{
  char address[64];
  uint16_t port;
  int hash_code; // 0->none, 1->md5, 2->sha256, 3->sha512

  jcon_system_t *system;
  jcon_server_t *server;
  jlog_t *logger;

  int run;
} jsys_t;

static char *argHandler_syslog(const char **data, size_t data_size);
static char *argHandler_ip(const char **data, size_t data_size);
static char *argHandler_port(const char **data, size_t data_size);
static char *argHandler_hashcode(const char **data, size_t data_size);

static jutil_args_progDesc_t prog_desc =
{
  "jsys_test",

  "Test program to checkout jcon_system. Creates a TCP server " \
  "and handles every new connection in a thread. When a message " \
  "is recieved, the server responds with the hashed message.",

  "-TEST-",
  "Manuel Nadji (https://github.com/gnarrf95)",
  "Copyright (c) 2020 by Manuel Nadji"
};

static jutil_args_option_t options[] =
{
  {
    "jlog_syslog",
    "System will use jlog_syslog to log information.",
    "syslog",
    0,
    &argHandler_syslog,
    0,
    0,
    0,
    {
      {
        "facility",
        "Syslog facility used (supports \"daemon\" and \"user\"."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  },
  {
    "Server address",
    "Address the server should bind to.",
    "address",
    'a',
    &argHandler_ip,
    0,
    0,
    0,
    {
      {
        "server-address",
        "IP/DNS address for server to use."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  },
  {
    "Server port",
    "Port number the server should bind to.",
    "port",
    'p',
    &argHandler_port,
    0,
    0,
    0,
    {
      {
        "server-port",
        "Port for server to use."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  },
  {
    "Hash Code",
    "Defines the hash algorithm that should be used.",
    "hash",
    0,
    &argHandler_hashcode,
    0,
    0,
    0,
    {
      {
        "hash-code",
        "Algorithm reference (0->NONE, 1->MD5, 2->SHA256, 3->SHA512)."
      },
      JUTIL_ARGS_OPTIONPARAM_END
    }
  }
};

static jsys_t g_data = 
{
  JSYS_DEFAULT_IP,
  JSYS_DEFAULT_PORT,
  JSYS_DEFAULT_HASHCODE,
  NULL,
  NULL,
  NULL,
  1
};

static void jsys_signalHandler(int dummy, void *ctx);

static void jsys_dataHandler(void *ctx, jcon_client_t *client);

static void jsys_createHandler(void *ctx, const char *ref_string);

static void jsys_closeHandler(void *ctx, const char *ref_string);

static void jsys_cleanup(int exit_value, void *ctx);

//------------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
  jproc_exit_setHandler(jsys_cleanup, NULL);
  jproc_signal_setHandler(SIGINT, jsys_signalHandler, NULL);

  if(jutil_args_process
    (
      &prog_desc,
      argc,
      argv,
      (jutil_args_option_t *)options,
      sizeof(options)/sizeof(jutil_args_option_t)
    ) == 0)
  {
    jproc_exit(EXIT_FAILURE);
  }

  if(g_data.logger == NULL)
  {
    g_data.logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
    if(g_data.logger == NULL)
    {
      jproc_exit(EXIT_FAILURE);
    }
  }

  jlog_global_session_set(g_data.logger);

  g_data.server = jcon_server_tcp_session_init(g_data.address, g_data.port, g_data.logger);
  if(g_data.server == NULL)
  {
    jproc_exit(EXIT_FAILURE);
  }

  g_data.system = jcon_system_init
  (
    g_data.server,
    jsys_dataHandler,
    jsys_createHandler,
    jsys_closeHandler,
    g_data.logger,
    (void *)&g_data
  );
  if(g_data.system == NULL)
  {
    jproc_exit(EXIT_FAILURE);
  }

  struct timespec sleep_time;
  sleep_time.tv_sec = 1;
  sleep_time.tv_nsec = 0;

  while(g_data.run)
  {
    nanosleep(&sleep_time, NULL);
  }

  jproc_exit(EXIT_SUCCESS);
}

//------------------------------------------------------------------------------
//
void jsys_signalHandler(int dummy, void *ctx)
{
  JLOG_DEBUG("Shutdown signal recieved.");
  g_data.run = 0;
}

//------------------------------------------------------------------------------
//
void jsys_dataHandler(void *ctx, jcon_client_t *client)
{
  char hash_buf[256] = { 0 };
  char msg_buf[2048] = { 0 };

  if(jcon_client_recvData(client, msg_buf, sizeof(msg_buf)) == 0)
  {
    JLOG_WARN("jcon_client_recvData() failed for client [%s].", jcon_client_getReferenceString(client));
    return;
  }

  switch(g_data.hash_code)
  {
    case JSYS_HASHCODE_NONE:
    {
      memcpy(hash_buf, msg_buf, sizeof(hash_buf));
      break;
    }
    case JSYS_HASHCODE_MD5:
    {
      jutil_crypto_md5_str(msg_buf, strlen(msg_buf), hash_buf);
      break;
    }
    case JSYS_HASHCODE_SHA256:
    {
      jutil_crypto_sha256_str(msg_buf, strlen(msg_buf), hash_buf);
      break;
    }
    case JSYS_HASHCODE_SHA512:
    {
      jutil_crypto_sha512_str(msg_buf, strlen(msg_buf), hash_buf);
      break;
    }

    default:
    {
      JLOG_WARN("Invalid hash code [%d].", g_data.hash_code);
      return;
    }
  }

  hash_buf[sizeof(hash_buf)-1] = 0;

  if(jcon_client_sendData(client, hash_buf, strlen(hash_buf)) == 0)
  {
    JLOG_WARN("jcon_client_sendData() failed for client [%s].", jcon_client_getReferenceString(client));
  }

  JLOG_INFO("Handled session with [%s] : [%s - %s].",
    jcon_client_getReferenceString(client),
    msg_buf,
    hash_buf);
}

//------------------------------------------------------------------------------
//
void jsys_createHandler(void *ctx, const char *ref_string)
{

}

//------------------------------------------------------------------------------
//
void jsys_closeHandler(void *ctx, const char *ref_string)
{

}

//------------------------------------------------------------------------------
//
void jsys_cleanup(int exit_value, void *ctx)
{
  if(g_data.system)
  {
    jcon_system_free(g_data.system);
  }

  if(g_data.server)
  {
    jcon_server_free(g_data.server);
  }
}

//------------------------------------------------------------------------------
//
char *argHandler_syslog(const char **data, size_t data_size)
{
  int fac;
  if(strcmp(data[0], "user") == 0)
  {
    fac = LOG_USER;
  }
  else if(strcmp(data[0], "daemon") == 0)
  {
    fac = LOG_DAEMON;
  }
  else
  {
    return jutil_args_error("Invalid value for facility [%s].", data[0]);
  }

  g_data.logger = jlog_syslog_session_init(JLOG_LOGTYPE_DEBUG, "jsys_test", fac);
  if(g_data.logger == NULL)
  {
    return jutil_args_error("Logger could not be initialized.");
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
char *argHandler_ip(const char **data, size_t data_size)
{
  if(strlen(data[0]) >= sizeof(g_data.address))
  {
    return jutil_args_error("Address has invalid size [%d : %s].", strlen(data[0]), data[0]);
  }

  memset(g_data.address, 0, sizeof(g_data.address));
  memcpy(g_data.address, data[0], sizeof(g_data.address));

  return NULL;
}

//------------------------------------------------------------------------------
//
char *argHandler_port(const char **data, size_t data_size)
{
  uint16_t port = atoi(data[0]);
  if(port == 0)
  {
    return jutil_args_error("Invalid value for port [%u].", port);
  }

  g_data.port = port;
  return NULL;
}

//------------------------------------------------------------------------------
//
char *argHandler_hashcode(const char **data, size_t data_size)
{
  int hashcode = atoi(data[0]);
  if(hashcode > 3)
  {
    return jutil_args_error("Invalid value for hash code [%d].", hashcode);
  }

  g_data.hash_code = hashcode;
  return NULL;
}
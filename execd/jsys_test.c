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
#include <getopt.h>
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

static const struct option long_options[] =
{
  { "syslog", required_argument,  0, 's' },
  { "ip",     required_argument,  0, 'i' },
  { "port",   required_argument,  0, 'p' },
  { "hash",   required_argument,  0, 0   },
  { "help",   no_argument,        0, 'h' },
  { NULL,     0,                  0, 0   }
};

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

static void jsys_signalHandler(int dummy);

static void jsys_paramMgr(int argc, char *argv[]);

static void jsys_errorUsage(const char *name);

static void jsys_setSyslog(const char *name, const char *facility);

static void jsys_dataHandler(void *ctx, jcon_client_t *client);

static void jsys_createHandler(void *ctx, const char *ref_string);

static void jsys_closeHandler(void *ctx, const char *ref_string);

static void jsys_cleanup();

//------------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
  jsys_paramMgr(argc, argv);
  jlog_global_session_set(g_data.logger);

  g_data.server = jcon_server_tcp_session_init(g_data.address, g_data.port, g_data.logger);
  if(g_data.server == NULL)
  {
    jsys_cleanup();
    return EXIT_FAILURE;
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
    jsys_cleanup();
    return EXIT_FAILURE;
  }

  struct timespec sleep_time;
  sleep_time.tv_sec = 1;
  sleep_time.tv_nsec = 0;

  signal(SIGINT, jsys_signalHandler);
  while(g_data.run)
  {
    nanosleep(&sleep_time, NULL);
  }

  jsys_cleanup();
  return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
//
void jsys_signalHandler(int dummy)
{
  JLOG_DEBUG("Shutdown signal recieved.");
  g_data.run = 0;
}

//------------------------------------------------------------------------------
//
void jsys_paramMgr(int argc, char *argv[])
{
  // --syslog <facility>
  // --ip <ip-address> (Default: localhost)
  // --port <port> (Default: 1234)
  // --hash <hash-enum> (Default: 1)
  // --help

  struct option *op = NULL;
  int index = -1;
  int opt;
  while( (opt = getopt_long(argc, argv, "s:i:p:", long_options, &index)) != -1)
  {
    switch(opt)
    {
      case 0:
      {
        op = (struct option *)&(long_options[index]);
        if(strcmp(op->name, "hash"))
        {
          jsys_errorUsage(argv[0]);
        }

        g_data.hash_code = atoi(optarg);
        break;
      }

      case 's':
      {
        jsys_setSyslog(argv[0], optarg);
        break;
      }
      case 'i':
      {
        memcpy(g_data.address, optarg, sizeof(g_data.address));
        break;
      }
      case 'p':
      {
        g_data.port = atoi(optarg);
        break;
      }
      case 'h':
      {
        jsys_errorUsage(argv[0]);
      }

      default:
      {
        jsys_errorUsage(argv[0]);
      }
    }
  }

  if(g_data.logger == NULL)
  {
    g_data.logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  }
}

//------------------------------------------------------------------------------
//
void jsys_errorUsage(const char *name)
{
  fprintf(stderr, "Usage: %s [--syslog/-s <syslog-facility>] [--ip/-i <ip-address>] [--port/-ip <port>] [--hash <hash-code>] [--help/-h] .\n", name);
  exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
//
void jsys_setSyslog(const char *name, const char *facility)
{
  int fac;

  if(strcmp(facility, "user") == 0)
  {
    fac = LOG_USER;
  }
  else if(strcmp(facility, "daemon") == 0)
  {
    fac = LOG_DAEMON;
  }
  else
  {
    jsys_errorUsage(name);
  }

  g_data.logger = jlog_syslog_session_init(JLOG_LOGTYPE_DEBUG, name, fac);
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
void jsys_cleanup()
{
  if(g_data.system)
  {
    jcon_system_free(g_data.system);
  }

  if(g_data.server)
  {
    jcon_server_free(g_data.server);
  }

  if(g_data.logger)
  {
    jlog_session_free(g_data.logger);
  }
}
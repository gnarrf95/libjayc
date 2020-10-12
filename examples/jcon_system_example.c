#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jutil_time.h>
#include <jayc/jcon_server_tcp.h>
#include <jayc/jcon_system.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define SIGNAL_INTERRUPT 2

static int run_system = 1;

static jcon_server_t *g_server = NULL;
static jcon_system_t *g_sys = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_sys != NULL)
  {
    jcon_system_free(g_sys);
  }

  if(g_server != NULL)
  {
    jcon_server_free(g_server);
  }
}

static void data_handler(void *ctx, jcon_client_t *client);
static void create_handler(void *ctx, const char *ref_string);
static void close_handler(void *ctx, const char *ref_string);

static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Caught signal [%d], stopping system.");
  run_system = 0;
}

int main()
{
  jproc_exit_setHandler(&exit_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  int ret_check;

  // Create server.
  g_server = jcon_server_tcp_session_init("127.0.0.1", 1234, logger);
  if(g_server == NULL)
  {
    JLOG_ERROR("Could not create server.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Start server.
  ret_check = jcon_server_reset(g_server);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not start server.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Create and start system.
  g_sys = jcon_system_init
  (
    g_server,
    &data_handler,
    &create_handler,
    &close_handler,
    logger,
    NULL
  );
  if(g_sys == NULL)
  {
    JLOG_ERROR("Could not create system.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  jproc_signal_setHandler(SIGNAL_INTERRUPT, &signal_handler, NULL);

  // Run system until signal is caught.
  while(run_system)
  {
    jutil_time_sleep(1, 0);
  }

  // Stop and free everything.
  jcon_system_free(g_sys);
  g_sys = NULL;

  jcon_server_free(g_server);
  g_server = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}

void data_handler(void *ctx, jcon_client_t *client)
{
  int ret_check;

  char buf[256] = { 0 };
  size_t buf_size = sizeof(buf);

  // Read data.
  ret_check = jcon_client_recvData(client, buf, buf_size);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not read data.");
    return;
  }

  // Print data.
  JLOG_INFO("Client [%s] recieved data [%s].", jcon_client_getReferenceString(client), buf);

  char *resp = "ACK";

  // Send acknowledgement message.
  ret_check = jcon_client_sendData(client, resp, strlen(resp));
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not send data.");
  }
}

void create_handler(void *ctx, const char *ref_string)
{
  JLOG_INFO("New connection [%s].", ref_string);
}

void close_handler(void *ctx, const char *ref_string)
{
  JLOG_INFO("Client disconnected [%s].", ref_string);
}

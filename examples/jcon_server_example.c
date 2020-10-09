#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jcon_client.h>
#include <jayc/jcon_server_tcp.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define SIGNAL_INTERRUPT 2

static int run_server = 1;

static jcon_server_t *g_server = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_server != NULL)
  {
    jcon_server_free(g_server);
  }
}

static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Caught signal [%d], stopping server.");
  run_server = 0;
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

  // Create server session.
  g_server = jcon_server_tcp_session_init("127.0.0.1", 1234, logger);
  if(g_server == NULL)
  {
    JLOG_ERROR("Could not create server.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Open server.
  ret_check = jcon_server_reset(g_server);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not open server.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  jproc_signal_setHandler(SIGNAL_INTERRUPT, &signal_handler, NULL);

  // Manage server until signal is caught.
  while(run_server)
  {
    // Check if new connection available.
    if(jcon_server_newConnection(g_server) == 0)
    {
      continue;
    }

    // Accept new connection.
    jcon_client_t *new_con = jcon_server_acceptConnection(g_server);
    if(new_con == NULL)
    {
      JLOG_ERROR("Could not accept connection.");
      continue;
    }

    // Manage connection until disconnect.
    while(jcon_client_isConnected(new_con))
    {
      // Check if new data available.
      if(jcon_client_newData(new_con) == 0)
      {
        continue;
      }

      char buf[256] = { 0 };
      size_t buf_size = sizeof(buf);

      // Read data.
      ret_check = jcon_client_recvData(new_con, buf, buf_size);
      if(ret_check == 0)
      {
        JLOG_ERROR("Could not read data.");
        continue;
      }

      // Print data read.
      JLOG_INFO("Client [%s] read data [%s].", jcon_client_getReferenceString(new_con), buf);

      char *resp = "ACK";

      // Send acknowledgement message.
      ret_check = jcon_client_sendData(new_con, resp, strlen(resp));
      if(ret_check == 0)
      {
        JLOG_ERROR("Could not send data.");
        continue;
      }
    }

    // Free memory of connection.
    jcon_client_session_free(new_con);
  }

  // Close server.
  jcon_server_close(g_server);

  // Free server memory.
  jcon_server_free(g_server);
  g_server = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
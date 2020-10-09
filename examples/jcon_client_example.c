#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jcon_client_tcp.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static jcon_client_t *client = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(client != NULL)
  {
    jcon_client_session_free(client);
  }
}

int main()
{
  jproc_exit_setHandler(exit_handler, NULL);

  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  int ret_check;

  // Create client.
  client = jcon_client_tcp_session_init("127.0.0.1", 1234, logger);
  if(client == NULL)
  {
    JLOG_ERROR("Could not create client.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Connect client.
  ret_check = jcon_client_reset(client);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not connect client.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Manage while connected.
  while(jcon_client_isConnected(client))
  {
    // Check if new data available.
    if(jcon_client_newData(client) == 0)
    {
      continue;
    }

    char buf[256] = { 0 };
    size_t buf_size = sizeof(buf);

    // Read data.
    ret_check = jcon_client_recvData(client, buf, buf_size);
    if(ret_check == 0)
    {
      JLOG_ERROR("Could not read data.");
      continue;
    }

    // Print data read.
    JLOG_INFO("Client [%s] read data [%s].", jcon_client_getReferenceString(client), buf);

    char *resp = "ACK";

    // Send acknowledgement message.
    ret_check = jcon_client_sendData(client, resp, strlen(resp));
    if(ret_check == 0)
    {
      JLOG_ERROR("Could not send data.");
      continue;
    }
  }

  // Free memory of client.
  jcon_client_session_free(client);
  client = NULL;

  jproc_exit(EXITVALUE_SUCCESS);
}
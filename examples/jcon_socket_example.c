#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jcon_socketTCP.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

#define SIGNAL_INTERRUPT 2

static int run_server = 1;

static jcon_socket_t *socket = NULL;

static void example_client()
{
  int ret_check;

  // Create TCP socket session (will use global logger).
  socket = jcon_socketTCP_simple_init("127.0.0.1", 1234, NULL);
  if(socket == NULL)
  {
    JLOG_ERROR("Could not create socket.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Conntect to server.
  ret_check = jcon_socket_connect(socket);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not connect.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Run while socket is connected.
  while(jcon_socket_isConnected(socket))
  {
    // Check if data is available.
    if(jcon_socket_pollForInput(socket, 100) == 0)
    {
      continue;
    }

    char buf[256] = { 0 };
    size_t buf_size = sizeof(buf);

    // When available, read data.
    ret_check = jcon_socket_recvData(socket, buf, buf_size);
    if(ret_check == 0)
    {
      JLOG_ERROR("Could not read data.");
      continue;
    }

    // Print data read.
    JLOG_INFO("Socket [%s] recieved data [%s].",
    jcon_socket_getReferenceString(socket),
    buf);

    char *resp = "ACK";

    // Send acknowledgement message.
    ret_check = jcon_socket_sendData(socket, resp, strlen(resp));
    if(ret_check == 0)
    {
      JLOG_ERROR("Could not send data.");
      continue;
    }
  }

  // Free socket session.
  jcon_socket_free(socket);
  socket = NULL;
}

static void example_server()
{
  int ret_check;

  // Create TCP socket session (will use global logger).
  socket = jcon_socketTCP_simple_init("127.0.0.1", 1234, NULL);
  if(socket == NULL)
  {
    JLOG_ERROR("Could not create socket.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Open server.
  ret_check = jcon_socket_bind(socket);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not bind.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  jproc_signal_setHandler(SIGNAL_INTERRUPT, &signal_handler, NULL);

  // Run until SIGINT signal is caught.
  while(run_server)
  {
    // Check if new connection is available.
    if(jcon_socket_pollForInput(socket, 100) == 0)
    {
      continue;
    }

    // If available, accept new connection.
    jcon_socket_t *new_con = jcon_socket_accept(socket);
    if(new_con == NULL)
    {
      JLOG_ERROR("Could not accept new connection.");
      continue;
    }

    // Manage connection to client.
    while(jcon_socket_isConnected(new_con))
    {
      // Check if data is available.
      if(jcon_socket_pollForInput(new_con, 100) == 0)
      {
        continue;
      }

      char buf[256] = { 0 };
      size_t buf_size = sizeof(buf);

      // When available, read data.
      ret_check = jcon_socket_recvData(new_con, buf, buf_size);
      if(ret_check == 0)
      {
        JLOG_ERROR("Could not read data.");
        continue;
      }

      // Print data read.
      JLOG_INFO("Socket [%s] recieved data [%s].",
      jcon_socket_getReferenceString(new_con),
      buf);

      char *resp = "ACK";

      // Send acknowledgement message.
      ret_check = jcon_socket_sendData(new_con, resp, strlen(resp));
      if(ret_check == 0)
      {
        JLOG_ERROR("Could not send data.");
        continue;
      }
    }

    // Free memory of client connection.
    jcon_socket_free(new_con);
  }

  // Close server.
  jcon_socket_close(socket);

  // Free socket session.
  jcon_socket_free(socket);
  socket = NULL;
}

static void signal_handler(int signum, void *ctx)
{
  JLOG_INFO("Signal [%d] caught, stopping server.", signum);
  run_server = 0;
}

static void exit_handler(int exit_value, void *ctx)
{
  if(socket != NULL)
  {
    jcon_socket_free(socket);
  }
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

  example_client();
  example_server();

  jproc_exit(EXITVALUE_SUCCESS);
}
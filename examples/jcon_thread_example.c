#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jutil_time.h>
#include <jayc/jcon_client_tcp.h>
#include <jayc/jcon_thread.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

static jcon_client_t *g_client = NULL;
static jcon_thread_t *g_thread = NULL;

static void exit_handler(int exit_value, void *ctx)
{
  if(g_client != NULL)
  {
    jcon_client_session_free(g_client);
  }

  if(g_thread != NULL)
  {
    jcon_thread_free(g_thread);
  }
}

static void data_handler(void *ctx, jcon_client_t *client);
static void create_handler(void *ctx, int create_type, const char *reference_string);
static void close_handler(void *ctx, int close_type, const char *reference_string);

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

  // Create client.
  g_client = jcon_client_tcp_session_init("127.0.0.1", 1234, logger);
  if(g_client == NULL)
  {
    JLOG_ERROR("Could not create client.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Connect client.
  ret_check = jcon_client_reset(g_client);
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not connect client.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Create thread.
  g_thread = jcon_thread_init(g_client, data_handler, create_handler, close_handler, logger, NULL);
  if(g_thread == NULL)
  {
    JLOG_ERROR("Could not create thread.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  // Wait while thread is running.
  while(jcon_thread_isRunning(g_thread))
  {
    // Sleep 1 second before checking again.
    jutil_time_sleep(1, 0);
  }

  // Free memory.
  jcon_thread_free(g_thread);
  g_thread = NULL;

  jcon_client_session_free(g_client);
  g_client = NULL;

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
    JLOG_ERROR("Could not read.");
    return;
  }

  // Print data read.
  JLOG_INFO("Client [%s] recieved [%s].", jcon_client_getReferenceString(client), buf);

  char *resp = "ACK";

  // Send acknowledgement message.
  ret_check = jcon_client_sendData(client, resp, strlen(resp));
  if(ret_check == 0)
  {
    JLOG_ERROR("Could not send data.");
    return;
  }
}

void create_handler(void *ctx, int create_type, const char *reference_string)
{
  JLOG_INFO("ClientThread [%s] started with create-type [%d].", reference_string, create_type);
}

void close_handler(void *ctx, int close_type, const char *reference_string)
{
  JLOG_INFO("ClientThread [%s] closed with close-type [%d].", reference_string, close_type);
}

#include <jcon_client.h>
#include <jcon_client_dev.h>
#include <stdlib.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
//
void jcon_client_session_free(jcon_client_t *session)
{
  if(session == NULL)
  {
    return;
  }

  if(session->session_free_handler)
  {
    session->session_free_handler(session->session_context);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_client_reset(jcon_client_t *session)
{
  if(session == NULL)
  {
    return false;
  }

  if(session->function_reset)
  {
    return session->function_reset(session->session_context);
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
//
void jcon_client_close(jcon_client_t *session)
{
  if(session == NULL)
  {
    return;
  }

  if(session->function_close)
  {
    session->function_close(session->session_context);
  }
}

//------------------------------------------------------------------------------
//
const char *jcon_client_getConnectionType(jcon_client_t *session)
{
  if(session == NULL)
  {
    return NULL;
  }

  return session->connection_type;
}

//------------------------------------------------------------------------------
//
int jcon_client_isConnected(jcon_client_t *session)
{
  if(session == NULL)
  {
    return false;
  }

  if(session->function_isConnected)
  {
    return session->function_isConnected(session->session_context);
  }

  return false;
}

//------------------------------------------------------------------------------
//
int jcon_client_newData(jcon_client_t *session)
{
  if(session == NULL)
  {
    return false;
  }

  if(session->function_newData)
  {
    return session->function_newData(session->session_context);
  }

  return false;
}

//------------------------------------------------------------------------------
//
size_t jcon_client_recvData(jcon_client_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    return 0;
  }

  if(data_ptr == NULL)
  {
    return 0;
  }

  if(session->function_recvData)
  {
    return session->function_recvData(session->session_context, data_ptr, data_size);
  }

  return 0;
}

//------------------------------------------------------------------------------
//
size_t jcon_client_sendData(jcon_client_t *session, void *data_ptr, size_t data_size)
{
  if(session == NULL)
  {
    return 0;
  }

  if(data_ptr == NULL)
  {
    return 0;
  }

  if(session->function_sendData)
  {
    return session->function_sendData(session->session_context, data_ptr, data_size);
  }

  return 0;
}
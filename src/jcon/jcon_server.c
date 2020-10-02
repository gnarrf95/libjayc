/**
 * @file jcon_server.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief 
 * 
 * @date 2020-09-22
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jcon_server_dev.h>
#include <stdbool.h>
#include <stdlib.h>
//------------------------------------------------------------------------------
//
void jcon_server_free(jcon_server_t *session)
{
  if(session->session_free_handler)
  {
    session->session_free_handler(session->session_context);
  }

  free(session);
}

//------------------------------------------------------------------------------
//
int jcon_server_reset(jcon_server_t *session)
{
  if(session->function_reset)
  {
    return session->function_reset(session->session_context);
  }

  return false;
}

//------------------------------------------------------------------------------
//
void jcon_server_close(jcon_server_t *session)
{
  if(session->function_close)
  {
    session->function_close(session->session_context);
  }
}

//------------------------------------------------------------------------------
//
int jcon_server_isOpen(jcon_server_t *session)
{
  if(session->function_isOpen)
  {
    return session->function_isOpen(session->session_context);
  }

  return false;
}

//------------------------------------------------------------------------------
//
const char *jcon_server_getConnectionType(jcon_server_t *session)
{
  return session->connection_type;
}

//------------------------------------------------------------------------------
//
const char *jcon_server_getReferenceString(jcon_server_t *session)
{
  if(session->function_getReferenceString)
  {
    return session->function_getReferenceString(session->session_context);
  }

  return NULL;
}

//------------------------------------------------------------------------------
//
int jcon_server_newConnection(jcon_server_t *session)
{
  if(session->function_newConnection)
  {
    return session->function_newConnection(session->session_context);
  }

  return false;
}

//------------------------------------------------------------------------------
//
jcon_client_t *jcon_server_acceptConnection(jcon_server_t *session)
{
  if(session->function_acceptConnection)
  {
    return session->function_acceptConnection(session->session_context);
  }

  return NULL;
}
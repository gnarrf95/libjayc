#include <jlog_syslog.h>
#include <syslog.h>
#include <stdlib.h>

#define TEST_ID "jlog_syslog_test"
#define TEST_FAC LOG_USER

void test_logLevel(uint8_t loglevel);

void test_object(uint8_t loglevel);
void test_messageHandler(uint8_t loglevel);
void test_messageHandler_m(uint8_t loglevel);
void test_global_object(uint8_t loglevel);
void test_global_messageHandler(uint8_t loglevel);
void test_global_messageHandler_m(uint8_t loglevel);
void test_macro_messages(uint8_t loglevel);

//------------------------------------------------------------------------------
//
int main()
{
  for(uint8_t i = 0; i < 10; i++)
  {
    test_logLevel(i);
  }

  return 0;
}

//------------------------------------------------------------------------------
//
void test_logLevel(uint8_t loglevel)
{
  test_object(loglevel);
  test_messageHandler(loglevel);
  test_messageHandler_m(loglevel);

  test_global_object(loglevel);
  test_global_messageHandler(loglevel);
  test_global_messageHandler_m(loglevel);

  test_macro_messages(loglevel);
}

//------------------------------------------------------------------------------
//
void test_object(uint8_t loglevel)
{
  jlog_t *session = NULL;

  session = jlog_syslog_init(loglevel, TEST_ID, TEST_FAC);
  jlog_session_free(session);

  session = jlog_syslog_init(loglevel, TEST_ID, TEST_FAC);
  if(session->free_handler)
  {
    session->free_handler(session->session_context);
  }
  free(session);
}

//------------------------------------------------------------------------------
//
void test_messageHandler(uint8_t loglevel)
{
  jlog_t *session = jlog_syslog_init(loglevel, TEST_ID, TEST_FAC);

  session->log_function(session->session_context, JLOG_LOGTYPE_DEBUG, "Debug log from log_handler() with loglevel [%u].", loglevel);
  session->log_function(session->session_context, JLOG_LOGTYPE_INFO, "Info log from log_handler() with loglevel [%u].", loglevel);
  session->log_function(session->session_context, JLOG_LOGTYPE_WARN, "Warning log from log_handler() with loglevel [%u].", loglevel);
  session->log_function(session->session_context, JLOG_LOGTYPE_ERROR, "Error log from log_handler() with loglevel [%u].", loglevel);

  jlog_log_message(session, JLOG_LOGTYPE_DEBUG, "Debug log from jlog_log_message() with loglevel [%u].", loglevel);
  jlog_log_message(session, JLOG_LOGTYPE_INFO, "Info log from jlog_log_message() with loglevel [%u].", loglevel);
  jlog_log_message(session, JLOG_LOGTYPE_WARN, "Warning log from jlog_log_message() with loglevel [%u].", loglevel);
  jlog_log_message(session, JLOG_LOGTYPE_ERROR, "Error log from jlog_log_message() with loglevel [%u].", loglevel);

  jlog_session_free(session);
}

//------------------------------------------------------------------------------
//
void test_messageHandler_m(uint8_t loglevel)
{
  jlog_t *session = jlog_syslog_init(loglevel, TEST_ID, TEST_FAC);

  session->log_function_m(session->session_context, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, "Debug log from log_handler_m() with loglevel [%u].", loglevel);
  session->log_function_m(session->session_context, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, "Info log from log_handler_m() with loglevel [%u].", loglevel);
  session->log_function_m(session->session_context, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, "Warning log from log_handler_m() with loglevel [%u].", loglevel);
  session->log_function_m(session->session_context, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, "Error log from log_handler_m() with loglevel [%u].", loglevel);

  jlog_log_message_m(session, JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, "Debug log from jlog_log_message_m() with loglevel [%u].", loglevel);
  jlog_log_message_m(session, JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, "Info log from jlog_log_message_m() with loglevel [%u].", loglevel);
  jlog_log_message_m(session, JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, "Warning log from jlog_log_message_m() with loglevel [%u].", loglevel);
  jlog_log_message_m(session, JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, "Error log from jlog_log_message_m() with loglevel [%u].", loglevel);

  jlog_session_free(session);
}

//------------------------------------------------------------------------------
//
void test_global_object(uint8_t loglevel)
{
  jlog_global_session_set(jlog_syslog_init(loglevel, TEST_ID, TEST_FAC));
  jlog_global_session_free();
}

//------------------------------------------------------------------------------
//
void test_global_messageHandler(uint8_t loglevel)
{
  jlog_global_session_set(jlog_syslog_init(loglevel, TEST_ID, TEST_FAC));

  jlog_global_log_message(JLOG_LOGTYPE_DEBUG, "Debug log from jlog_global_log_message() with loglevel [%u].", loglevel);
  jlog_global_log_message(JLOG_LOGTYPE_INFO, "Info log from jlog_global_log_message() with loglevel [%u].", loglevel);
  jlog_global_log_message(JLOG_LOGTYPE_WARN, "Warning log from jlog_global_log_message() with loglevel [%u].", loglevel);
  jlog_global_log_message(JLOG_LOGTYPE_ERROR, "Error log from jlog_global_log_message() with loglevel [%u].", loglevel);

  jlog_global_session_free();
}

//------------------------------------------------------------------------------
//
void test_global_messageHandler_m(uint8_t loglevel)
{
  jlog_global_session_set(jlog_syslog_init(loglevel, TEST_ID, TEST_FAC));

  jlog_global_log_message_m(JLOG_LOGTYPE_DEBUG, __FILE__, __func__, __LINE__, "Debug log from jlog_global_log_message_m() with loglevel [%u].", loglevel);
  jlog_global_log_message_m(JLOG_LOGTYPE_INFO, __FILE__, __func__, __LINE__, "Info log from jlog_global_log_message_m() with loglevel [%u].", loglevel);
  jlog_global_log_message_m(JLOG_LOGTYPE_WARN, __FILE__, __func__, __LINE__, "Warning log from jlog_global_log_message_m() with loglevel [%u].", loglevel);
  jlog_global_log_message_m(JLOG_LOGTYPE_ERROR, __FILE__, __func__, __LINE__, "Error log from jlog_global_log_message_m() with loglevel [%u].", loglevel);

  jlog_global_session_free();
}

//------------------------------------------------------------------------------
//
void test_macro_messages(uint8_t loglevel)
{
  jlog_global_session_set(jlog_syslog_init(loglevel, TEST_ID, TEST_FAC));

  JLOG_DEBUG("Debug log from JLOG_DEBUG() with loglevel [%u].", loglevel);
  JLOG_INFO("Info log from JLOG_INFO() with loglevel [%u].", loglevel);
  JLOG_WARN("Warning log from JLOG_WARNING() with loglevel [%u].", loglevel);
  JLOG_ERROR("ERROR log from JLOG_ERROR() with loglevel [%u].", loglevel);

  jlog_global_session_free();
}
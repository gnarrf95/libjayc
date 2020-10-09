#include <jayc/jlog_stdio.h>
#include <stddef.h>

int main()
{
  // Create logger that prints everything but debug messages to stdout/stderr.
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_INFO);
  if(logger == NULL)
  {
    // Something went wrong.
  }

  // Set logger as global, so that it can be accessed from anywhere in program.
  jlog_global_session_set(logger);
  // logger variable can now be reused (because global logger is saved individually).
  
  // Create color context for colored output.
  void *color_context = jlog_stdio_color_context_init("\033[0;32m", NULL, "\033[01;33m", "\033[1;31m");

  // Create logger with colored input that also prints debug messages.
  logger = jlog_stdio_color_session_init(JLOG_LOGTYPE_DEBUG, color_context);

  // Print debug messages (global logger should not print).
  jlog_log_message(logger, JLOG_LOGTYPE_DEBUG, "Hello debug.");
  jlog_global_log_message(JLOG_LOGTYPE_DEBUG, "Hello debug.");

  // Print info messages.
  jlog_log_message(logger, JLOG_LOGTYPE_INFO, "Hello info.");
  jlog_global_log_message(JLOG_LOGTYPE_INFO, "Hello info.");

  // Print warning messages.
  jlog_log_message(logger, JLOG_LOGTYPE_WARN, "Hello warning.");
  jlog_global_log_message(JLOG_LOGTYPE_WARN, "Hello warning.");

  // Print error messages.
  jlog_log_message(logger, JLOG_LOGTYPE_ERROR, "Hello error.");
  jlog_global_log_message(JLOG_LOGTYPE_ERROR, "Hello error.");

  // Print critical messages.
  jlog_log_message(logger, JLOG_LOGTYPE_CRITICAL, "Hello critical.");
  jlog_global_log_message(JLOG_LOGTYPE_CRITICAL, "Hello critical.");

  // Print fatal messages (program will exit after print).
  jlog_log_message(logger, JLOG_LOGTYPE_FATAL, "Hello fatal.");
  jlog_global_log_message(JLOG_LOGTYPE_FATAL, "Hello fatal.");

  // Global logger also allows simplified interface.
  JLOG_DEBUG("Debug message.");
  JLOG_INFO("Info message.");
  JLOG_WARN("Warning message.");
  JLOG_ERROR("Error message.");
  JLOG_CRITICAL("Critical message.");
  JLOG_FATAL("Fatal message.");

  // Free local logger (also frees color context).
  jlog_session_free(logger);

  // Free global logger.
  jlog_global_session_free();

  return 0;
}
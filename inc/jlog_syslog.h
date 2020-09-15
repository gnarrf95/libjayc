#ifndef INCLUDE_JLOG_SYSLOG_H
#define INCLUDE_JLOG_SYSLOG_H

#include <jlog.h>

/*******************************************************************************
 * @brief Create session object. jlog_syslog session is handled as singleton.
 *        Only one session can exists in program.
 *
 * @param id : Program name.
 * @param facility : Type of program (see '$ man syslog').
 * 
 * @return : jlog session object.
 */
jlog_t *jlog_syslog_session_init(uint8_t log_level, const char *id, int facility);

#endif
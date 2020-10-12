/**
 * @file jlog_syslog.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implementation of jlog, that sends logs to syslogd.
 * 
 * This jlog implementation sends its logs to the syslogd daemon.
 * The log types get passed as the according types defined by sys/types.h .
 * 
 * This implementation handles sessions as singleton. Once
 * @c #jlog_syslog_session_init() is called, calling the function again
 * will only return the same session, that was created the first time.
 * Only after freeing the session, a new session can be created.
 * 
 * When creating a session, a syslog ID and facility have to be stated.
 * As ID the program name is recommended. The facility should be stated
 * to represent the use of the program.
 * 
 * @date 2020-09-21
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 * @see https://man7.org/linux/man-pages/man3/syslog.3.html#DESCRIPTION
 * @see jlog.h
 * 
 */

#ifndef INCLUDE_JLOG_SYSLOG_H
#define INCLUDE_JLOG_SYSLOG_H

#include <jayc/jlog.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create jlog_syslog session object.
 * 
 * jlog_syslog session is handled as singleton. Only one session can exists
 * in program.
 *
 * @param id        Program name.
 * @param facility  Type of program (see <tt>$ man syslog</tt>).
 * 
 * @return          jlog session object.
 * @return          @c NULL , if failed.
 */
jlog_t *jlog_syslog_session_init(int log_level, const char *id, int facility);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JLOG_SYSLOG_H */
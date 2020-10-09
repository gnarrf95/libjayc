/**
 * @file jutil_io.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Abstract interface for data IO.
 * 
 * Some applications might need input/output using
 * different methods. This interface allows, to
 * swap these methods out and replace them using
 * a unified interface.
 * 
 * The functions are there and can be used with
 * any implementation.
 * 
 * <b>Example Applications:</b>
 * - CLI command parsing from stdio.
 * - Communicating with daemon using IPC communication.
 * 
 * @date 2020-10-10
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_IO_H
#define INCLUDE_JUTIL_IO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __jutil_io_session jutil_io_t;

void jutil_io_free(jutil_io_t *session);

int jutil_io_print(jutil_io_t *session, const char *fmt, ...);
int jutil_io_printLine(jutil_io_t *session, const char *fmt, ...);

size_t jutil_io_read(jutil_io_t *session, char *buffer, size_t buf_size);
int jutil_io_readLine(jutil_io_t *session, char **buf_ptr, size_t *buf_size);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_IO_H */
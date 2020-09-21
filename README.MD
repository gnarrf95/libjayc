
[![GitHub last commit](https://img.shields.io/github/last-commit/gnarrf95/libjayc?style=flat-square)]()
[![GitHub issues](https://img.shields.io/github/issues/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/issues)
[![GitHub](https://img.shields.io/github/license/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/blob/master/LICENSE)

```
 _  _  _        ___                _____ 
| |(_)| |      |_  |              /  __ \
| | _ | |__      | |  __ _  _   _ | /  \/
| || || '_ \     | | / _` || | | || |    
| || || |_) |/\__/ /| (_| || |_| || \__/\
|_||_||_.__/ \____/  \__,_| \__, | \____/
                             __/ |       
                            |___/        
```

`libjayc` is a C-only utility library, which implements a lot of useful
features, to make the creation of projects easier. There are background
features like Loggers, connectors, server handlers, etc.

Although the project is written in C, it uses a object-oriented paradigm.
I decided to use this approach, to allow new implementations, while making
it possible to use these implementations in a unified, abstract interface.

## Table of Contents

* [About the Project](#about-the-project)
* [Getting Started](#getting-started)
  * [Prerequesites](#prerequesites)
  * [Build the Library](#build-the-library)
  * [Build the Documentation](#build-the-documentation)
  * [Installation](#installation)
* [Components](#components)
  * [jlog](#jlog)
  * [jlog_stdio](#jlog_stdio)
  * [jlog_syslog](#jlog_syslog)
  * [jcon_client](#jcon_client)
  * [jcon_client_tcp](#jcon_client_tcp)
  * [jcon_thread](#jcon_thread)
* [Using the Library](#using-the-library)
  * [Example Program](#example-program)
  * [Compiling Programs with libJayC](#compiling-programs-with-libjayc)
* [License](#license)

## About The Project

## Getting Started

### Prerequesites
_This project does not have any library requirements yet._

### Build The Library
The check, that all the source files compile successfully, call:
```bash
make check
```

To compile the library, simply call `make`.

### Build The Documentation
The documentation is build using:
```bash
make docs
```

The doxygen HTML documentation will be placed in `./docs/doxygen/`.

### Installation
To install the library, just call:
```bash
make install
```

By default the library will be installed in `/usr/local/`.

If you want to specify the directory, in which the library gets installed,
you can provide a path-prefix using the PREFIX variable:
```bash
make install PREFIX=<your installation path>
```

## Components
The library uses _j_ as a prefix for its naming convention (f.ex. _jlog_, _jcon_).

### jlog
_jlog_ is a interface for a logger system. It provides a type and global
functions to use different implementations.

There also are functions to use a global jlog session. This logger (when
initialized) can be accessed from anywhere in the program, with out having
direct access to the session object.

_jlog_ on its own does not provide a useful implementations, however _libjayc_
does provide simple implementations.

There is a _quiet_ session, that can be created (`jlog_session_quiet()`).
This session however, does not do anything. It is there to use in components,
that you don't want to log anything (f.ex. if _jcon\_client\_tcp_ does not
get a logger session, it defaults to the global logger).

#### Interface
`_session_init()` functions from implementations will return a `jlog_t`
pointer. This will be the session object to use in the interface functions.

Since session object are dynamically allocated memory, the session must
be closed by calling `jlog_session_free()` whith the session object as
parameter.   

Logs messages are sent by using `jlog_log_message()` and `jlog_log_message_m()`.
The `_m` suffix states, that the function is including source code
information in its log messages.   
The source code information are:
* Filename (by using `__FILE__`)
* Function name (by using `__func__`)
* Line number (by using `__LINE__`)

#### Global Session
Created sessions can also be used as a _global session_. That means, that
a logger can be used by calling the functions `jlog_global_log_message()` and
`jlog_global_log_message_m()`.

For the global session to be usable, it has to be set with
`jlog_global_session_set()` with a initialized session as parameter.   
This can then be freed by using `jlog_global_session_free()`.

The global session can also be accessed by macros. There is a macro for
each log type. These macros also directly pass the source code information
to the function.   
They are:
* `JLOG_DEBUG()`
* `JLOG_INFO()`
* `JLOG_WARN()`
* `JLOG_ERROR()`
* `JLOG_CRITICAL()`
* `JLOG_FATAL()`

#### Log Types
_jlog_ has 6 log types:
* `JLOG_LOGTYPE_DEBUG`
* `JLOG_LOGTYPE_INFO`
* `JLOG_LOGTYPE_WARN`
* `JLOG_LOGTYPE_ERROR`
* `JLOG_LOGTYPE_CRITICAL`
* `JLOG_LOGTYPE_FATAL`

While _DEBUG_, _INFO_, _WARN_ and _ERROR_ are widely known; _CRITICAL_ and
_FATAL_ are a bit more special-case.

If a log with the log type `JLOG_LOGTYPE_FATAL` gets sent (no matter what
kind of implementation), the program will exit with return value
`EXIT_FAILURE`.   
`JLOG_LOGTYPE_CRITICAL` and `JLOG_LOGTYPE_ERROR` can work the same way. For
that to happen, the macros `JLOG_EXIT_ATERROR` or `JLOG_EXIT_ATCRITICAL`
have to be defined.
```bash
gcc -D JLOG_EXIT_ATCRITICAL ...
gcc -D JLOG_EXIT_ATERROR ...
```

### jlog_stdio
### jlog_syslog
### jcon_client
### jcon_client_tcp
### jcon_thread

## Using The Library

### Example Program
For an example to use the library, let's write a simple _Hello World_
program, using the _jlog\_stdio_ logger. Let's call it `hw.c`.

```c
#include <jlog_stdio.h>
#include <stdlib.h>

int main()
{
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    return EXIT_FAILURE;
  }

  jlog_log_message(logger, JLOG_LOGTYPE_INFO, "Hello World!");
  jlog_session_free(logger);

  return EXIT_SUCCESS;
}
```
### Compiling Programs With libJayC
Now to compile the program we have to pass the _libjayc_ library to the
linker:
```bash
gcc hw.c -o hw -ljayc
```

The program should return this output:
```
[ =INF= ] Hello World!
```

If you used a different install directory, than `/usr/local` or `/usr`, then
you need to pass the include- and library-path to the compiler:
```bash
gcc hw.c -o hw -I<path to library>/include/ -L<path to library>/lib/ -ljayc
```

## License
Distributed under [MIT License](LICENSE).
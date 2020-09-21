
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

#### Log Types
_jlog_ has 6 log types:
* JLOG_LOGTYPE_DEBUG
* JLOG_LOGTYPE_INFO
* JLOG_LOGTYPE_WARN
* JLOG_LOGTYPE_ERROR
* JLOG_LOGTYPE_CRITICAL
* JLOG_LOGTYPE_FATAL

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
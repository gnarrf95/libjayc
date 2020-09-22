
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

## Table of Contents

* [About the Project](#about-the-project)
* [Getting Started](#getting-started)
  * [Prerequesites](#prerequesites)
  * [Build the Library](#build-the-library)
  * [Build the Documentation](#build-the-documentation)
  * [Installation](#installation)
* [Components](#components)
  * [jlog](#jlog)
  * [jcon_client](#jcon_client)
  * [jcon_thread](#jcon_thread)
  * [jcon_server](#jcon_server)
* [Using the Library](#using-the-library)
  * [Example Program](#example-program)
  * [Compiling Programs with libJayC](#compiling-programs-with-libjayc)
* [License](#license)

## About The Project
`libjayc` is a C-only utility library, which implements a lot of useful
features, to make the creation of projects easier. There are background
features like Loggers, connectors, server handlers, etc.

Although the project is written in C, it uses a object-oriented paradigm.
I decided to use this approach, to allow new implementations, while making
it possible to use these implementations in a unified, abstract interface.

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

Since session objects are dynamically allocated memory, the session must
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

### jcon_client
_jcon\_client_ is a interface for connector functionality.
The concept is to create a template for implementations,
that connect to a server using all types of communications
and allow them to be controlled through a unified interface.

For example, there can be implementations using TCP, UDP, HTTP,
or more specialized protocols, like syslog protocol, UNIX sockets,
websockets, etc.

### Sessions
`_session_init()` functions from implementations will return a `jcon_client_t`
pointer. This will be the session object to use in the interface functions.

Since session objects are dynamically allocated memory, the session must
be closed by calling `jcon_client_session_free()` whith the session object as
parameter.

After initialization, the client must be reset (meaning closed if connected,
then connect). This is done using the function `jcon_client_reset()`.   
Likewise a session is closed using `jcon_client_close()`. This will
automatically done, when the session is freed.

With the function `jcon_client_isConnected()` the connection state can be
checked.

#### Data IO
Data is sent and recieved as byte arrays. To send a message, the function
`jcon_client_sendData()` is used. It needs the pointer to the array, as well
as the size of the array.

For recieving data, there is the function `jcon_client_newData()`, that returns
if the client has data available to read.   
If true, the data can be read using `jcon_client_recvData()`. Same as the
send function, the recv function needs a byte array and its size.   
It is also possible to give a NULL pointer for the byte array. In this case,
the function will read from the client (as many bytes as stated in size
parameter) but nothing can be done with the data.

#### Getters
_jcon\_client_ also has getters, to request information about the client.

`jcon_client_getConnectionType()` will return a string representing the
type of implementation (type of communication). This can be for example:
* `TCP`
* `UDP`
* `UNIX_SOCK`
* etc.

`jcon_client_getReferenceString()` adds to that information about the
connection. In case of a TCP connection, that would be IP address and port
the client is connected to.   
Example: `TCP:127.0.0.1:4443`

### jcon_thread
_jcon\_thread_ is a threaded jcon_client. The thread runs in a loop and
checks for new data and for disconnects.

There are handlers that trigger, when there is new data available, or
when the client disconnects.

#### Interface
_jcon\_thread_ gets initialized using `jcon_thread_init()`. It needs
a client, the handlers, a logger and a context that gets passed to the
handlers as a parameter.

Likewise, the thread gets destroyed with `jcon_thread_free()`.   
This tells the thread to stop and waits for it to exit.
It then frees any memory allocated.

The client does not get automatically destroyed. This has to be done either
manually, after the thread was destroyed; or in the
`jcon_thread_close_handler()`.

#### Getters
_jcon\_thread_ has getters to give information about its and the clients
status.

`jcon_thread_isRunning()` tells, if the thread is still running.   
`jcon_thread_getConnectionType()` gives the connection type of the client.   
`jcon_thread_getReferenceString()` gives the reference string of the client.

### jcon_server

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

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
* [Note from Developer](#note-from-developer)
* [Getting Started](#getting-started)
  * [Prerequesites](#prerequesites)
  * [Building the Library](#building-the-library)
  * [Building the Documentation](#building-the-documentation)
  * [Installation](#installation)
* [Components](#components)
  * [jlog](#jlog)
  * [jcon](#jcon)
  * [jutil](#jutil)
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

### Note From Developer
This is a study project for me. I am creating this library,
first because these are components i regularly need in my
projects; and second for me to learn how to manage, structure
and deploy projects like this.

Some of my solutions and implementations might seem
_suboptimal_ to some developers. I know that.
I'm trying out some new things here.

Thanks for your understanding! :grin:   
Greetings, [@gnarrf95](https://github.com/gnarrf95).

## Getting Started
### Prerequesites
_This project does not have any library requirements yet._

### Building The Library
The check, that all the source files compile successfully, call:
```bash
make check
```

To compile the library, simply call `make libs`.

### Building The Documentation
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
The library uses _j_ as a prefix for its naming convention (f.ex. _jlog_, _jcon_, _jutil_).

### jlog
_jlog_ is a interface for a logger system. It provides a type and global
functions to use different implementations.

There also are functions to use a global jlog session. This logger (when
initialized) can be accessed from anywhere in the program, with out having
direct access to the session object.

_jlog_ on its own does not provide implementations, however _libjayc_
does provide simple usable implementations.

There is a _quiet_ session, that can be created (`jlog_session_quiet()`).
This session however, does not do anything. It is there to use in components,
that you don't want to log anything (f.ex. if _jcon\_client\_tcp_ does not
get a logger session, it defaults to the global logger).

### jcon
_jcon_ is a component that provides interfaces (and simple
implementations) for connection handling.

Most are simple server/client systems, but there are more powerful
modules.

#### jcon_client
This is a simple client interface, that connects to a server
and communicates (depending on implementation).

#### jcon_thread
A threaded client. This runs in the background and calls
a user defined handler for events (creation, data input, disconnect).

#### jcon_server
The server counterpart to _jcon\_client_.
Listens to a server connection and creates _jcon\_client_ instances
connected to clients.

#### jcon_system
A whole server system. The server gets manages in a thread
and new connections are managed as _jcon\_thread_ instances.

Also calls handlers for events.

### jutil
The _jutil_ component contains a few useful abstractions for
functionality.

#### jutil_linkedlist
A implementation of a simple linked list for easy data storage.

#### jutil_thread
A abstraction for thread handling (using pthread library).

#### jutil_crypto
Contains functions to generate hashes.

(Encryptions also planned, but not currently available)

## Using The Library

### Example Program
For an example to use the library, let's write a simple _Hello World_
program, using the _jlog\_stdio_ logger. Let's call it `hw.c`.

```c
#include <jayc/jlog_stdio.h>
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
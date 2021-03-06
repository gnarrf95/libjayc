
[![GitHub last commit](https://img.shields.io/github/last-commit/gnarrf95/libjayc?style=flat-square)]()
[![GitHub issues](https://img.shields.io/github/issues/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/issues)
[![GitHub](https://img.shields.io/github/license/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/blob/master/LICENSE)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/990a9602ea134bb583852ec2fc43c095)](https://www.codacy.com/manual/gnarrf95/libjayc/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=gnarrf95/libjayc&amp;utm_campaign=Badge_Grade)

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
  * [jinfo](#info)
  * [jproc](#jproc)
  * [jconfig](#jconfig)
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
There are no library requirements to compile the project.

However, the documentation requires _doxygen_, _doxygen-doc_
and _graphviz_ to create.

### Building The Library
The check, that all the source files compile successfully, call:
`make check` or only `make`.

To compile the library, simply call `make lib`.   
To additionally compile the included binaries, call `make bins`.

To compile everything, call `make all`.

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

If you want the headers also installed for development, use
```bash
make install_dev
```

To also install the binaries, use
```bash
make install_all
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
that you don't want to log anything (f.ex. if _jcon\_thread_ does not
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

#### jutil_args
A command line argument parser. Uses user defined parameter
options with handler, to call when argument is passed.

#### jutil_linkedlist
A implementation of a simple linked list for easy data storage.

#### jutil_map
A string indexed map. Based on _jutil\_linkedlist_, so not
really an efficient approach. But it works and is simple
to use.

#### jutil_thread
A abstraction for thread handling (using pthread library).

#### jutil_crypto
Contains functions to generate hashes.

(Encryptions also planned, but not currently available)

#### jutil_cli
A interface to handle CLI input.

#### jutil_time
Provides functionality for time management.

Provides stop watch, timer, time format and sleep functions.

### jinfo
The _jinfo_ component has functionality to get information
about the library build.

`jinfo_build_version()` returns info about the library version
and the datetime of the compilation.

`jinfo_build_platform()` returns info about the OS and the
architecture the library was compiled on.

`jinfo_build_compiler()` returns the name and version of
the compiler used.

### jproc
The _jproc_ component provides functionality for process
handling.

For now it allows setting a exit handler, which is useful
to free any global memory before exiting the program. Second
you can set a signal handler to catch any signals recieved by
the process.

### jconfig
_jconfig_ provides functionality for parsing configuration files.
For now only simple _raw_ file formats are supported.

In the future, more file types may be supported.

## Using The Library

### Example Program
For an example to use the library, let's write a simple _Hello World_
program, using the _jlog\_stdio_ logger. Let's call it `hw.c`.

```c
#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>

int main()
{
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(-1);
  }

  jlog_log_message(logger, JLOG_LOGTYPE_INFO, "Hello World!");
  jlog_session_free(logger);

  jproc_exit(0);
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
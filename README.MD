
[![GitHub last commit](https://img.shields.io/github/last-commit/gnarrf95/libjayc?style=flat-square)]()
[![GitHub issues](https://img.shields.io/github/issues/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/issues)
[![GitHub](https://img.shields.io/github/license/gnarrf95/libjayc?style=flat-square)](https://github.com/gnarrf95/libjayc/blob/master/LICENSE)

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
* [Using the Library](#using-the-library)
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

## Using The Library

## License
Distributed under [MIT License](LICENSE).
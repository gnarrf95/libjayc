# ==============================================================================
# Compiler

# Change compiler using "make COMPILER=<other-compiler>"
COMPILER ?= gcc
CC = $(COMPILER) -Wall -Werror -std=c11 -fPIC

# ==============================================================================
#Compiler and linker flags

INC = -I inc/

CF_MYSQL = `mysql_config --cflags`
CFLAGS = $(INC)

LDF_JANSSON = -ljansson
LDF_PTHREAD = -lpthread
LDF_MYSQL = `mysql_config --libs`
LDF_CRYPTO = -lcrypto

LDFLAGS = $(LDF_PTHREAD) $(LDF_CRYPTO)

# Use build flags to change compilation parameters for library.
# Flags:
# * "-D JCON_NO_DEBUG"  if jcon modules should not log debug messages
# * "-D JUTIL_NO_DEBUG" if jutil modules should not log debug messages
# * "-D JLOG_EXIT_ATCRITICAL" if program should exit at critical log
# * "-D JLOG_EXIT_ATERROR" if program should exit at error log
BUILD_FLAGS = 

# ==============================================================================
# Sources

HEADERS = $(wildcard inc/*.h)

SRC = $(wildcard src/*.c)
SRC_TESTS = $(wildcard tests/*.c)
SRC_EXEC = $(wildcard exec/*.c)
SRC_EXECD = $(wildcard execd/*.c)

# ==============================================================================
# Objects

OBJ = $(subst src, build/objects, $(SRC:.c=.o))

# ==============================================================================
# Targets

TARGET_LIBJAYC = build/lib/libjayc.so
# TARGET_TESTS = $(subst tests,build/tests,$(SRC_TESTS:.c=))
TARGET_EXEC = $(subst exec,build/bin,$(SRC_EXEC:.c=))
TARGET_EXECD = $(subst execd,build/sbin,$(SRC_EXECD:.c=))

# ==============================================================================
# Install variables

PREFIX ?= /usr/local

HEADERS_INSTALLED = $(subst inc/,$(PREFIX)/include/,$(HEADERS))
TARGET_LIBJAYC_INSTALLED = $(PREFIX)/lib/$(subst build/lib/,,$(TARGET_LIBJAYC))
TARGET_EXEC_INSTALLED = $(PREFIX)/bin/$(subst build/bin/,,$(TARGET_EXEC))
TARGET_EXECD_INSTALLED = $(PREFIX)/sbin/$(subst build/sbin/,,$(TARGET_EXECD))

# ==============================================================================
# Build recipes

# ------------------------------------------------------------------------------
# Compile Tests
check: $(OBJ)
	@echo "Source Code compiled successfully."

# ------------------------------------------------------------------------------
# Build all parts of the project
.PHONY: all
all: libs bins

# ------------------------------------------------------------------------------
# Compile Executables
.PHONY: bins
bins: $(TARGET_EXEC) $(TARGET_EXECD)
	@echo "All executables done."

build/bin/%: exec/%.c $(OBJ)
	$(CC) $? -o $@ $(INC) $(LIB) $(LDFLAGS) $(CFLAGS)

build/sbin/%: execd/%.c $(OBJ)
	$(CC) $? -o $@ $(INC) $(LIB) $(LDFLAGS) $(CFLAGS)

# ------------------------------------------------------------------------------
# Compile Library
.PHONY: libs
libs: $(TARGET_LIBJAYC)
	@echo "All libraries done."

$(TARGET_LIBJAYC): $(OBJ)
	$(CC) -shared -o $@ $(CFLAGS) $? $(LDFLAGS)

# ------------------------------------------------------------------------------
# Library Installation
.PHONY: install
install: install_lib install_inc install_bin install_sbin
	@echo "Installation finished."

install_lib: $(TARGET_LIBJAYC) preinstall
	install -m 755 $(TARGET_LIBJAYC) $(PREFIX)/lib/

install_inc: preinstall
	for header in $(HEADERS); do install -m 755 $$header $(PREFIX)/include/; done

install_bin: preinstall $(TARGET_EXEC)
	# for binary in $(TARGET_EXEC); do install -m 755 $$binary $(PREFIX)/bin/; done

install_sbin: preinstall $(TARGET_EXECD)
	for binary in $(TARGET_EXECD); do install -m 755 $$binary $(PREFIX)/sbin/; done

preinstall:
	install -d $(PREFIX)/include/
	install -d $(PREFIX)/lib/
	install -d $(PREFIX)/bin/
	install -d $(PREFIX)/sbin/

# ------------------------------------------------------------------------------
# Library Uninstall
.PHONY: uninstall
uninstall: uninstall_lib uninstall_inc uninstall_bin uninstall_sbin
	@echo "Uninstallation finished."

uninstall_lib:
	rm -f $(TARGET_LIBJAYC_INSTALLED)

uninstall_inc:
	for header in $(HEADERS_INSTALLED); do rm -f $$header; done

uninstall_bin:
	# for binary in $(TARGET_EXEC_INSTALLED); do rm -f $$binary; done

uninstall_sbin:
	for binary in $(TARGET_EXECD_INSTALLED); do rm -f $$binary; done

# ------------------------------------------------------------------------------
# Make Documentation
.PHONY: docs
docs: doc_doxygen
	@echo "Documentation done."

doc_md: 
	

doc_doxygen: Doxyfile
	doxygen Doxyfile

# ------------------------------------------------------------------------------
# Make Tests
# all_tests: $(TARGET_TESTS)
# 	@echo "All tests done."

# build/tests/%: tests/%.c $(OBJ)
# 	$(CC) $? -o $@ $(INC) $(LIB)

# ------------------------------------------------------------------------------
# General Recipes

build/objects/%.o: src/%.c build
	$(CC) -c $< -o $@ $(CFLAGS) $(BUILD_FLAGS)

build:
	mkdir -p build/objects
	mkdir -p build/tests
	mkdir -p build/lib
	mkdir -p build/bin
	mkdir -p build/sbin

clean:
	rm -rf $(TARGET_TESTS) $(TARGET_LIBJAYC) $(OBJ) docs/doxygen/ build/
	clear
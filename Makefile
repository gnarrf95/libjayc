# ==============================================================================
# Compiler

# Change compiler using "make COMPILER=<other-compiler>"
COMPILER ?= gcc
CC = $(COMPILER) -Wall -Werror -std=c11

# ==============================================================================
# Compiler and linker flags

INC = -I inc/

CFLAGS = $(INC)

LDF_PTHREAD = -lpthread
LDF_CRYPTO = -lcrypto

LDFLAGS = $(LDF_PTHREAD) $(LDF_CRYPTO)

# Use build flags to change compilation parameters for library.
# Flags:
# * "-D JCON_NO_DEBUG"  if jcon modules should not log debug messages
# * "-D JUTIL_NO_DEBUG" if jutil modules should not log debug messages
# * "-D JLOG_EXIT_ATCRITICAL" if program should exit at critical log
# * "-D JLOG_EXIT_ATERROR" if program should exit at error log
BUILD_FLAGS = -DJUTIL_NO_DEBUG

HEADERS_LIB = $(wildcard inc/jayc/*.h)
HEADERS_INT = $(wildcard inc/*.h)

HEADERS = $(HEADERS_LIB) $(HEADERS_INT)

# Get all subdirectories of src
# https://stackoverflow.com/questions/13897945/wildcard-to-obtain-list-of-all-directories
SRCDIRS = $(sort $(dir $(wildcard src/*/)))

SRC = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)*.c))
SRC_EXEC = $(wildcard exec/*.c)
SRC_EXECD = $(wildcard execd/*.c)

# ==============================================================================
# Objects

OBJ = $(subst src, build/objects, $(SRC:.c=.o))

OBJDIRS = $(sort $(dir $(OBJ)))

# ==============================================================================
# Targets

TARGET_LIBJAYC = build/lib/libjayc.so
TARGET_EXEC = $(subst exec,build/bin,$(SRC_EXEC:.c=))
TARGET_EXECD = $(subst execd,build/sbin,$(SRC_EXECD:.c=))

# ==============================================================================
# Install variables

PREFIX ?= /usr/local

HEADERS_INSTALLED = $(subst inc/jayc/,$(PREFIX)/include/jayc/,$(HEADERS_INST))
TARGET_LIBJAYC_INSTALLED = $(subst build/lib,$(PREFIX)/lib,$(TARGET_LIBJAYC))
TARGET_EXEC_INSTALLED = $(subst build/bin,$(PREFIX)/bin,$(TARGET_EXEC))
TARGET_EXECD_INSTALLED = $(subst build/bin,$(PREFIX)/bin,$(TARGET_EXECD))

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

build/bin/%: exec/%.c $(TARGET_LIBJAYC)
	$(CC) $< -o $@ $(INC) -Lbuild/lib/ -ljayc

build/sbin/%: execd/%.c $(TARGET_LIBJAYC)
	$(CC) $< -o $@ $(INC) -Lbuild/lib/ -ljayc

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
	ldconfig

install_inc: preinstall
	for header in $(HEADERS); do install -m 755 $$header $(PREFIX)/include/jayc/; done

install_bin: preinstall $(TARGET_EXEC)
	for binary in $(TARGET_EXEC); do install -m 755 $$binary $(PREFIX)/bin/; done

install_sbin: preinstall $(TARGET_EXECD)
	for binary in $(TARGET_EXECD); do install -m 755 $$binary $(PREFIX)/sbin/; done

preinstall:
	install -d $(PREFIX)/include/jayc/
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
	rm -rf $(PREFIX)/include/jayc

uninstall_bin:
	for binary in $(TARGET_EXEC_INSTALLED); do rm -f $$binary; done

uninstall_sbin:
	for binary in $(TARGET_EXECD_INSTALLED); do rm -f $$binary; done

# ------------------------------------------------------------------------------
# Make Documentation
.PHONY: docs
docs: doc_doxygen
	@echo "Documentation done."

doc_md: 
	# Need to add usage and dev docs.

doc_doxygen: Doxyfile
	doxygen Doxyfile

# ------------------------------------------------------------------------------
# General Recipes

build/objects/%.o: src/%.c build
	$(CC) -c $< -o $@ $(CFLAGS) $(BUILD_FLAGS) -fpic

build:
	mkdir -p build/objects
	mkdir -p build/tests
	mkdir -p build/lib
	mkdir -p build/bin
	mkdir -p build/sbin
	for dir in $(OBJDIRS); do mkdir -p $$dir; done

clean:
	rm -rf docs/doxygen/ build/
	clear
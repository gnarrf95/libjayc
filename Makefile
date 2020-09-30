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

HEADERS_INST = $(wildcard inc/jayc/*.h)
HEADERS = $(wildcard inc/*.h) $(HEADERS_INST)

# Get all subdirectories of src
# https://stackoverflow.com/questions/13897945/wildcard-to-obtain-list-of-all-directories
SRCDIRS = $(sort $(dir $(wildcard src/*/)))

SRC = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)*.c))
SRC_TESTS = $(wildcard tests/*.c)
SRC_EXEC = $(wildcard exec/*.c)
SRC_EXECD = $(wildcard execd/*.c)

# ==============================================================================
# Objects

OBJ = $(subst src, build/objects, $(SRC:.c=.o))

OBJDIRS = $(sort $(dir $(OBJ)))

# ==============================================================================
# Targets

TARGET_LIBJAYC = build/lib/libjayc.so
# TARGET_TESTS = $(subst tests,build/tests,$(SRC_TESTS:.c=))
TARGET_EXEC = $(subst exec,build/bin,$(SRC_EXEC:.c=))
TARGET_EXECD = $(subst execd,build/sbin,$(SRC_EXECD:.c=))

# ==============================================================================
# Install variables

PREFIX ?= /usr/local

HEADERS_INSTALLED = $(subst inc/jayc/,$(PREFIX)/include/jayc/,$(HEADERS_INST))
TARGET_LIBJAYC_INSTALLED = $(PREFIX)/lib/$(subst build/lib/,,$(TARGET_LIBJAYC))
TARGET_EXEC_INSTALLED = $(PREFIX)/bin/$(subst build/bin/,,$(TARGET_EXEC))
TARGET_EXECD_INSTALLED = $(PREFIX)/sbin/$(subst build/sbin/,,$(TARGET_EXECD))

# ==============================================================================
# Build recipes

srcs:
	@echo $(OBJDIRS)

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
	for header in $(HEADERS_INST); do install -m 755 $$header $(PREFIX)/include/jayc/; done

install_bin: preinstall $(TARGET_EXEC)
	# for binary in $(TARGET_EXEC); do install -m 755 $$binary $(PREFIX)/bin/; done

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
	for dir in $(OBJDIRS); do mkdir -p $$dir; done

clean:
	rm -rf docs/doxygen/ build/
	clear

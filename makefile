# Compiler
CC = gcc -Wall -Werror -std=c11 -fPIC

#Compiler and linker flags
INC = -I inc/
CFLAGS = `mysql_config --cflags`
LIB_JANSSON = -ljansson
LIB_PTHREAD = -lpthread
LIB_MYSQL = `mysql_config --libs`
LIB = $(LIB_PTHREAD)

# Sources
HEADERS = $(wildcard inc/*.h)

SRC = $(wildcard src/*.c)
SRC_TESTS = $(wildcard tests/*.c)

# Objects
OBJ = $(subst src, build/objects, $(SRC:.c=.o))

# Targets
TARGET_TESTS = $(subst tests, build/tests, $(SRC_TESTS:.c=))
TARGET_LIBJAYC = build/lib/libjayc.so

# Install variables
PREFIX = ./build/usr
ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

HEADERS_INSTALLED = $(subst inc/,$(PREFIX)/include/,$(HEADERS))
TARGET_LIBJAYC_INSTALLED = $(PREFIX)/lib/$(subst build/lib/,,$(TARGET_LIBJAYC))

# Build recipes

compile_test: $(OBJ)
	@echo "Source Code compiled successfully."

install: install_lib install_inc
	@echo "Installation finished."

install_lib: $(TARGET_LIBJAYC) preinstall
	install -m 644 $(TARGET_LIBJAYC) $(PREFIX)/lib/

install_inc: preinstall
	for header in $(HEADERS); do install -m 644 $$header $(PREFIX)/include/; done

preinstall:
	install -d $(PREFIX)/include/
	install -d $(PREFIX)/lib/

uninstall: uninstall_lib uninstall_inc
	@echo "Uninstallation finished."

uninstall_lib:
	rm -f $(TARGET_LIBJAYC_INSTALLED)

uninstall_inc:
	for header in $(HEADERS_INSTALLED); do rm $$header; done

all_docs: doc_doxygen
	@echo "Documentation done."

doc_md: 
	

doc_doxygen: Doxyfile
	doxygen Doxyfile

all_tests: $(TARGET_TESTS)
	@echo "All tests done."

all_libs: $(TARGET_LIBJAYC)
	@echo "All libraries done."

$(TARGET_LIBJAYC): $(OBJ)
	$(CC) -shared -o $@ $(INC) $? $(LIB)

build/tests/%: tests/%.c $(OBJ)
	$(CC) $? -o $@ $(INC) $(LIB)

build/objects/%.o: src/%.c build
	$(CC) -c $< -o $@ $(INC) $(CFLAGS)

build:
	mkdir -p build/objects
	mkdir -p build/tests
	mkdir -p build/lib
	mkdir -p build/bin

clean:
	rm -rf $(TARGET_TESTS) $(TARGET_LIBJAYC) $(OBJ) docs/doxygen/ build/
	clear
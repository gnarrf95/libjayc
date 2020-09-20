CC = gcc -Wall -Werror -std=c11 -fPIC

INC = -I inc/

CFLAGS = `mysql_config --cflags`

LIB_JANSSON = -ljansson
LIB_PTHREAD = -lpthread
LIB_MYSQL = `mysql_config --libs`

LIB = $(LIB_PTHREAD)

SRC = $(wildcard src/*.c)
SRC_TESTS = $(wildcard tests/*.c)

OBJ = $(subst src, build/objects, $(SRC:.c=.o))

TARGET_TESTS = $(subst tests, build/tests, $(SRC_TESTS:.c=))

TARGET_LIBJAYC = build/lib/libjayc.so

compile_test: $(OBJ)
	@echo "Source Code compiled successfully."

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
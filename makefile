CC = gcc -Wall -Werror -std=c11 -fPIC
CXX = g++ -Wall -Werror -std=c++11 -fPIC

INC = -I inc/

CFLAGS = `mysql_config --cflags`
CXXFLAGS = `mysql_config --cxxflags`

LIB_JANSSON = -ljansson
LIB_PTHREAD = -lpthread
LIB_MYSQL = `mysql_config --libs`

LIB = $(LIB_PTHREAD)

SRC_C = $(wildcard src/*.c)
SRC_CPP = $(wildcard src/*.cpp)
SRC_TESTS_C = $(wildcard tests/*.c)
SRC_TESTS_CPP = $(wildcard tests/*.cpp)

OBJ_C = $(subst src, build/objects, $(SRC_C:.c=.o))
OBJ_CPP = $(subst src, build/objects, $(SRC_CPP:.cpp=.o))

TARGET_TESTS_C = $(subst tests, build/tests, $(SRC_TESTS_C:.c=))
TARGET_TESTS_CPP = $(subst tests, build/tests, $(SRC_TESTS_CPP:.cpp=))

TARGET_LIBJAYC = build/lib/libjayc.so

compile_test: $(OBJ_C) $(OBJ_CPP)
	@echo "Source Code compiled successfully."

all_docs: doc_doxygen
	@echo "Documentation done."

doc_md: 
	

doc_doxygen: Doxyfile
	doxygen Doxyfile

all_tests: $(TARGET_TESTS_C) $(TARGET_TESTS_CPP)
	@echo "All tests done."

all_libs: $(TARGET_LIBJAYC)
	@echo "All libraries done."

$(TARGET_LIBJAYC): $(OBJ_C) $(OBJ_CPP)
	$(CC) -shared -o $@ $(INC) $? $(LIB)

build/tests/%: tests/%.c $(OBJ_C)
	$(CC) $? -o $@ $(INC) $(LIB)

build/tests/%: tests/%.cpp $(OBJ_C) $(OBJ_CPP)
	$(CXX) $? -o $@ $(INC)

build/objects/%.o: src/%.c build
	$(CC) -c $< -o $@ $(INC) $(CFLAGS)

build/objects/%.o: src/%.cpp build
	$(CXX) -c $< -o $@ $(INC) $(CXXFLAGS)

build:
	mkdir -p build/objects
	mkdir -p build/tests
	mkdir -p build/lib
	mkdir -p build/bin

clean:
	rm -rf $(TARGET_TESTS_C) $(TARGET_TESTS_CPP) $(TARGET_LIBJAYC) $(OBJ_C) $(OBJ_CPP) docs/doxygen/ build/
	clear
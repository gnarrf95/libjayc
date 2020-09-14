CC = gcc -Wall -Werror -std=c11
CXX = g++ -Wall -Werror -std=c++11

INC = -I inc/

SRC_C = $(wildcard src/*.c)
SRC_CPP = $(wildcard src/*.cpp)
SRC_TESTS_C = $(wildcard tests/*.c)
SRC_TESTS_CPP = $(wildcard tests/*.cpp)

OBJ_C = $(subst src, build/objects, $(SRC_C:.c=.o))
OBJ_CPP = $(subst src, build/objects, $(SRC_CPP:.cpp=.o))

TARGET_TESTS_C = $(subst tests, build/tests, $(SRC_TESTS_C:.c=))
TARGET_TESTS_CPP = $(subst tests, build/tests, $(SRC_TESTS_CPP:.cpp=))

all_tests: $(TARGET_TESTS_C) $(TARGET_TESTS_CPP)
	@echo "All tests done."

build/tests/%: tests/%.c $(OBJ_C)
	$(CC) $? -o $@ $(INC)

build/tests/%: tests/%.cpp $(OBJ_C) $(OBJ_CPP)
	$(CXX) $? -o $@ $(INC)

build/objects/%.o: src/%.c build
	$(CC) -c $< -o $@ $(INC)

build/objects/%.o: src/%.cpp build
	$(CXX) -c $< -o $@ $(INC)

build:
	mkdir -p build/objects
	mkdir -p build/tests
	mkdir -p build/lib
	mkdir -p build/bin

clean:
	rm -rf $(TARGET_TESTS_C) $(TARGET_TESTS_CPP) $(OBJ_C) $(OBJ_CPP)
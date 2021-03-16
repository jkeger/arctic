# 
# 	Makefile for ArCTIC
# 
# 	Options
# 	-------
# 	arctic
# 		(Default) The main program. See src/main.cpp.
# 
# 	test, test_arctic
# 		The unit tests. See test/*.cpp.
# 
# 	lib, libarctic.so
# 		The shared library object.
# 
# 	lib_test
# 		A test for using the shared library. See python/lib_test.cpp.
# 
# 	all
# 		All of the above.
# 

# Compiler
CXX := g++
CXXFLAGS := -std=c++11 -fPIC -O3 -Wall -Wno-reorder -Wno-sign-compare
# CXXFLAGS := -std=c++11 -pg -no-pie -fno-builtin       # for gprof
# CXXFLAGS := -std=c++11 -g                             # for valgrind
LDFLAGS := -shared

# Executables
TARGET := arctic
TEST_TARGET := test_arctic
LIB_TARGET := libarctic.so
LIB_TEST_TARGET := lib_test

# Directories
DIR_SRC := src/
DIR_OBJ := build/
DIR_INC := include/
DIR_TEST := test/
DIR_LIB_TEST := python/
INCLUDE := -I $(DIR_INC)
$(shell mkdir -p $(DIR_OBJ))

# Source and object files, and dependency files to detect header file changes
SOURCES := $(shell find $(DIR_SRC) -type f -name *.cpp)
OBJECTS := $(patsubst $(DIR_SRC)%, $(DIR_OBJ)%, $(SOURCES:.cpp=.o))
DEPENDS := $(patsubst %.o, %.d, $(OBJECTS))
TEST_SOURCES := $(shell find $(DIR_TEST) -type f -name *.cpp)
TEST_OBJECTS := $(patsubst $(DIR_TEST)%, $(DIR_OBJ)%, $(TEST_SOURCES:.cpp=.o)) \
	$(filter-out $(DIR_OBJ)main.o, $(OBJECTS))
TEST_DEPENDS := $(patsubst %.o, %.d, $(TEST_OBJECTS))

# Ignore any files with these names
.PHONY: all lib test lib_test clean

# Default to main program
.DEFAULT_GOAL := $(TARGET)

# Main program, unit tests, library, and library test
all: $(TARGET) $(TEST_TARGET) $(LIB_TARGET) $(LIB_TEST_TARGET)

# Main program
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

-include $(DEPENDS)

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

# Unit tests
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

-include $(TEST_DEPENDS)

$(DIR_OBJ)%.o: $(DIR_TEST)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

# Shared library
lib: $(LIB_TARGET)

$(LIB_TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Test using the shared library
$(LIB_TEST_TARGET): $(LIB_TARGET)
	$(CXX) $(INCLUDE) -L./ -l$(TARGET) $(DIR_LIB_TEST)$@.cpp -o $(DIR_LIB_TEST)$@

clean:
	@rm -fv $(OBJECTS) $(DEPENDS) $(TEST_OBJECTS) $(TEST_DEPENDS)
	@rm -fv $(TARGET) $(TEST_TARGET) $(LIB_TARGET)

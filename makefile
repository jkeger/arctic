
# Compiler
CXX := g++
CXXFLAGS := -std=c++11 -O3 #-Wall
# CXXFLAGS := -std=c++11 -pg -no-pie -fno-builtin		# Profiling with gprof
# CXXFLAGS := -std=c++11 -g 							# Profiling with valgrind

# Executables
TARGET := arctic
TEST_TARGET := test_arctic

# Directories
DIR_SRC := src/
DIR_OBJ := build/
DIR_INC := include/
DIR_TEST := test/
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
.PHONY: all test clean

# Default to main program
.DEFAULT_GOAL := $(TARGET)

# Main program and unit tests
all: $(TARGET) $(TEST_TARGET)

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

clean:
	@rm -fv $(OBJECTS) $(DEPENDS) $(TEST_OBJECTS) $(TEST_DEPENDS)
	@rm -fv $(TARGET) $(TEST_TARGET)

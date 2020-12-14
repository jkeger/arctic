
# Compiler
CXX := g++
CXXFLAGS := -std=c++11 #-Wall

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

# Source and object files
SOURCES := $(shell find $(DIR_SRC) -type f -name *.cpp)
OBJECTS := $(patsubst $(DIR_SRC)%, $(DIR_OBJ)%, $(SOURCES:.cpp=.o))
TEST_SOURCES := $(shell find $(DIR_TEST) -type f -name *.cpp)
TEST_OBJECTS := $(patsubst $(DIR_TEST)%, $(DIR_OBJ)%, $(TEST_SOURCES:.cpp=.o)) \
	$(filter-out $(DIR_OBJ)main.o, $(OBJECTS))

# Main program
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

# Main program and tests
all: $(TARGET) $(TEST_TARGET)

# Tests
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

$(DIR_OBJ)%.o: $(DIR_TEST)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

clean:
	@rm -fv $(DIR_OBJ)*.o
	@rm -fv $(TARGET) $(TEST_TARGET)


# Compiler
CXX := g++
CXXFLAGS := -Wall

# Executables
TARGET := arctic
TEST_TARGET := test_arctic

# Directories
DIR_SRC := src/
DIR_OBJ := build/
DIR_INC := include/
INCLUDE := -I $(DIR_INC)
$(shell mkdir -p $(DIR_OBJ))

# Source and object files
SOURCES := $(shell find $(DIR_SRC) -type f -name *.cpp)
OBJECTS := $(patsubst $(DIR_SRC)%, $(DIR_OBJ)%, $(SOURCES:.cpp=.o))

# Main program
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

clean:
	@rm -fv $(DIR_OBJ)*.o
	@rm -fv $(TARGET)

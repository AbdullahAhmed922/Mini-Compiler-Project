# ============================================================
# Makefile — Build System
# Person 6's Module
# Usage:
#   make          → compile the compiler
#   make run      → compile and run with input.txt
#   make clean    → remove build artifacts
# ============================================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = compiler
SOURCES = main.cpp lexer.cpp symbol_table.cpp parser.cpp semantic.cpp icg.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default: build the compiler
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile and run with default input
run: $(TARGET)
	./$(TARGET) input.txt

# Clean build artifacts
clean:
	del /Q *.o $(TARGET).exe 2>nul || rm -f *.o $(TARGET)

.PHONY: all run clean

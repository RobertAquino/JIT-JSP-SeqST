CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O0 -g -Iinclude -MMD -MP

TARGET := bin/tabu_scheduler
SRC := main.cpp $(wildcard src/*.cpp)
OBJ := $(patsubst %.cpp,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ) -o $@

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(INSTANCE)

clean:
	rm -rf build bin

rebuild: clean all

-include $(DEP)
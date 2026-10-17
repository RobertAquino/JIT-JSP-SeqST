CXX := g++

CPLEX_HOME := /opt/ibm/ILOG/CPLEX_Studio222

CXXFLAGS := -std=c++17 -Wall -Wextra -O0 -g \
            -Iinclude \
            -I$(CPLEX_HOME)/cplex/include \
            -I$(CPLEX_HOME)/concert/include \
            -MMD -MP

LDFLAGS := \
	-L$(CPLEX_HOME)/cplex/lib/x86-64_linux/static_pic \
	-L$(CPLEX_HOME)/concert/lib/x86-64_linux/static_pic

LDLIBS := \
	-lilocplex \
	-lconcert \
	-lcplex \
	-lm \
	-lpthread \
	-ldl

TARGET := bin/tabu_scheduler

SRC := main.cpp $(wildcard src/*.cpp)

OBJ := $(patsubst %.cpp,build/%.o,$(SRC))

DEP := $(OBJ:.o=.d)

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(INSTANCE)

clean:
	rm -rf build bin

rebuild: clean all

-include $(DEP)
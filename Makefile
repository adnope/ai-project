CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -Iinclude -MMD -MP
LDFLAGS :=
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32 -lwsock32
endif

# Source files
MAIN_SRC := src/main.cpp
GENERATOR_SRC := src/generator.cpp
WARMUP_SRC := src/warmup_generator.cpp

# Targets
MAIN_TARGET := $(BUILD_DIR)/main
GENERATOR_TARGET := $(BUILD_DIR)/generator
WARMUP_TARGET := $(BUILD_DIR)/warmup_generator

# Object files
MAIN_OBJ := $(OBJ_DIR)/main.o
GENERATOR_OBJ := $(OBJ_DIR)/generator.o
WARMUP_OBJ := $(OBJ_DIR)/warmup_generator.o

# Dependency files
DEPS := $(MAIN_OBJ:.o=.d) $(GENERATOR_OBJ:.o=.d) $(WARMUP_OBJ:.o=.d)

all: $(MAIN_TARGET) $(GENERATOR_TARGET) $(WARMUP_TARGET)

# Main executable
$(MAIN_TARGET): $(MAIN_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

# Generator executable
$(GENERATOR_TARGET): $(GENERATOR_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

# Warmup executable
$(WARMUP_TARGET): $(WARMUP_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

# Compile any CPP file to object
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directories
$(OBJ_DIR):
	@mkdir -p "$(OBJ_DIR)" "$(BUILD_DIR)"

# Include dependency rules
-include $(DEPS)

# Run targets
run: $(MAIN_TARGET)
	@./$(MAIN_TARGET) $(ARGS)

generate: $(GENERATOR_TARGET)
	@./$(GENERATOR_TARGET) $(ARGS)

warmup: $(WARMUP_TARGET)
	@./$(WARMUP_TARGET)

clean:
	rm -rf "$(BUILD_DIR)"

.PHONY: all run generate warmup clean
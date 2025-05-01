CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS :=
BUILD_DIR := build

ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32 -lwsock32
endif

SRC_DIR := src
MAIN_SRC := $(SRC_DIR)/main.cpp
GENERATOR_SRC := $(SRC_DIR)/generator.cpp
WARMUP_SRC := $(SRC_DIR)/warmup_generator.cpp

MAIN_TARGET := $(BUILD_DIR)/main
GENERATOR_TARGET := $(BUILD_DIR)/generator
WARMUP_TARGET := $(BUILD_DIR)/warmup_generator

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)

all: clean $(MAIN_TARGET) $(GENERATOR_TARGET) $(WARMUP_TARGET)

$(MAIN_TARGET): $(MAIN_SRC) | $(BUILD_DIR)
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)

$(GENERATOR_TARGET): $(GENERATOR_SRC) | $(BUILD_DIR)
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)

$(WARMUP_TARGET): $(WARMUP_SRC) | $(BUILD_DIR)
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR):
	@mkdir -p "$@"

run: $(MAIN_TARGET)
	@./$(MAIN_TARGET) $(ARGS)

generate: $(GENERATOR_TARGET)
	@./$(GENERATOR_TARGET) $(ARGS)

warmup: $(WARMUP_TARGET)
	@./$(WARMUP_TARGET)

clean:
	rm -rf "$(BUILD_DIR)"
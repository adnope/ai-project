CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32 -lwsock32
else
    LDFLAGS =
endif

TARGET = main
SRC = main.cpp

all: clean $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f "$(TARGET)"

run: all
	./$(TARGET) $(ARGS)

generate:
	$(CXX) generator.cpp -o generator $(CXXFLAGS)
	./generator $(ARGS)

warmup:
	$(CXX) warmup_generator.cpp -o warmup_generator $(CXXFLAGS)
	./warmup_generator
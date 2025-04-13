CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = main
SRC = main.cpp

all: clean $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f "$(TARGET)"

run: all
	./$(TARGET) $(ARGS)

generate:
	$(CXX) $(CXXFLAGS) -o generator generator.cpp
	./generator $(ARGS)
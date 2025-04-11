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
	g++ generator_blog.cpp -o generator_blog
	./generator_blog
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = solver
SRC = Solver.cpp

all: clean solver

solver: $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) Solver.cpp

clean:
	rm -f "$(TARGET)"

run: all
	./$(TARGET) $(ARGS)
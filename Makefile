<<<<<<< HEAD
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
=======
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = main

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)
>>>>>>> 63447906801ba2dcfbff5e967870c3120b624702

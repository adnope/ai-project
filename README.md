# Connect 4 game solver - an AI project for UET INT3401E

## Launch instructions:

### Compile executable (solver):

```
make
```

### Clean executables:

```
make clean
```

### Run the solver:

```
make run
```

Additionally, you can run ```make run ARGS="arg"``` to launch the solver with diffrent modes (remove the arguments with -t or -f)

The solver currently has 2 modes:

- (Default mode: -f): Find the best move and calculate the score: It prints out the best move to make and the score of the current game position. Additionally, it also prints the number of nodes explored, the number of moves on the board, and the time it takes to find the best move.

- -t: Run the solver with the test provided in /tests. You can modify the runTest() function in the solver main function to run other tests.

- -b: Test board input mode: Allows you to input a Connect 4 board state manually and converts it to a move sequence. The board should be input row by row from top to bottom (7x6), using 0 for empty cells, 1 for player 1's pieces, and 2 for player 2's pieces.

- -s: Run sequence-to-board-to-sequence testing: Processes test cases from begin_medium_test.txt, converts move sequences to boards and back, and writes detailed results to results.txt including success rate and timing information.

- -q: Test sequence-to-board conversion: Allows you to input a move sequence and see the resulting board state. The sequence should consist of column numbers (1-7).
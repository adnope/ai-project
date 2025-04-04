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
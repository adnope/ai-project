# Connect 4 game solver - an AI project for UET INT3401E

## Compile instructions:

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

Additionally, you can run ```make run ARGS="<arg>"``` to launch the solver with diffrent modes (replace the ```<arg>``` with -t or -f)

The solver currently has 4 modes:

- (Default mode: -f, --find): Find the best move and calculate the score: It prints out the best move to make and the score of the current game position. Additionally, it also prints the number of nodes explored, the number of moves on the board, and the time it takes to find the best move.

- (Test mode) -t, --test: Run the solver with the test provided in ```/tests```. You can modify the ```runTest()``` function in the solver main function to run other tests.

- (Play mode) -p, --play: Launch the game, the player could choose to be red or yellow, and the game will start with the AI as the opponent.

- (Continuous find) -c, --cfind: Only 1 position exists, the user inputs moves continuously and the solver will find the best move for the current position.

- (Bot versus bot) -b, --botgame: Create 2 bots and make them play against each other. You could see the boards, the moves they make and time elapsed. 

- (Training mode) -tr, --training: Used for generating the hard moves (moves that take more than 2 seconds) to add to the warmup moves book. This mode basically is a bot game but the board is reset whenever it reaches moves 14 (modifiable), making it an infinite loop.

Alternatively, if you already compiled the solver first using ```make```, you could just run the executable with the corresponding argument. For example:
```
If you're on Windows:
.\solver.exe -f
.\solver.exe --find

Or if you're on Linux:
./solver --play
./solver -t
./solver --test
```

## Platform connection:

The AI is connected to the competition platform by app.py. It is a uvicorn app which listens on port 8111. Feel free to deploy the AI however you like.
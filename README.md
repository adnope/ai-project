<a id="about"></a>
# Connect 4 game solver - an AI project for UET INT3401E

## Table of content
1. [**About**](#about)
2. [**Compile instructions**](#compile)
3. [**Launch instructions**](#launch)
4. [**Platform connection**](#connection)

<a id="compile"></a>
## Compile instructions:

### Prerequisites:

- cmake
- g++

### Using project env (Linux only):

```
source projenv
```
Build executables:
```
build
```
Clean executables:
```
clean
```
Run the main executable with arguments:
```
run <ARGS>
```

### Using cmake:

```
mkdir build
cmake -B build .
cmake --build build
```

<a id="launch"></a>
## Launch instructions:

Just run the executable in `build/`

If you're on Windows:
```
.\build\main.exe <ARGS>
```

Or if you're on Linux:
```
./build/main <ARGS>
```
<a id="json-format"></a>
If launched without an argument, the program will run at the default mode (argument `-w`), which is a request handler that takes in a POST request (a game state) in the following JSON format, for example:

```
{
  "board": [
    [0,0,0,0,0,0,0],
    [0,0,0,1,0,0,0],
    [0,0,0,2,0,0,0],
    [0,0,0,1,0,0,0],
    [0,0,2,2,0,2,0],
    [1,2,1,1,0,2,1]
  ],
  "current_player": 1,
  "valid_moves": [0,1,2,3,4,5,6],
  "is_new_game": false
}
```

And it returns a response in this format (the move calculated):

```
{
  "move": 2
}
```

Aside from that, the program supports some more options. Here is the list of all the modes as arguments:

- **Default mode: -w, --web**: Run a request handler from a client and returns a response (as stated [above](#json-format)).

- **Find mode: -f, --find**: The user inputs a sequence representing a board and the program returns the best move for that board. It prints out the best move to make and the score of the current game position. It also prints the number of nodes explored, the number of moves on the board, and the time it takes to find the best move.

- **Continuous find: -c, --cfind**: Only 1 board exists, the user inputs moves continuously to construct the board and the solver will find the best move for the current board.

- **Test mode: -t, --test**: Run the solver with the test provided in ```/tests```. A test is a file containing lines of a sequence and its expected score. The solver then iterates through all the lines and calculates the score for each line, then compares the results. It is used to measure the time taken and the accuracy of the solver. You can modify the ```runTest()``` function in the solver main function to run other tests.

- **Play mode: -p, --play**: Start the game, the player could choose to be either red or yellow and play with our AI bot.

- **Bot versus bot: -b, --botgame**: Create 2 bots and make them play against each other. You could see the board in each move, the moves they make and the time elapsed.

- **Training mode: -tr, --training**: This mode is used for generating the hard moves (moves that take more than 2 seconds) to add to the file ```hard_moves.txt.``` You can then run the ```warmup_generator``` to contribute to the ```warmup.book``` file. Then run the generator to convert this book into binary form, just like the opening book. This mode basically is a bot game but the board is reset whenever it reaches moves 14 (this can be changed), making it an infinite loop.

The program needs the opening book to calculate moves at the early game. There is also a warmup book to store some moves that takes abnormally long to calculate so that the solver can look them up faster, but it is optional. By default the books are generated and saved in `data/`, and you **MUST RUN** the main executable from the project root directory. If you run the executable from anywhere else, or you have your own books to use, specify the path to the book by using the arguments `--opening-book` and `--warmup-book`. For example:
```
./main -f --opening-book <path> --warmup-book <path> # Specify both books
./main -f --opening-book <path> # Specify one book
./main -f <path> <path> # Opening book first, warmup book second
```

<a id="connection"></a>
## Platform connection:

The program is connected to the battle platform by the request handling mode **"-w"**. By default it listens to ```0.0.0.0:8112```, the request and reponse format is mentioned [above](#json-format).
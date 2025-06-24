# C4AI

C4ai is a connect four game solver. It evaluates arbitrary 7x6 board positions, finds the best moves for given states, and allows you to play against our bot.

# Build instructions:

## Prerequisites:

- g++
- cmake
- ninja

## Building:

### Using project env:

**This is just aliases of CMake commands, source the file to enable them:**

* Windows:

  ```
  . .\projenv.ps1
  ```

* Linux:

  ```
  source projenv
  ```

**After that, run these commands:**

* Build executables:

  ```
  build
  ```
* Clean executables:

  ```
  clean
  ```

### Using cmake:

```
cd c4ai
mkdir build
cmake -B build .
cmake --build build
```

## Launch instructions:

**If you use the project env:**

```
c4ai <ARGS>
```

**Or you could just run the executable in `build/bin`:**

* On Windows:
  ```
  .\build\bin\c4ai.exe <ARGS>
  ```

* On Linux:
  ```
  ./build/c4ai <ARGS>
  ```

**The program currently supports 4 modes:**

* **Find mode: -f, --find**: The user inputs a sequence representing a board and the program returns the best move for that board. It prints out the best move to make and the score of the current game position. It also prints the number of nodes explored, the number of moves on the board, and the time it takes to find the best move.

* **Play mode: -p, --play**: Start the game, the player could choose to be either red or yellow and play with our AI bot.

* **Bot versus bot: -b, --botgame**: Create 2 bots and make them play against each other. In each move, you could see the board, the move they make and the time taken for that move.

* **Training mode: -tr, --training**: This mode is used for generating the hard moves (moves that take more than 2 seconds) to add to the file `hard_moves.txt.` You can then run the `generator` to convert them to a book file (see more on generator). This mode basically is a bot game but the board is reset whenever it reaches moves 20 (this can be changed), making it an infinite loop.

The program needs the opening book to calculate moves at the early game. There is also a warmup book to store some moves that takes abnormally long to calculate so that the solver can look them up faster, but it is optional. By default the books are generated and saved in `data/`, and you **MUST RUN** the c4ai executable from the project root directory. If you run the executable from anywhere else, or you have your own books to use, specify the path to the book by using the arguments `--opening-book` and `--warmup-book`. For example:
```
./c4ai -f --opening-book <path> --warmup-book <path> # Specify both books
./c4ai -f --opening-book <path> # Specify one book
./c4ai -f <path> <path> # Opening book first, warmup book second
```
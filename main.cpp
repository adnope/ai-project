#include "Position.hpp"
#include "Solver.cpp"
#include <iostream>
#include <vector>
#include <climits>
#include <chrono>

using namespace std::chrono;

void printBoard(const std::string& moves, const int COLS, const int ROWS) {
    std::vector<std::vector<char>> board(ROWS, std::vector<char>(COLS, ' '));

    bool isRedTurn = true;
    for (char move : moves) {
        int col = move - '1';
        if (col < 0 || col >= COLS) {
            std::cerr << "Invalid move: " << move << "\n";
            continue;
        }

        for (int row = ROWS - 1; row >= 0; --row) {
            if (board[row][col] == ' ') {
                board[row][col] = isRedTurn ? 'x' : 'o';
                isRedTurn = !isRedTurn;
                break;
            }
        }
    }

    for (const auto& row : board) {
        for (char cell : row) {
            std::cout << (cell == ' ' ? '.' : cell) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "1 2 3 4 5 6 7\n";
}

// Currently the board position format is as follow:
// E.g: 4423
// Move 1: red placed at column 4
// Move 2: yellow placed at column 4
// Move 3: red placed at column 2
// Move 4: yellow placed at column 3
int main(int argc, char const *argv[])
{
    Solver solver;
    bool analyze = false;
    bool calculateScore = false;
    bool findBestMove = false;
    bool testMode = false;

    std::string opening_book = "7x6.book";
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'a') // -s to calculate score of given position
            {
                analyze = true;
            }
            else if (argv[i][1] == 's') // -s to calculate score of given position
            {
                calculateScore = true;
            }
            else if (argv[i][1] == 'f')
            {
                findBestMove = true;
            }
            else if (argv[i][1] == 't')
            {
                testMode = true;
            }
        }
    }
    // solver.loadBook(opening_book);

    if (testMode) {
        std::string line;

        for (int l = 1; std::getline(std::cin, line); l++)
        {
            Position P;
            P.play(line);

            auto start = high_resolution_clock::now();
            auto moveAndScore = solver.findBestMove(P);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);

            std::cout << "Board: \n";
            printBoard(line, Position::WIDTH, Position::HEIGHT);
            std::cout << "Current position: " << line << "\n"
                        << "Number of moves: " << P.nbMoves() << "\n"
                        << "Current board score: " << solver.calculateScore(P) << "\n"
                        << "Best move: column " << moveAndScore.first << ", score: " << moveAndScore.second << "\n"
                        << "Time taken to find move: " << duration.count() / 1000 << " ms" << std::endl;
        }
    }

    Position P;

    std::string line;
    std::string current_sequence;
    for (int l = 1; std::getline(std::cin, line); l++)
    {
        unsigned int gameState = P.play(line);
        if (gameState != line.size())
        {
            if (gameState == 200) {
                current_sequence += line;
                std::cout << "Board: \n";
                printBoard(current_sequence, Position::WIDTH, Position::HEIGHT);
                std::cout << "Red wins.";
                return 0;
            }
            else if (gameState == 300) {
                current_sequence += line;
                std::cout << "Board: \n";
                printBoard(current_sequence, Position::WIDTH, Position::HEIGHT);
                std::cout << "Yellow wins.";
                return 0;
            }
            std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
        }
        else
        {
            if (analyze) {
                current_sequence += line;

                auto start = high_resolution_clock::now();
                auto scoresOfMoves = solver.analyzeAllMoves(P);
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(stop - start);

                std::string analysisResult = "";
                for (int col = 0; col < Position::WIDTH; ++col) {
                    analysisResult += (std::to_string(col + 1) + "(" + std::to_string(scoresOfMoves[col]) + "), ");
                }
                analysisResult.erase(analysisResult.size() - 2);

                std::cout << "Board: \n";
                printBoard(current_sequence, Position::WIDTH, Position::HEIGHT);
                std::cout << "Current position: " << current_sequence << "\n"
                          << "Number of moves: " << P.nbMoves() << "\n"
                          << "Current board score: " << solver.calculateScore(P) << "\n"
                          << "Moves (Scores): " << analysisResult << "\n"
                          << "Time taken to analyze: " << duration.count() / 1000 << " ms" << std::endl;
            }
            else if (calculateScore)
            {
                current_sequence += line;

                auto start = high_resolution_clock::now();
                int score = solver.calculateScore(P);
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(stop - start);

                std::cout << "Board: \n";
                printBoard(current_sequence, Position::WIDTH, Position::HEIGHT);
                std::cout << "Current position: " << current_sequence << "\n"
                          << "Number of moves: " << P.nbMoves() << "\n"
                          << "Current board score: " << score << "\n"
                          << "Time taken: " << duration.count() / 1000 << " ms" << std::endl;
            }
            else if (findBestMove)
            {
                current_sequence += line;

                auto start = high_resolution_clock::now();
                auto moveAndScore = solver.findBestMove(P);
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(stop - start);

                std::cout << "Board: \n";
                printBoard(current_sequence, Position::WIDTH, Position::HEIGHT);
                std::cout << "Current position: " << current_sequence << "\n"
                          << "Number of moves: " << P.nbMoves() << "\n"
                          << "Current board score: " << solver.calculateScore(P) << "\n"
                          << "Best move: column " << moveAndScore.first << ", score: " << moveAndScore.second << "\n"
                          << "Time taken to find move: " << duration.count() / 1000 << " ms" << std::endl;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}

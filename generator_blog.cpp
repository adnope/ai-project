#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/Solver.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <fstream>

std::ofstream moves("moves.txt", std::ios::app);

std::unordered_set<uint64_t> visited;
int number_of_moves = 0;

void explore(const Position &P, char *pos_str, const int depth)
{
    uint64_t key = P.key3();
    if (!visited.insert(key).second)
        return; // already explored position

    int nb_moves = P.nbMoves();
    if (nb_moves <= depth)
    {
        // std::cout << pos_str << std::endl;
        moves << pos_str << std::endl;
        number_of_moves++;
    }
    if (nb_moves >= depth)
        return; // do not explore at further depth

    for (int i = 0; i < Position::WIDTH; i++) // explore all possible moves
        if (P.canPlay(i) && !P.isWinningMove(i))
        {
            Position P2(P);
            P2.playCol(i);
            pos_str[nb_moves] = '1' + i;
            explore(P2, pos_str, depth);
            pos_str[nb_moves] = 0;
        }
}

void calculateScore()
{
    std::ifstream moves_file("moves.txt");
    std::ofstream moves_with_scores("moves_with_scores.txt", std::ios::app);

    Solver solver;

    std::string line;
    while (getline(moves_file, line))
    {
        Position P;
        P.play(line);
        int score = solver.solve(P);

        moves_with_scores << line << " " << score << "\n";
    }
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        int depth = atoi(argv[1]);
        char pos_str[depth + 1] = {0};
        explore(Position(), pos_str, depth);
        std::cout << "Number of moves: " << number_of_moves;
    }
    else calculateScore();
}
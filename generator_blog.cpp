#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/Solver.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <fstream>
#include <chrono>

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
    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream moves_file("moves.txt");
    std::ofstream moves_with_scores("moves_with_scores.txt", std::ios::app);

    std::string line;

    for (int i = 1; i <= 1272; i++)
    {
        getline(moves_file, line);
    }

    Solver solver;

    const int check_period = 300;
    int count = 0;
    int next_time = check_period;

    while (getline(moves_file, line))
    {
        Position P;
        P.play(line);
        int score = solver.solve(P);

        moves_with_scores << line << " " << score << "\n";
        count++;

        auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;

        double time_elapsed = duration.count() / 1000;
        if (time_elapsed >= next_time) {
            std::cout << "Time elapsed: " << duration.count() / 1000 << " seconds, " << count << " lines processed\n";
            next_time += check_period;
            moves_with_scores.flush();
        }
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
    else
    calculateScore();
}
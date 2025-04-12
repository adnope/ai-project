#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/Solver.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <fstream>
#include <chrono>

std::unordered_set<uint64_t> visited;
int number_of_explored_moves = 0;

void explore(const Position &P, char *pos_str, const int depth, std::ofstream &explored_moves_stream)
{
    uint64_t key = P.Key3();
    if (!visited.insert(key).second)
        return;

    int nb_moves = P.nbMoves();
    if (nb_moves <= depth)
        explored_moves_stream << pos_str << std::endl;
        number_of_explored_moves++;
    if (nb_moves >= depth)
        return;

    for (int i = 0; i < Position::WIDTH; i++)
        if (P.CanPlay(i) && !P.IsWinningMove(i))
        {
            Position P2(P);
            P2.playCol(i);
            pos_str[nb_moves] = '1' + i;
            explore(P2, pos_str, depth, explored_moves_stream);
            pos_str[nb_moves] = 0;
        }
}

/**
 * Automatically continue from the last session if the program is terminated while processing.
 * CHECK_PERIOD is a period of time (in seconds) which determines how often the program prints out its progress into the console.
 */
void calculateScore(std::string input_file, std::string result_file)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::string line;

    int lines_done = 0;
    std::ifstream input(result_file);
    while (getline(input, line) && !line.empty())
    {
        lines_done++;
    }
    input.close();

    std::ifstream moves_file(input_file);
    std::ofstream moves_with_scores(result_file, std::ios::app);

    for (int i = 1; i <= lines_done; i++)
    {
        getline(moves_file, line);
    }

    Solver solver;

    const int CHECK_PERIOD = 10;
    int count = 0;
    int next_time = CHECK_PERIOD;

    while (getline(moves_file, line))
    {
        Position P;
        P.Play(line);
        int score = solver.Solve(P);

        moves_with_scores << line << " " << score << "\n";
        count++;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        double time_elapsed = duration.count() / 1000;
        if (time_elapsed >= next_time)
        {
            std::cout << "Time elapsed: " << duration.count() / 1000 << " seconds, " << count << " lines processed\n";
            next_time += CHECK_PERIOD;
            moves_with_scores.flush();
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Please enter arguments\n"
                  << "1. Explore and print moves to a file: enter <depth>\n"
                  << "2. Calculate score for the moves: enter <input_file> <result_file>";
        return 1;
    }
    else if (argc == 2)
    {
        std::ofstream moves_explored_stream("moves_explored.txt");
        assert(atoi(argv[1]) >= 0 && atoi(argv[1]) <= 42);
        int depth = atoi(argv[1]);
        char pos_str[depth + 1] = {0};
        explore(Position(), pos_str, depth, moves_explored_stream);
        std::cout << "Number of moves: " << number_of_explored_moves;
        return 0;
    }
    else if (argc == 3)
    {
        std::string input_file = argv[1];
        std::string result_file = argv[2];
        calculateScore(input_file, result_file);
    }

    return 0;
}
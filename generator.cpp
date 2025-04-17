#include "headers/Position.hpp"
#include "headers/Solver.hpp"
#include "headers/OpeningBook.hpp"

#include <iostream>
#include <unordered_set>
#include <fstream>
#include <chrono>
#include <sstream>

/**
 * How to use the generator to generate an opening book:
 *
 * First, you need to run the explore() function to generate all the moves up to a specific depth,
 * which can be accomplished by running: make generate ARGS="depth" (replace 'depth' with your depth).
 * The moves then will be saved to a file called "moves_explored.txt"
 *
 * After that, run the calculateScore() function, which will take your moves file, calculate
 * the score of each moves, then saved all of the scores to an output file (I'll call it results.txt).
 * Run: make generate ARGS="moves_explored.txt results.txt"
 * Note: this step may take a very long time, if you terminate the program while it's running, it
 * will automatically continue from where you left, so don't worry :3
 *
 * When you've got the results.txt file, put it into the project's directory, then the AI should run
 * correctly with "make run..."
 * 
 * The repo already has a sample opening book, which is named "depth_11_scores.book", it is the results
 * file after running explore and calculateScore with depth 11.
 */
void explore(const Position &P, char *pos_str, std::unordered_set<uint64_t> &visited,
             int &number_of_explored_moves, const int depth, std::ofstream &explored_moves_stream)
{
    uint64_t key = P.Key3();
    if (!visited.insert(key).second)
        return;

    int nb_moves = P.nbMoves();
    if (nb_moves <= depth)
    {

        explored_moves_stream << pos_str << std::endl;
        number_of_explored_moves++;
    }
    if (nb_moves > depth)
        return;

    for (int i = 0; i < Position::WIDTH; i++)
        if (P.CanPlay(i) && !P.IsWinningMove(i))
        {
            Position P2(P);
            P2.playCol(i);
            pos_str[nb_moves] = '1' + i;
            explore(P2, pos_str, visited, number_of_explored_moves, depth, explored_moves_stream);
            pos_str[nb_moves] = 0;
        }
}

void calculateScore(char *input_file, char *result_file)
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
    if (!moves_file)
    {
        std::cerr << "Invalid moves file!";
        return;
    }
    std::ofstream moves_with_scores(result_file, std::ios::app);
    if (!result_file)
    {
        std::cerr << "Invalid results file!";
        return;
    }

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

void generateOpeningBook(const std::string& book_name) {
    TranspositionTable* table = new TranspositionTable(268435459);

    std::ifstream in(book_name);
    if (!in) {
        std::cerr << "Cannot open file: " << book_name << std::endl;
        return;
    }

    long long count = 1;
    for (std::string line; std::getline(in, line); count++) {
        if (line.empty()) break;

        std::istringstream iss(line);
        std::string pos;
        int score;

        std::getline(iss, pos, ' ');
        iss >> score;

        Position P;
        if (iss.fail() || !iss.eof()
            || P.Play(pos) != pos.length()
            || score < Position::MIN_SCORE || score > Position::MAX_SCORE) {
            std::cerr << "Invalid line (ignored): " << line << std::endl;
            continue;
        }

        table->Put(P.Key3(), score - Position::MIN_SCORE + 1);

        if (count % 1000000 == 0)
            std::cerr << "Processed " << count << " lines\n";
    }

    OpeningBook book = OpeningBook(table);

    std::ostringstream book_file;
    book_file << "depth_12_scores_7x6.book";
    book.save(book_file.str());
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Please enter arguments\n"
                  << "1. Explore and print moves to a file: enter <depth>\n"
                  << "2. Calculate score for the moves: enter <input_file> <result_file>"
                  << "3. Generate opening book: enter book <input_file>\n";
        return 1;
    }
    else if (argc == 2)
    {
        // Explore
        if (atoi(argv[1]) >= 0 && atoi(argv[1]) <= 42)
        {
            std::ofstream moves_explored_stream("moves_explored.txt");
            std::unordered_set<uint64_t> visited;
            int number_of_explored_moves = 0;

            int depth = atoi(argv[1]);
            char pos_str[depth + 1] = {0};
            explore(Position(), pos_str, visited, number_of_explored_moves, depth, moves_explored_stream);
            std::cout << "Number of moves: " << number_of_explored_moves;
            return 0;
        }
        else {
            std::cerr << "Invalid depth!";
            return 1;
        }

    }
    else if (argc == 3)
    {
        if (std::string(argv[1]) == "book") 
        {
            std::string book_name = argv[2];
            generateOpeningBook(book_name);
        }
        else  
        {
            calculateScore(argv[1], argv[2]);
        }
    }
    

    return 0;
}
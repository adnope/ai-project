#include "../include/header/Position.hpp"
#include "../include/header/Solver.hpp"
#include "../include/header/OpeningBook.hpp"

#include <unordered_set>

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
 * The repo already has a sample opening book, which is named "data/depth_12_scores_7x6.book", it is the results
 * file after running explore and calculateScore with depth 13.
 */
void explore(const Position &P, char *pos_str, std::unordered_set<uint64_t> &visited,
             int &number_of_explored_moves, const int depth, std::ofstream &explored_moves_stream)
{
    uint64_t key = P.Key3();
    if (!visited.insert(key).second)
        return;

    int nb_moves = P.nbMoves();
    if (nb_moves >= 13 && nb_moves <= depth)
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
            P2.PlayCol(i);
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

void convertScoreBookToBinary(const char *input_filename, const char *output_filename)
{
    using key_t = uint64_t;
    using score_t = uint8_t;

    std::string line;
    std::string move_str;
    int score_raw;
    long long line_count = 0;

    std::cout << "Conversion started...\n";

    std::ifstream text_file(input_filename);
    std::ofstream binary_file(output_filename, std::ios::binary);
    while (getline(text_file, line))
    {
        std::istringstream iss(line);
        if (!(iss >> move_str >> score_raw))
        {
            std::cerr << "WARNING: skipping invalid line: " << line << "\n";
            continue;
        }

        Position P;
        P.Play(move_str);
        key_t hashed_move = P.Key3();
        score_t score = score_raw - Position::MIN_SCORE + 1;

        binary_file.write(reinterpret_cast<const char *>(&hashed_move), sizeof(hashed_move));
        binary_file.write(reinterpret_cast<const char *>(&score), sizeof(score));

        line_count++;
        if (line_count % 100000 == 0)
        {
            std::cout << line_count << " lines processed\n";
        }
    }

    std::cout << "Conversion complete! Converted " << line_count << " lines.\n";
    std::cout << "Binary file saved in: " << output_filename << "\n";

    text_file.close();
    binary_file.close();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Please enter arguments\n"
                  << "1. Explore and print moves to a file: enter <depth>\n"
                  << "2. Calculate score for the moves: enter <input_file> <result_file>\n"
                  << "3. Convert score book to binary book: enter convert <input_file> <output_file>\n";
        return 1;
    }
    else if (argc == 2)
    {
        if (atoi(argv[1]) >= 0 && atoi(argv[1]) <= 42)
        {
            std::ofstream moves_explored_stream("data/moves_explored.txt");
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
        calculateScore(argv[1], argv[2]);
    }
    else if (argc == 4)
    {
        if (strcmp(argv[1], "convert") == 0) convertScoreBookToBinary(argv[2], argv[3]);
        else std::cout << "Invalid arguments!\n";
    }

    return 0;
}
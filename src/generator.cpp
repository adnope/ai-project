#include "Position.hpp"
#include "Solver.hpp"
#include "OpeningBook.hpp"

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
            P2.PlayCol(i);
            pos_str[nb_moves] = '1' + i;
            explore(P2, pos_str, visited, number_of_explored_moves, depth, explored_moves_stream);
            pos_str[nb_moves] = 0;
        }
}

void calculateScore(const char *input_file,const char *result_file)
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

int convertScoreBookToBinary(const char *input_filename, const char *output_filename)
{
    using key_t = uint64_t;
    using score_t = uint8_t;

    std::string line;
    std::string move_str;
    int score_raw;
    long long line_count = 0;

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

    text_file.close();
    binary_file.close();

    return line_count;
}

void removeDuplicateLines(const std::string &file_name)
{
    std::ifstream input_file(file_name);
    if (!input_file.is_open())
        return;

    std::vector<std::string> unique_lines;
    std::unordered_set<std::string> seen_lines;
    std::string line;

    while (getline(input_file, line))
    {
        if (seen_lines.insert(line).second)
        {
            unique_lines.push_back(line);
        }
    }
    input_file.close();

    std::ofstream output_file(file_name, std::ios::trunc);
    if (!output_file.is_open())
        return;

    for (const auto &uniqueLine : unique_lines)
    {
        output_file << uniqueLine << "\n";
    }
    output_file.close();
}

void generateWarmupBook(const char* hard_moves_file, const char* book_file)
{
    std::cout << "Generating warmup book...\n";

    removeDuplicateLines(hard_moves_file);
    calculateScore(hard_moves_file, "scores.tmp");
    int warmup_book_num_moves = convertScoreBookToBinary("scores.tmp", book_file);

    std::cout << "Completed generating warmup book for " << warmup_book_num_moves << " hard moves.\n" << "Warmup book saved in: " << book_file << "\n";

    remove("scores.tmp");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Generator: generate opening book & warmup book\n\n"

                  << "Usage:\n"
                  << "  generator [option] [args]\n\n"

                  << "List of options:\n"
                  << "  explore <depth> <moves_file>                    explore all possible moves to a depth, store them in moves_file\n"
                  << "  calculate <moves_file> <scores_file>            calculate the scores of the explored moves, store them in scores_file\n"
                  << "  convert <scores_file> <book_file>               convert the scores_file to proper binary format aka book_file for the solver to read.\n"
                  << "  warmup <hard_moves_file> <warmup_book_file>     generate a binary warmup book from hard_moves.txt from the training mode.\n\n"

                  << "Some examples:\n"
                  << "Generate opening book:\n"
                  << "  generator explore 9 moves.txt\n"
                  << "  generator calculate moves.txt scores.txt\n"
                  << "  generator convert scores.txt opening_binary.book\n"
                  << "Generate warmup book:\n"
                  << "  generator warmup hard_moves.txt warmup_binary.book\n";

        return 1;
    }
    if (strcmp(argv[1], "explore") == 0)
    {
        if (argc != 4)
        {
            std::cout << "Invalid number of arguments.\n"
                      << "Usage: generator explore <depth> <output_file>\n";
            return 1;
        }
        int depth = atoi(argv[2]);
        if (depth < 0 || depth > 42) 
        {
            std::cout << "Invalid depth, depth must be between 0 and 42.\n";
            return 1;
        }
        std::string output_file_path = argv[3];

        std::ofstream moves_explored_stream(output_file_path);
        std::unordered_set<uint64_t> visited;
        int number_of_explored_moves = 0;

        char pos_str[depth + 1] = {0};
        explore(Position(), pos_str, visited, number_of_explored_moves, depth, moves_explored_stream);
        std::cout << "Number of moves: " << number_of_explored_moves << "\n";
        return 0;
    }
    else if (strcmp(argv[1], "calculate") == 0)
    {
        if (argc != 4)
        {
            std::cout << "Invalid number of arguments.\n"
                      << "Usage: generator calculate <moves_file> <scores_file>\n";
            return 1;
        }
        calculateScore(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "convert") == 0)
    {
        if (argc != 4)
        {
            std::cout << "Invalid number of arguments.\n"
                      << "Usage: generator convert <scores_file> <book_file>\n";
            return 1;
        }
        std::cout << "Conversion started...\n";
        int num_moves = convertScoreBookToBinary(argv[2], argv[3]);
        std::cout << "Complete converting " << num_moves << " moves to binary.\nOpening book saved in: " << argv[3] << "\n";
    }
    else if (strcmp(argv[1], "warmup") == 0)
    {
        if (argc != 4)
        {
            std::cout << "Invalid number of arguments.\n"
                      << "Usage: generator warmup <hard_moves_file> <warmup_book_file>\n";
            return 1;
        }
        generateWarmupBook(argv[2], argv[3]);
    }
    else
    {
        std::cout << "Invalid option. List of options are: explore, calculate, convert.\n";
        return 1;
    }

    return 0;
}
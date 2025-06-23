#include "core/position.hpp"
#include "core/solver.hpp"

#include <iostream>
#include <chrono>
#include <cstring>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using std::string;
using std::cout;
using std::ifstream;
using std::ofstream;

/**
 * Instructions:
 * Generating opening book:
 *      First, you need to run the explore() function to generate all the moves from a lower depth
 *      to a higher depth.
 *      Run: generator explore <lower_depth> <higher_depth> <moves_file>
 *      The moves then will be saved to moves_file.
 *
 *      After that, run the calculateScore() function, which will take your moves file, calculate
 *      the score of each move, then save all the scores to scores_file
 *      Run: generator calculate <moves_file> <scores_file>
 *      Note: this step may take a very long time, if you terminate the program while it's running,
 *      it will continue from where you left, so don't worry :3
 *
 *      Finally, you need to convert the scores_file to a binary format (called a book) so that the
 *      solver could read it.
 *      Run: generator convert <scores_file> <book_file>
 *      The repo already has a sample opening book, which is named "data/opening.book", it contains
 *      all moves from depth 1 to depth 12.
 * Generating warmup book:
 *      The warmup book essentially is a moves_file with all the hard moves obtained from a training
 *      session. In the calculateHardMoves() function, the program reads each move, looks at all the
 *      next possible moves from it, then calculates their score. The result is saved in scores_file
 *      Run: generator warmup <hard_moves_file> <scores_file>
 *      After that, convert scores_file into a binary book just like generating an opening book.
 */

void explore(const Position &P, string &pos_str, std::unordered_set<uint64_t> &visited,
             int &number_of_explored_moves, const int lower_depth, const int higher_depth,
             ofstream &explored_moves_stream) {
  const uint64_t key = P.Key3();
  if (!visited.insert(key).second) {
    return;
  }

  const int nb_moves = P.NumMoves();
  if (nb_moves >= lower_depth && nb_moves <= higher_depth) {
    explored_moves_stream << pos_str << '\n';
    ++number_of_explored_moves;
  }
  if (nb_moves > higher_depth) {
    return;
  }

  for (int i = 0; i < Position::WIDTH; ++i) {
    if (P.CanPlay(i) && !P.IsWinningMove(i)) {
      Position P2(P);
      P2.PlayCol(i);

      pos_str.push_back(static_cast<char>(i + '1'));

      explore(P2, pos_str, visited, number_of_explored_moves,
              lower_depth, higher_depth, explored_moves_stream);

      pos_str.pop_back();
    }
  }
}

void calculateScore(const char *input_file, const char *result_file) {
  auto start = std::chrono::high_resolution_clock::now();

  string line;

  int lines_done = 0;
  ifstream input(result_file);
  while (getline(input, line) && !line.empty()) {
    lines_done++;
  }
  input.close();

  ifstream moves_file(input_file);
  if (!moves_file) {
    cout << "Invalid moves file!";
    return;
  }
  ofstream moves_with_scores(result_file, std::ios::app);
  if (!moves_with_scores) {
    cout << "Invalid results file!";
    return;
  }

  for (int i = 1; i <= lines_done; i++) {
    getline(moves_file, line);
  }

  Solver solver;

  constexpr int CHECK_PERIOD = 10;
  int count = 0;
  int next_time = CHECK_PERIOD;

  while (getline(moves_file, line)) {
    Position P;
    P.Play(line);
    int score = solver.Solve(P);

    moves_with_scores << line << " " << score << "\n";
    count++;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    double time_elapsed = duration.count();
    if (time_elapsed >= next_time) {
      cout << "Time elapsed: " << duration.count() << " seconds, "
          << count << " lines processed\n";
      next_time += CHECK_PERIOD;
      moves_with_scores.flush();
    }
  }
}

void removeDuplicateLines(const string &file_name) {
  ifstream input_file(file_name);
  if (!input_file.is_open()) {
    return;
  }

  std::vector<string> unique_lines;
  std::unordered_set<string> seen_lines;
  string line;

  while (getline(input_file, line)) {
    if (seen_lines.insert(line).second) {
      unique_lines.push_back(line);
    }
  }
  input_file.close();

  ofstream output_file(file_name, std::ios::trunc);
  if (!output_file.is_open()) {
    return;
  }

  for (const auto &uniqueLine: unique_lines) {
    output_file << uniqueLine << "\n";
  }
  output_file.close();
}

void calculateHardMoves(const char *hard_moves_file, const char *scores_file) {
  cout << "Calculating scores for hard moves...\n";

  removeDuplicateLines(hard_moves_file);
  ofstream ofs(scores_file, std::ios::app);
  ifstream ifs(hard_moves_file);

  Solver solver;
  string line;
  int count = 0;
  while (getline(ifs, line)) {
    Position P;
    P.Play(line);
    for (int i = 0; i < Position::WIDTH; ++i) {
      Position P2(P);
      if (P2.CanPlay(i)) {
        P2.PlayCol(i);
        int score = solver.Solve(P2);
        string line2 = line;
        line2 += std::to_string(i + 1);
        ofs << line2 << " " << score << "\n";
        count++;
      }
    }
    ofs.flush();
  }

  cout << "Completed calculating scores for " << count << " moves.\n";
}

int convertScoreBookToBinary(const char *input_file, const char *output_file) {
  removeDuplicateLines(input_file);
  using key_t = uint64_t;
  using score_t = uint8_t;

  string line;
  string move_str;
  int score_raw;
  int line_count = 0;

  ifstream text_file(input_file);
  ofstream binary_file(output_file, std::ios::binary);
  while (getline(text_file, line)) {
    std::istringstream iss(line);
    if (!(iss >> move_str >> score_raw)) {
      cout << "WARNING: skipping invalid line: " << line << "\n";
      continue;
    }

    Position P;
    P.Play(move_str);
    key_t hashed_move = P.Key3();
    score_t score = score_raw - Position::MIN_SCORE + 1;

    binary_file.write(reinterpret_cast<const char *>(&hashed_move),
                      sizeof(hashed_move));
    binary_file.write(reinterpret_cast<const char *>(&score),
                      sizeof(score));

    line_count++;
    constexpr int CHECK_PERIOD = 100000;
    if (line_count % CHECK_PERIOD == 0) {
      cout << line_count << " lines processed\n";
    }
  }

  text_file.close();
  binary_file.close();

  return line_count;
}

int main(const int argc, char **argv) {
  if (argc < 2) {
    cout << "Generator: obtaining opening book & warmup book\n\n"

        << "Usage:\n"
        << "  generator [option] [args]\n\n"
        << "List of options:\n"
        <<
        "  explore <lower_depth> <higher_depth> <moves_file>   explore all possible moves from lower_depth to higher_depth, store them in moves_file\n"
        <<
        "  calculate <moves_file> <scores_file>                calculate the scores of the explored moves, store them in scores_file\n"
        <<
        "  convert <scores_file> <book_file>                   convert the scores_file to proper binary format aka book_file for the solver to read.\n"
        <<
        "  warmup <hard_moves_file> <scores_file>              calculate the scores of moves resulting from each move in hard_moves.txt obtained from the training mode.\n\n"
        << "Some examples:\n"
        << "Generate opening book:\n"
        << "  generator explore 1 12 moves.txt\n"
        << "  generator calculate moves.txt scores.txt\n"
        << "  generator convert scores.txt opening_binary.book\n"
        << "Generate warmup book:\n"
        << "  generator warmup hard_moves.txt warmup_scores.txt\n"
        << "  generator convert warmup_scores.txt warmup.book\n";

    return 1;
  }

  if (std::strcmp(argv[1], "explore") == 0) {
    constexpr int NUM_REQUIRED_ARGS = 5;
    if (argc != NUM_REQUIRED_ARGS) {
      cout << "Invalid number of arguments.\n"
          << "Usage: generator explore <lower_depth> <higher_depth> <output_file>\n";
      return 1;
    }

    const int lower_depth = atoi(argv[2]);
    const int higher_depth = atoi(argv[3]);
    constexpr int MIN_DEPTH = 0;
    constexpr int MAX_DEPTH = 42;
    if (lower_depth < MIN_DEPTH || lower_depth > MAX_DEPTH) {
      cout <<
          "Invalid lower_depth, depth must be between 0 and 42.\n";
      return 1;
    }
    if (higher_depth < MIN_DEPTH || higher_depth > MAX_DEPTH) {
      cout <<
          "Invalid higher_depth, depth must be between 0 and 42.\n";
      return 1;
    }
    string output_file_path = argv[4];

    string pos_str;
    pos_str.reserve(higher_depth + 1);
    int number_of_explored_moves = 0;
    std::unordered_set<uint64_t> visited;
    ofstream moves_explored_stream(output_file_path);

    explore(Position(), pos_str, visited,
            number_of_explored_moves,
            lower_depth, higher_depth,
            moves_explored_stream);
    cout << "Number of moves: " << number_of_explored_moves << "\n";
  } else if (strcmp(argv[1], "calculate") == 0) {
    if (argc != 4) {
      cout << "Invalid number of arguments.\n"
          << "Usage: generator calculate <moves_file> <scores_file>\n";
      return 1;
    }
    calculateScore(argv[2], argv[3]);
  } else if (strcmp(argv[1], "convert") == 0) {
    if (argc != 4) {
      cout << "Invalid number of arguments.\n"
          << "Usage: generator convert <scores_file> <book_file>\n";
      return 1;
    }
    cout << "Conversion started...\n";
    int num_moves = convertScoreBookToBinary(argv[2], argv[3]);
    cout << "Complete converting " << num_moves <<
        " moves to binary.\nOpening book saved in: " << argv[3] <<
        "\n";
  } else if (strcmp(argv[1], "warmup") == 0) {
    if (argc != 4) {
      cout << "Invalid number of arguments.\n"
          << "Usage: generator warmup <hard_moves_file> <warmup_book_file>\n";
      return 1;
    }
    calculateHardMoves(argv[2], argv[3]);
  } else {
    cout <<
        "Invalid option. List of options are: explore, calculate, convert.\n";
    return 1;
  }

  return 0;
}

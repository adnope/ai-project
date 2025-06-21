#include <string>
#include <unordered_set>
#include <fstream>
#include <chrono>

#include "core/RequestHandler.hpp"
#include "core/Solver.hpp"
#include "core/Game.hpp"

#include "argparse.hpp"

using std::cout;
using std::cin;
using std::string;
using std::ofstream;
using std::ifstream;
using hr_clock = std::chrono::high_resolution_clock;
using std::chrono::duration;

void findMoveAndCalculateScore(const string &OPENING_BOOK_PATH,
                               const string &WARMUP_BOOK_PATH) {
  Solver solver;
  solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);

  string line;
  while (getline(cin, line)) {
    Position P;
    if (P.Play(line) != line.size()) {
      cout << "Invalid move: " << line << '\n';
    } else {
      auto start = hr_clock::now();
      const int score = solver.Solve(P);
      const int best_move = solver.FindBestMove(P);
      auto end = hr_clock::now();
      duration<double, std::milli> duration = end - start;

      cout << line
          << ": " << P.NumMoves() << " moves, "
          << "Score: " << score
          << ", Nodes: " << solver.GetNodeCount()
          << ", Time: " << duration.count() << " ms"
          << ", Best move: column " << best_move + 1 << "\n";
    }
  }
}

void startTraining(const string &OPENING_BOOK_PATH,
                                const string &WARMUP_BOOK_PATH) {
  ofstream hard_moves_stream("hard_moves.txt");
  std::unordered_set<string> seen_lines;

  Solver solver;
  solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);

  cout << "\nTRAINING SESSION STARTED!\n\n";

  string initial_sequence = "44444";
  string sequence = initial_sequence;
  Position P;
  P.Play(sequence);

  int move = 0;
  while (true) {
    constexpr int RESET_LIMIT = 20;
    if (P.NumMoves() == RESET_LIMIT) {
      sequence = initial_sequence;
      P = Position();
      P.Play(sequence);
    }

    // cout << "Position: " << sequence << ", " << sequence.size() << " moves\n";

    // bool is_red_turn = (sequence.size() + 1) % 2;
    // string player_name;
    // if (is_red_turn)
    // 	player_name = "Red";
    // else
    // 	player_name = "Yellow";
    // cout << player_name << " is thinking...\n";

    auto start = hr_clock::now();
    // Play randomly at the 7th move of the board
    // if (P.nbMoves() == 7)
    // 	move = solver.RandomMove();
    // else
    move = solver.FindBestMove(P);
    auto end = hr_clock::now();
    duration<double, std::milli> duration = end - start;

    constexpr int TIMEOUT = 2000;
    if (duration.count() >= TIMEOUT && seen_lines.find(sequence) == seen_lines.
        end()) {
      cout << "HARD MOVE FOUND: " << sequence << ", " << duration.count() <<
          " ms.\n";
      seen_lines.insert(sequence);
      hard_moves_stream << sequence << "\n";
      hard_moves_stream.flush();

      Position hard_pos;
      hard_pos.Play(sequence);
      for (int col = 0; col < Position::WIDTH; ++col) {
        Position next_pos(hard_pos);
        if (next_pos.CanPlay(col)) {
          next_pos.PlayCol(col);
          uint64_t key = next_pos.Key3();
          uint8_t score = solver.Solve(next_pos) - Position::MIN_SCORE + 1;
          solver.GetTranspositionTable().PutOpeningMove(key, score);
        }
      }
    }
    P.PlayCol(move);
    sequence += std::to_string(move + 1);
  }
}

int main(const int argc, char **argv) {
  argparse::ArgumentParser program("c4ai", "1.0",
                                   argparse::default_arguments::help);

  const std::vector<string> modes = {
    "find", "play", "botgame", "training", "server"
  };

  program.add_argument("mode")
      .help("Mode to run (find, play, botgame, training, server)")
      .required()
      .action([&modes](const string &value) {
        if (find(modes.begin(), modes.end(), value) == modes.end()) {
          throw std::runtime_error("Invalid mode '" + value + "'.");
        }
        return value;
      });

  program.add_argument("opening_book")
      .nargs(0, 1)
      .help("Optional opening book path")
      .default_value(string("data/opening.book"));
  // Default book contains all moves of depth 1 to depth 12

  program.add_argument("warmup_book")
      .nargs(0, 1)
      .help("Optional warmup book path")
      .default_value(string("data/warmup.book"));

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    cout << err.what() << "\n\n"
        << program << '\n';
    return 1;
  }

  const auto mode = program.get<string>("mode");
  auto opening_book = program.get<string>("opening_book");
  auto warmup_book = program.get<string>("warmup_book");

  if (mode == modes[0]) {
    findMoveAndCalculateScore(opening_book, warmup_book);
  } else if (mode == modes[1]) {
    Solver solver;
    solver.GetReady(opening_book, warmup_book);
    Game game(solver);
    game.StartPlayerVsBotGame();
  } else if (mode == modes[2]) {
    Solver solver;
    solver.GetReady(opening_book, warmup_book);
    Game game(solver);
    game.StartBotGame();
  } else if (mode == modes[3]) {
    startTraining(opening_book, warmup_book);
  } else if (mode == modes[4]) {
    Solver solver;
    solver.GetReady(opening_book, warmup_book);
    constexpr int port = 8112;
    RequestHandler requestHandler(solver, "0.0.0.0", port);
    requestHandler.Run();
  }

  return 0;
}

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

#include "core/solver.hpp"
#include "game/game.hpp"
// #include "server/request_handler.hpp"

#include <cxxopts.hpp>

using std::cin;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;
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

      cout << line << ": " << P.NumMoves() << " moves, "
           << "Score: " << score
           << ", Nodes: " << solver.GetNodeCount()
           << ", Time: " << duration.count() << " ms"
           << ", Best move: column " << best_move + 1 << '\n';
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

    // cout << "Position: " << sequence << ", " << sequence.size() << "
    // moves\n";

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
    if (duration.count() >= TIMEOUT &&
        seen_lines.find(sequence) == seen_lines.end()) {
      cout << "HARD MOVE FOUND: " << sequence << ", " << duration.count() << " ms.\n";
      seen_lines.insert(sequence);
      hard_moves_stream << sequence << '\n';
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

// int main(const int argc, char **argv) {
//   argparse::ArgumentParser program("c4ai", "1.0",
//                                    argparse::default_arguments::help);

//   const std::vector<string> modes = {"find", "play", "botgame", "training",
//                                      "server"};

//   program.add_argument("mode")
//       .help("Mode to run (find, play, botgame, training, server)")
//       .required()
//       .action([&modes](const string &value) {
//         if (find(modes.begin(), modes.end(), value) == modes.end()) {
//           throw std::runtime_error("Invalid mode '" + value + "'.");
//         }
//         return value;
//       });

//   program.add_argument("opening_book")
//       .nargs(0, 1)
//       .help("Optional opening book path")
//       .default_value(string("data/opening.book"));
//   // Default book contains all moves of depth 1 to depth 12

//   program.add_argument("warmup_book")
//       .nargs(0, 1)
//       .help("Optional warmup book path")
//       .default_value(string("data/warmup.book"));

//   try {
//     program.parse_args(argc, argv);
//   } catch (const std::runtime_error &err) {
//     cout << err.what() << "\n\n" << program << '\n';
//     return 1;
//   }

//   const auto mode = program.get<string>("mode");
//   auto opening_book = program.get<string>("opening_book");
//   auto warmup_book = program.get<string>("warmup_book");

//   if (mode == modes[0]) {
//     findMoveAndCalculateScore(opening_book, warmup_book);
//   }
//   if (mode == modes[1]) {
//     Solver solver;
//     solver.GetReady(opening_book, warmup_book);
//     Game game(solver);
//     game.StartPlayerVsBotGame();
//   }
//   if (mode == modes[2]) {
//     Solver solver;
//     solver.GetReady(opening_book, warmup_book);
//     Game game(solver);
//     game.StartBotGame();
//   }
//   if (mode == modes[3]) {
//     startTraining(opening_book, warmup_book);
//   }
//   if (mode == modes[4]) {
//     Solver solver;
//     solver.GetReady(opening_book, warmup_book);
//     constexpr int port = 8112;
//     RequestHandler requestHandler(solver, "0.0.0.0", port);
//     requestHandler.Run();
//   }

//   return 0;
// }

int main(const int argc, char **argv) {
  cxxopts::Options options("c4ai", "A connect four game solver");

  options.add_options("MODE")(
      "f,find", "Find the best move for a given sequence of game state.",
      cxxopts::value<bool>()->default_value("false")->implicit_value("true"))(
      "p,play", "Play a game versus bot.",
      cxxopts::value<bool>()->default_value("false")->implicit_value("true"))(
      "b,botgame", "See a game between 2 bots.",
      cxxopts::value<bool>()->default_value("false")->implicit_value("true"))(
      "h,help", "Print this menu",
      cxxopts::value<bool>()->default_value("false")->implicit_value("true"))(
      "t,training", "Start a training session.",
      cxxopts::value<bool>()->default_value("false")->implicit_value("true"));

  options.add_options("BOOK")(
      "opening-book", "Specify an opening book.",
      cxxopts::value<string>()->default_value("data/opening.book"))(
      "warmup-book", "Specify a warmup book.",
      cxxopts::value<string>()->default_value("data/warmup.book"));

  options.parse_positional({"opening-book", "warmup-book"});

  constexpr int OPTION_LENGTH = 100;
  options.show_positional_help()
      .custom_help("MODE")
      .positional_help("[BOOK...]")
      .set_width(OPTION_LENGTH);

  if (argc == 1) {
    cout << options.help();
    return 1;
  }

  cxxopts::ParseResult result;
  try {
    result = options.parse(argc, argv);
  } catch (cxxopts::exceptions::parsing &e) {
    cout << e.what() << '\n' << options.help();
    return 1;
  }

  if (result.contains("help")) {
    cout << options.help();
    return 0;
  }

  const int find_option_count = static_cast<int>(result.count("find"));
  const int play_option_count = static_cast<int>(result.count("play"));
  const int botgame_option_count = static_cast<int>(result.count("botgame"));
  const int training_option_count = static_cast<int>(result.count("training"));

  if (find_option_count + play_option_count + botgame_option_count + training_option_count > 1) {
    std::cerr << "Specify 1 option only.\n";
    cout << options.help();
    return 1;
  }

  const bool find = result["find"].as<bool>();
  const bool play = result["play"].as<bool>();
  const bool botgame = result["botgame"].as<bool>();
  const bool training = result["training"].as<bool>();
  const string opening_book = result["opening-book"].as<string>();
  const string warmup_book = result["warmup-book"].as<string>();

  if (find) {
    findMoveAndCalculateScore(opening_book, warmup_book);
  } else if (play || botgame) {
    Solver solver;
    solver.GetReady(opening_book, warmup_book);
    Game game(solver);
    if (play) {
      game.StartPlayerVsBotGame();
    } else {
      game.StartBotGame();
    }
  } else if (training) {
    startTraining(opening_book, warmup_book);
  }

  return 0;
}
#pragma once

#include "httplib.h"
#include "json.hpp"
#include "Solver.hpp"
#include "Position.hpp"

#include <future>
#include <utility>

using json = nlohmann::json;

class RequestHandler {
private:
  httplib::Server svr;
  Solver solver;
  std::string ip;
  uint16_t port;

  static void Log(const std::vector<std::vector<int> > &board,
                  const int &current_player,
                  const std::vector<int> &valid_moves,
                  const bool &is_new_game) {
    std::cout << "Board: \n";
    for (const std::vector<int> &v: board) {
      for (const int &i: v) {
        std::cout << i << " ";
      }
      std::cout << "\n";
    }
    std::cout << "Current player: " << current_player << "\n";
    std::cout << "Valid moves: ";
    for (const int i: valid_moves) {
      std::cout << i << " ";
    }
    std::cout << "\nIs new game: " << std::boolalpha << is_new_game << "\n";
  }

  int GetMoveFromSolver(const std::vector<std::vector<int> > &board,
                        const bool &is_new_game, const int &current_player,
                        const std::vector<int> &valid_moves) {
    if (is_new_game && current_player == 1) {
      return Solver::DEFAULT_FIRST_MOVE;
    }

    Position P(board);

    std::packaged_task<std::vector<std::vector<int> >(
      Solver &, const Position &)> task(&Solver::Analyze);

    std::future<std::vector<std::vector<int> > > analyze_future = task.
        get_future();
    std::thread analyze_thread(move(task), std::ref(solver), P);

    constexpr int timeout = 7;
    const std::chrono::seconds timeout_duration(timeout);

    const std::future_status status = analyze_future.wait_for(timeout_duration);
    if (status == std::future_status::timeout) {
      if (analyze_thread.joinable()) {
        analyze_thread.detach();
      }
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> distrib(0, Position::WIDTH - 1);
      const int random_move = distrib(gen);

      std::cout << "[Solver] Computation timed out after " << timeout <<
          " seconds. Using random move: " << random_move + 1 << "\n";
      return random_move;
    }
    if (status == std::future_status::ready) {
      const std::vector<std::vector<int> > ranked_moves = analyze_future.get();
      if (analyze_thread.joinable()) {
        analyze_thread.join();
      }
      std::vector<int> move_list = {};
      for (const auto &cols: ranked_moves) {
        for (const int col: cols) {
          move_list.push_back(col);
        }
      }

      std::cout << "[Solver] Moves to make (from best to worst): ";
      for (const int i: move_list) {
        std::cout << i << " ";
      }
      std::cout << "\n";

      int move = -1;
      for (const int col: move_list) {
        if (find(valid_moves.begin(), valid_moves.end(), col) != valid_moves.
            end()) {
          move = col;
          break;
        }
        std::cout <<
            "[Solver] Best move is invalid, changing to next best move...\n";
      }

      if (move == -1) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(
          0, static_cast<int>(valid_moves.size()) - 1);
        move = valid_moves[dist(gen)];

        std::cout << "[Solver] No valid moves found. Using random move: " <<
            move + 1 << "\n";
        return move;
      }

      std::cout << "[Solver] Number of moves: " << P.NumMoves() <<
          ", Best move: " << move + 1 << "\n";
      return move;
    }
    return -1;
  }

public:
  RequestHandler(Solver solver, std::string ip,
                 const uint16_t port) : solver(std::move(solver)),
                                        ip(std::move(ip)),
                                        port(port) {
  }

  void Run() {
    auto handle{
      [&](const httplib::Request &req, httplib::Response &res) {
        try {
          json req_data = json::parse(req.body);
          std::cout << "\nNew request: " << req_data.dump() << "\n\n";

          const std::vector<std::vector<int> > board = req_data["board"].get<
            std::vector<std::vector<int> > >();
          const std::vector<int> valid_moves = req_data["valid_moves"].get<
            std::vector<int> >();
          const int current_player = req_data["current_player"];
          const bool is_new_game = req_data["is_new_game"];

          Log(board, current_player, valid_moves, is_new_game);

          const auto start = std::chrono::high_resolution_clock::now();

          int move = GetMoveFromSolver(board, is_new_game, current_player,
                                       valid_moves);

          const json json_response = {
            {"move", move}
          };

          res.set_content(json_response.dump(), "application/json");

          const auto end = std::chrono::high_resolution_clock::now();
          const std::chrono::duration<double, std::milli> duration =
              end - start;
          std::cout << "[Solver] Total time: " << duration.count() << " ms.\n";
          std::cout.flush();
        } catch (const std::exception &e) {
          constexpr int ERROR_CODE = 400;
          res.status = ERROR_CODE;
          res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
      }
    };

    svr.Post("/api/connect4-move", handle);

    std::cout << "The server is running at http://" << ip << ":" << port <<
        "\n";
    std::cout.flush();

    svr.listen(ip, port);
  }
};

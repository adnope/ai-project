#pragma once

#include "lib/httplib.h"
#include "lib/json.hpp"
#include "Solver.hpp"
#include "Position.hpp"

#include <future>
#include <utility>

using json = nlohmann::json;
using namespace std;

class RequestHandler
{
private:
    httplib::Server svr;
    string ip;
    uint16_t port;
    Solver solver;

    static void Log(const vector<vector<int>> &board, const int &current_player, const vector<int> &valid_moves, const bool &is_new_game)
    {
        cout << "Board: \n";
        for (const vector<int> &v : board)
        {
            for (const int &i : v)
            {
                cout << i << " ";
            }
            cout << "\n";
        }
        cout << "Current player: " << current_player << "\n";
        cout << "Valid moves: ";
        for (const int i : valid_moves)
        {
            cout << i << " ";
        }
        cout << "\nIs new game: " << boolalpha << is_new_game << "\n";
    }

    int GetMoveFromSolver(const vector<vector<int>> &board, const bool &is_new_game, const int &current_player, const vector<int> &valid_moves)
    {
        if (is_new_game && current_player == 1)
        {
            return solver.GetDefaultFirstMove();
        }

        Position P(board);

        packaged_task<vector<vector<int>>(Solver&, const Position&)> task(&Solver::Analyze);

        future<vector<vector<int>>> analyze_future = task.get_future();
        thread analyze_thread(move(task), ref(solver), P);

        constexpr int timeout = 7;
        const chrono::seconds timeout_duration(timeout);

        if (const future_status status = analyze_future.wait_for(timeout_duration); status == future_status::timeout)
        {
            if (analyze_thread.joinable()) {
                analyze_thread.detach();
            }
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distrib(0, 6);
            const int random_move = distrib(gen);

            cout << "[Solver] Computation timed out after " << timeout << " seconds. Using random move: " << random_move + 1 << "\n";
            return random_move;
        }
        else if (status == future_status::ready)
        {
            const vector<vector<int>> ranked_moves = analyze_future.get();
            if (analyze_thread.joinable()) {
                analyze_thread.join();
            }
            vector<int> move_list = {};
            for (const auto &cols : ranked_moves)
            {
                for (const int col : cols)
                {
                    move_list.push_back(col);
                }
            }

            cout << "[Solver] Moves to make (from best to worst): ";
            for (const int i : move_list) cout << i << " ";
            cout << "\n";

            int move = -1;
            for (const int col : move_list)
            {
                if (find(valid_moves.begin(), valid_moves.end(), col) != valid_moves.end() &&
                    !P.OverlapWithHiddenPos(col))
                {
                    move = col;
                    break;
                }
                else {
                    cout << "[Solver] Best move is invalid, changing to next best move...\n";
                }
            }

            if (move == -1)
            {
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> dist(0, static_cast<int>(valid_moves.size()) - 1);
                move = valid_moves[dist(gen)];

                cout << "[Solver] No valid moves found. Using random move: " << move + 1 << "\n";
                return move;
            }

            cout << "[Solver] Number of moves: " << P.nbMoves() << ", Best move: " << move + 1 << "\n";
            return move;
        }
        else return -1;
    }

public:
    RequestHandler(string ip, const uint16_t port) : ip(std::move(ip)), port(port)
    {
        solver.GetReady();
    }

    void Run()
    {
        auto handle{
            [&](const httplib::Request &req, httplib::Response &res)
            {
                try
                {
                    json req_data = json::parse(req.body);
                    cout << "\nNew request: " << req_data.dump() << "\n\n";

                    const vector<vector<int>> board = req_data["board"].get<vector<vector<int>>>();
                    const vector<int> valid_moves = req_data["valid_moves"].get<vector<int>>();
                    const int current_player = req_data["current_player"];
                    const bool is_new_game = req_data["is_new_game"];

                    Log(board, current_player, valid_moves, is_new_game);

                    const auto start = chrono::high_resolution_clock::now();

                    int move = GetMoveFromSolver(board, is_new_game, current_player, valid_moves);

                    const json json_response = {
                        {"move", move}};

                    res.set_content(json_response.dump(), "application/json");

                    const auto end = chrono::high_resolution_clock::now();
                    const chrono::duration<double, milli> duration = end - start;
                    cout << "[Solver] Total time: " << duration.count() << " ms.\n";
                    cout.flush();
                }
                catch (const exception &e)
                {
                    res.status = 400;
                    res.set_content(json{{"error", e.what()}}.dump(), "application/json");
                }
            }};

        svr.Post("/api/connect4-move", handle);

        cout << "The server is running at http://" << ip << ":" << port << "\n";
        cout.flush();

        svr.listen(ip, port);
    }
};
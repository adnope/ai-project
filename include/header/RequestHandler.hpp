#pragma once

#include "lib/httplib.h"
#include "lib/json.hpp"
#include "Solver.hpp"
#include "Position.hpp"

#include <chrono>
#include <future>

using json = nlohmann::json;
using namespace std;

class RequestHandler
{
private:
    httplib::Server svr;
    string ip;
    uint16_t port;
    Solver solver;

    void Log(const vector<vector<int>> &board, const int &current_player, const vector<int> &valid_moves, const bool &is_new_game)
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
        for (int i : valid_moves)
        {
            cout << i << " ";
        }
        cout << "\nIs new game: " << boolalpha << is_new_game << "\n";
    }

    // int GetMoveFromSolver(const vector<vector<int>> &board, const bool &is_new_game, const int &current_player)
    // {
    //     if (is_new_game && current_player == 1)
    //     {
    //         return solver.GetDefaultFirstMove();
    //     }

    //     Position P(board);
    //     int move = solver.FindBestMove(P);

    //     cout << "[Solver] Number of moves: " << P.nbMoves() << ", Best move: " << move + 1 << "\n";

    //     return move;
    // }

    int GetMoveFromSolver(const vector<vector<int>> &board, const bool &is_new_game, const int &current_player, const vector<int> &valid_moves)
    {
        if (is_new_game && current_player == 1)
        {
            return solver.GetDefaultFirstMove();
        }

        Position P(board);

        // Set up a future to run FindBestMove asynchronously
        auto future_ranked_moves = std::async(std::launch::async, [&]()
                                              { return solver.Analyze(P); });

        // Wait for at most 8 seconds
        if (future_ranked_moves.wait_for(std::chrono::seconds(8)) == std::future_status::ready)
        {
            // We got the result within the time limit
            std::vector<std::vector<int>> ranked_moves = future_ranked_moves.get();

            vector<int> move_list = {};
            for (const auto &cols : ranked_moves)
            {
                for (const int col : cols)
                {
                    move_list.push_back(col);
                }
            }

            cout << "Moves to make (from best to worst): ";
            for (int i : move_list) cout << i << " ";
            cout << "\n";

            int move = -1;
            for (const int col : move_list)
            {
                if (find(valid_moves.begin(), valid_moves.end(), col) != valid_moves.end())
                {
                    move = col;
                    break;
                }
                else {
                    cout << "Best move is invalid, changing to next best move...\n";
                }
            }

            if (move == -1)
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, valid_moves.size() - 1);
                move = valid_moves[dist(gen)];

                cout << "[Solver] No valid moves found. Using random move: " << move + 1 << "\n";
                return move;
            }

            cout << "[Solver] Number of moves: " << P.nbMoves() << ", Best move: " << move + 1 << "\n";
            return move;
        }
        else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, 6);
            int random_move = distrib(gen);

            cout << "[Solver] Computation timed out after 8 seconds. Using random move: " << random_move + 1 << "\n";
            return random_move;
        }
    }

public:
    RequestHandler(string ip, uint16_t port) : ip(ip), port(port)
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

                    vector<vector<int>> board = req_data["board"].get<vector<vector<int>>>();
                    vector<int> valid_moves = req_data["valid_moves"].get<vector<int>>();
                    int current_player = req_data["current_player"];
                    bool is_new_game = req_data["is_new_game"];

                    Log(board, current_player, valid_moves, is_new_game);

                    auto start = chrono::high_resolution_clock::now();

                    int move = GetMoveFromSolver(board, is_new_game, current_player, valid_moves);

                    json json_response = {
                        {"move", move}};

                    res.set_content(json_response.dump(), "application/json");

                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double, milli> duration = end - start;
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
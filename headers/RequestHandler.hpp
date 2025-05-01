#pragma once

#include "httplib.h"
#include "json.hpp"
#include "Solver.hpp"
#include "Position.hpp"
#include <chrono>

using namespace std;
using json = nlohmann::json;

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
        for (auto v : board)
        {
            for (int i : v)
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

    int FindMove(const vector<vector<int>> &board, const bool &is_new_game, const int &current_player)
    {
        if (is_new_game && current_player == 1)
        {
            return 4;
        }

        Position P(board);
        int move = solver.FindBestMove(P);

        return move + 1;
    }

public:
    RequestHandler(string ip, uint16_t port) : ip(ip), port(port)
    {
        solver.LoadBookAndWarmup("depth_12_scores_7x6.book", "warmup.book");
    }

    void run()
    {
        auto handle
        {
            [&](const httplib::Request &req, httplib::Response &res) {
                try {
                    json req_data = json::parse(req.body);
                    cout << "New request: " << req_data.dump() << "\n";
        
                    vector<vector<int>> board = req_data["board"].get<vector<vector<int>>>();
                    vector<int> valid_moves = req_data["valid_moves"].get<vector<int>>();
                    int current_player = req_data["current_player"];
                    bool is_new_game = req_data["is_new_game"];
                
                    Log(board, current_player, valid_moves, is_new_game);
    
                    auto start = chrono::high_resolution_clock::now();

                    int move = FindMove(board, is_new_game, current_player);
                    cout << "Found best move: " << move << "\n";

                    json json_response = {
                        {"move", move}
                    };

                    res.set_content(json_response.dump(), "application/json");

                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double, milli> duration = end - start;
                    cout << "API & Move time: " << duration.count() << " ms.\n";
                } catch (const exception &e) {
                    res.status = 400;
                    res.set_content(json{{"error", e.what()}}.dump(), "application/json");
                }
            }
        };

        svr.Post("/api/connect4-move", handle);

        cout << "The server is running at http://" << ip << ":" << port << "\n";
        svr.listen(ip, port);
    }
};
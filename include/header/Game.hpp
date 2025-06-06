#pragma once

#include "Solver.hpp"
#include "Position.hpp"

class Game
{
private:
    Solver solver;

    static void printConnectFourBoard(const std::string &sequence)
    {
        constexpr int ROWS = Position::HEIGHT;
        constexpr int COLS = Position::WIDTH;
        char board[ROWS][COLS] = {0};

        for (auto & i : board)
        {
            for (char & j : i)
            {
                j = '.';
            }
        }

        for (size_t i = 0; i < sequence.size(); i++)
        {
            const int col = sequence[i] - '1';
            if (col < 0 || col >= COLS)
            {
                std::cerr << "Invalid column: " << sequence[i] << "\n";
                return;
            }

            int row = ROWS - 1;
            while (row >= 0 && board[row][col] != '.')
            {
                row--;
            }

            if (row < 0)
            {
                std::cerr << "Column " << col + 1 << " is already full!" << "\n";
                return;
            }

            board[row][col] = (i % 2 == 0) ? 'x' : 'o';
        }

        for (const auto & i : board)
        {
            std::cout << "|";
            for (const char j : i)
            {
                std::cout << j << "|";
            }
            std::cout << std::endl;
        }

        std::cout << " ";
        for (int j = 1; j <= COLS; j++)
        {
            std::cout << j << " ";
        }
        std::cout << std::endl;
    }

public:
    Game()
    {
        solver.GetReady();
    }

    void StartPlayerVsBotGame()
    {
        std::string sequence;
        Position P;
        P.Play(sequence);

        std::cout << "Choose your side:\n"
                  << "[1]: Red\n"
                  << "[2]: Yellow\n"
                  << "Enter your choice: ";

        int choice;
        while (std::cin >> choice)
        {
            if (choice == 1)
            {
                break;
            }
            else if (choice == 2)
            {
                int best_move = solver.FindBestMove(P);
                P.PlayCol(best_move);
                sequence += std::to_string(best_move + 1);
                break;
            }
            else
            {
                std::cout << "Invalid choice\n"
                          << "Enter your choice: ";
            }
        }

        std::cout << "The game has started!\n";

        int player_move;
        while (true)
        {
            printConnectFourBoard(sequence);
            std::cout << "Enter your move: column: ";
            std::cin >> player_move;

            while (player_move < 1 || player_move > Position::WIDTH)
            {
                std::cout << "Invalid move\nEnter your move: ";
                std::cin >> player_move;
            }

            if (P.IsWinningMove(player_move - 1))
            {
                sequence += std::to_string(player_move);
                printConnectFourBoard(sequence);
                std::cout << "You win!\n";
                break;
            }

            sequence += std::to_string(player_move);
            P.PlayCol(player_move - 1);

            const int ai_move = solver.FindBestMove(P);
            if (P.IsWinningMove(ai_move))
            {
                std::cout << "Bot has played: column " << ai_move + 1 << std::endl;
                sequence += std::to_string(ai_move + 1);
                printConnectFourBoard(sequence);
                std::cout << "You lose!\n";
                break;
            }
            P.PlayCol(ai_move);
            sequence += std::to_string(ai_move + 1);
            std::cout << "Bot has played: column " << ai_move + 1 << std::endl;
        }
    }

    void StartBotGame()
    {
        std::cout << "\n<------------------>\n"
                  << "THE GAME HAS STARTED\n"
                  << "<------------------>\n\n";

        std::string sequence;
        Position P;
        P.Play(sequence);

        std::string player_name;
        bool is_red_turn = false;
        while (true)
        {
            is_red_turn = (sequence.size() + 1) % 2;
            std::cout << "Moves: " << sequence.size() << "\n";
            printConnectFourBoard(sequence);

            if (is_red_turn)
                player_name = "Red";
            else
                player_name = "Yellow";

            std::cout << player_name << " is thinking...\n";

            auto start = std::chrono::high_resolution_clock::now();
            const int move = solver.FindBestMove(P);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            std::cout << player_name << " has played: column " << move + 1 << ", " << duration.count() << " ms.\n";

            if (P.IsWinningMove(move))
            {
                sequence += std::to_string(move + 1);
                printConnectFourBoard(sequence);
                std::cout << player_name << " won!\n";
                return;
            }

            P.PlayCol(move);
            sequence += std::to_string(move + 1);
        }
    }
};
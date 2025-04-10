#include <iostream>
#include <chrono>
#include "headers/Solver.hpp"
#include "headers/OpeningBook.hpp"

int runTest()
{
	Solver solver;
	std::ifstream testStream("tests/10_moves_test.txt");

	if (!testStream)
	{
		std::cerr << "Cannot open test file.";
		return 1;
	}

	std::string line;
	int correct_score;

	int l = 0;
	while (testStream >> line && testStream >> correct_score)
	{
		Position P;
		if (P.play(line) != line.size())
		{
			std::cout << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
		}
		else
		{
			solver.reset();
			auto start = std::chrono::high_resolution_clock::now();
			unsigned int best_move = solver.findBestMove(P);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end - start;

			int score = solver.solve(P);

			std::cout << line
					  << ": " << P.nbMoves() << " moves, "
					  << "Score: " << score
					  << ", Nodes: " << solver.getNodeCount()
					  << ", Time: " << duration.count()
					  << " ms, Best move: column " << best_move + 1 << " - ";

			if (score == correct_score)
				std::cout << "[Correct!]" << std::endl;
			else
				std::cout << "[INCORRECT]";
		}
		l++;
	}

	return 0;
}

void findMoveAndCalculateScore()
{
	Solver solver;
	OpeningBook openingBook(7, 6, 14, &solver.transTable);
	openingBook.load("7x6.book");

	std::string line;

	for (int l = 1; std::getline(std::cin, line); l++)
	{
		Position P;

		if (P.play(line) != line.size())
		{
			std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
		}
		else
		{
			if (line.length() < 10)
			{
				uint8_t bestMove = (openingBook.getBestMove(solver.transTable.encodeMoves(line)));
				if (bestMove != OpeningBook::NO_MOVE)
				{
					std::cout << "Key " << line << " best move " << (int)(bestMove) << std::endl;
				}
				else
				{
					std::cout << "not find " << line << std::endl;
				}
			}
			else
			{
				solver.reset();
				auto start = std::chrono::high_resolution_clock::now();
				unsigned int best_move = solver.findBestMove(P);
				int score = solver.solve(P);
				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> duration = end - start;

				std::cout << line
						  << ": " << P.nbMoves() << " moves, "
						  << "Score: " << score
						  << ", Nodes: " << solver.getNodeCount()
						  << ", Time: " << duration.count()
						  << " ms, Best move: column " << best_move + 1 << "\n";
			}
		}
	}
}

void printConnectFourBoard(const std::string &moves)
{
	const int ROWS = Position::HEIGHT;
	const int COLS = Position::WIDTH;
	char board[ROWS][COLS] = {0};

	// Initialize empty board
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			board[i][j] = '.';
		}
	}

	// Process each move
	for (size_t i = 0; i < moves.size(); i++)
	{
		int col = moves[i] - '1'; // Convert char to 0-based column index
		if (col < 0 || col >= COLS)
		{
			std::cerr << "Invalid column: " << moves[i] << std::endl;
			return;
		}

		// Find the first empty row in the column
		int row = ROWS - 1;
		while (row >= 0 && board[row][col] != '.')
		{
			row--;
		}

		if (row < 0)
		{
			std::cerr << "Column " << col + 1 << " is already full!" << std::endl;
			return;
		}

		// Place the piece (alternate between 'R' and 'Y')
		board[row][col] = (i % 2 == 0) ? 'x' : 'o';
	}

	// Print the board
	for (int i = 0; i < ROWS; i++)
	{
		std::cout << "|";
		for (int j = 0; j < COLS; j++)
		{
			std::cout << board[i][j] << "|";
		}
		std::cout << std::endl;
	}
	
	// Print column numbers
	std::cout << " ";
	for (int j = 1; j <= COLS; j++)
	{
		std::cout << j << " ";
	}
	std::cout << std::endl;
}

int startGame()
{
	Solver solver;

	std::string sequence = "444344535356";
	Position P;
	P.play(sequence);

	std::cout << "Choose your side:\n"
			  << "[1]: Red\n"
			  << "[2]: Yellow\n"
			  << "Enter your choice: ";

	int choice;
	while (std::cin >> choice) {
		if (choice == 1)
		{
			break;
		}
		else if (choice == 2)
		{
			P.playCol(solver.findBestMove(P));
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
	while (1)
	{
		printConnectFourBoard(sequence);
		std::cout << "Enter your move: column: ";
		std::cin >> player_move;

		while (player_move < 1 || player_move > Position::WIDTH) {
			std::cout << "Invalid move\nEnter your move: ";
			std::cin >> player_move;
		}

		if (P.isWinningMove(player_move - 1))
		{
			sequence += std::to_string(player_move);
			printConnectFourBoard(sequence);
			std::cout << "You win!\n";
			break;
		}

		sequence += std::to_string(player_move);
		P.playCol(player_move - 1);

		int ai_move = solver.findBestMove(P);
		if (P.isWinningMove(ai_move))
		{
			std::cout << "Bot has played: column " << ai_move + 1 << std::endl;
			sequence += std::to_string(ai_move + 1);
			printConnectFourBoard(sequence);
			std::cout << "You lose!\n";
			break;
		}
		P.playCol(ai_move);
		sequence += std::to_string(ai_move + 1);
		std::cout << "Bot has played: column " << ai_move + 1<< std::endl;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--test") == 0)
		{
			runTest();
		}
		else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--find_move") == 0)
		{
			findMoveAndCalculateScore();
		}
		else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--play") == 0)
		{
			startGame();
		}
		else
		{
			std::cerr << "Argument not found.\n";
			return 1;
		}
	}
	else
	{
		findMoveAndCalculateScore();
	}

	return 0;
}
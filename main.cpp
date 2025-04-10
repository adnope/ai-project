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
	OpeningBook openingBook(7, 6, &solver.transTable);
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
	while (std::cin >> choice)
	{
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

		while (player_move < 1 || player_move > Position::WIDTH)
		{
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
		std::cout << "Bot has played: column " << ai_move + 1 << std::endl;
	}

	return 0;
}

std::vector<std::vector<int>> move_sequence_to_board(const std::string &sequence)
{
	std::vector<std::vector<int>> board(6, std::vector<int>(7, 0));
	int current_player = 1;

	for (char move_char : sequence)
	{
		int col = move_char - '1'; // Convert char to column index (0-6)

		if (col < 0 || col > 6)
		{
			std::cerr << "Invalid move in sequence: " << move_char << std::endl;
			return std::vector<std::vector<int>>(); // Return an empty board for invalid sequence
		}

		// Find the lowest empty row in the column
		int row = 5;
		while (row >= 0 && board[row][col] != 0)
		{
			--row;
		}

		// If the column is not full, place the piece
		if (row >= 0)
		{
			board[row][col] = current_player;
			current_player = (current_player == 1) ? 2 : 1;
		}
		else
		{
			std::cerr << "Column full in sequence: " << move_char << std::endl;
			return std::vector<std::vector<int>>(); // Return an empty board for invalid sequence
		}
	}

	return board;
}

bool is_valid_board(const std::vector<std::vector<int>> &curr_board,
					const std::vector<std::vector<int>> &target_board)
{
	// Check if current board could lead to target board
	for (int r = 0; r < 6; r++)
	{
		for (int c = 0; c < 7; c++)
		{
			if (curr_board[r][c] != 0 && curr_board[r][c] != target_board[r][c])
			{
				return false;
			}
		}
	}
	return true;
}

bool is_valid_connect4_board(const std::vector<std::vector<int>> &board)
{
	// Check if pieces are properly stacked due to gravity
	for (int col = 0; col < 7; col++)
	{
		for (int row = 4; row >= 0; row--)
		{
			if (board[row][col] != 0 && board[row + 1][col] == 0)
			{
				return false; // Floating piece found
			}
		}
	}
	return true;
}

bool dfs_find_sequence(const std::vector<std::vector<int>> &target_board,
					   std::vector<std::vector<int>> &curr_board,
					   std::vector<int> &move_seq,
					   int curr_player,
					   int &piece_count)
{

	// Base case: if we've placed all the pieces in the target board
	if (piece_count == 0)
	{
		return curr_board == target_board;
	}

	// Try each column
	for (int col = 0; col < 7; ++col)
	{
		// Find lowest empty row in this column
		int row = 5;
		while (row >= 0 && curr_board[row][col] != 0)
		{
			--row;
		}

		// If column is not full
		if (row >= 0)
		{
			// Make the move only if it matches target board
			if (target_board[row][col] == curr_player)
			{
				curr_board[row][col] = curr_player;
				move_seq.push_back(col + 1);
				piece_count--;

				// Recursively try next move with other player
				if (dfs_find_sequence(target_board, curr_board, move_seq,
									  curr_player == 1 ? 2 : 1, piece_count))
				{
					return true;
				}

				// If this path didn't work, undo the move
				curr_board[row][col] = 0;
				move_seq.pop_back();
				piece_count++;
			}
		}
	}

	return false;
}

std::string board_to_move_sequence(const std::vector<std::vector<int>> &board)
{
	// First, verify the board is valid Connect 4 board
	if (!is_valid_connect4_board(board))
	{
		return ""; // Invalid board configuration
	}

	std::vector<int> move_seq;
	std::vector<std::vector<int>> curr_board(6, std::vector<int>(7, 0));

	// Count total pieces
	int piece_count = 0;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (board[i][j] != 0)
				piece_count++;
		}
	}

	// Try starting with player 1
	int remaining = piece_count;
	if (dfs_find_sequence(board, curr_board, move_seq, 1, remaining))
	{
		std::string result;
		for (int move : move_seq)
		{
			result += std::to_string(move);
		}
		return result;
	}

	// If player 1 starting didn't work, try player 2
	move_seq.clear();
	curr_board = std::vector<std::vector<int>>(6, std::vector<int>(7, 0));
	remaining = piece_count;
	if (dfs_find_sequence(board, curr_board, move_seq, 2, remaining))
	{
		std::string result;
		for (int move : move_seq)
		{
			result += std::to_string(move);
		}
		return result;
	}

	return ""; // No valid sequence found
}

int testSequenceToBoard()
{
	std::string test_sequence;

	std::cout << "Enter a move sequence (e.g., 1234567): ";
	std::cin >> test_sequence;

	std::cout << "Testing sequence: " << test_sequence << "\n";
	std::vector<std::vector<int>> board = move_sequence_to_board(test_sequence);

	if (board.empty())
	{
		std::cout << "Invalid sequence!\n";
		return 1;
	}

	std::cout << "\nFinal board state:\n";
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			std::cout << board[i][j] << " ";
		}
		std::cout << "\n";
	}

	return 0;
}

void testBoardInput()
{
	std::vector<std::vector<int>> board(6, std::vector<int>(7, 0));

	std::cout << "Enter the Connect 4 board state (6 rows, 7 columns)\n"
			  << "Use 0 for empty, 1 for player 1, 2 for player 2\n"
			  << "Enter each row from top to bottom:\n";

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			std::cin >> board[i][j];
			if (board[i][j] < 0 || board[i][j] > 2)
			{
				std::cout << "Invalid input\n";
				return;
			}
		}
	}

	std::string sequence = board_to_move_sequence(board);

	std::cout << "Move sequence: " << sequence << "\n";

	Position P;
	if (P.play(sequence) != sequence.size())
	{
		std::cout << "Invalid sequence generated!\n";
	}
	else
	{
		Solver solver;
		solver.reset();
		unsigned int best_move = solver.findBestMove(P);
		int score = solver.solve(P);

		std::cout << "Current position analysis:\n";
		std::cout << "Best move: column " << best_move + 1 << "\n";
		std::cout << "Position score: " << score << "\n";
	}
}

int runTestForBoardToSequence()
{
	std::ifstream testStream("tests/begin_hard.txt");
	std::ofstream resultStream("results_1.txt");

	if (!testStream || !resultStream)
	{
		std::cerr << "Cannot open test file.";
		return 1;
	}

	std::string line;
	int correct_score;
	int total_tests = 0;
	int successful_matches = 0;

	resultStream << "Connect 4 Board-to-Sequence Test Results\n";
	resultStream << "========================================\n\n";

	while (testStream >> line >> correct_score)
	{
		total_tests++;
		resultStream << "Test case #" << total_tests + 1 << "\n";
		resultStream << "-------------\n";
		resultStream << "Original sequence: " << line << "\n";

		std::vector<std::vector<int>> board = move_sequence_to_board(line);

		if (board.empty())
		{
			resultStream << "Invalid move sequence!\n\n";
			continue;
		}

		// Convert board to move sequence
		auto start_seq = std::chrono::high_resolution_clock::now();
		std::string generated_sequence = board_to_move_sequence(board);
		auto end_seq = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration_seq = end_seq - start_seq;

		if (generated_sequence.empty())
		{
			resultStream << "ERROR: Could not generate sequence from board\n\n";
			continue;
		}

		std::vector<std::vector<int>> final_board = move_sequence_to_board(generated_sequence);

		bool boards_match = (board == final_board);
		if (boards_match)
			successful_matches++;

		resultStream << "Board state:\n";
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 7; j++)
			{
				resultStream << board[i][j] << " ";
			}
			resultStream << "\n";
		}

		resultStream << "Boards match: " << (boards_match ? "Yes" : "No") << "\n";
		resultStream << "Conversion time: " << duration_seq.count() << " ms\n";
		resultStream << "----------------------------------------\n\n";
	}

	// Write summary
	resultStream << "\nTest Summary\n";
	resultStream << "============\n";
	resultStream << "Total tests run: " << total_tests << "\n";
	resultStream << "Successful matches: " << successful_matches << "\n";
	resultStream << "Failed matches: " << (total_tests - successful_matches) << "\n";
	resultStream << "Success rate: " << (successful_matches * 100.0 / total_tests) << "%\n";

	testStream.close();
	resultStream.close();

	std::cout << "Results written to results.txt\n";
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
		else if (strcmp(argv[1], "-s") == 0)
		{
			runTestForBoardToSequence();
		}
		else if (strcmp(argv[1], "-q") == 0)
		{
			testSequenceToBoard();
		}
		else if (strcmp(argv[1], "-b") == 0)
		{
			testBoardInput();
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
#include <iostream>
#include <chrono>
#include "headers/Solver.hpp"
#include "headers/OpeningBook.hpp"

using namespace std;

int runTest()
{
	Solver solver;
	ifstream testStream("tests/10_moves_test.txt");

	if (!testStream)
	{
		cerr << "Cannot open test file.";
		return 1;
	}

	string line;
	int correct_score;

	int l = 0;
	while (testStream >> line && testStream >> correct_score)
	{
		Position P;
		if (P.Play(line) != line.size())
		{
			cout << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << endl;
		}
		else
		{
			solver.Reset();
			auto start = chrono::high_resolution_clock::now();
			unsigned int best_move = solver.FindBestMove(P);
			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double, milli> duration = end - start;

			int score = solver.Solve(P);

			cout << line
					  << ": " << P.nbMoves() << " moves, "
					  << "Score: " << score
					  << ", Nodes: " << solver.GetNodeCount()
					  << ", Time: " << duration.count()
					  << " ms, Best move: column " << best_move + 1 << " - ";

			if (score == correct_score)
				cout << "[Correct!]" << endl;
			else
				cout << "[INCORRECT]";
		}
		l++;
	}

	return 0;
}

void findMoveAndCalculateScore()
{
	Solver solver;
	// OpeningBook openingBook(7, 6, &solver.transTable);
	// openingBook.load("7x6.book");

	string line;

	for (int l = 1; getline(cin, line); l++)
	{
		Position P;

		if (P.Play(line) != line.size())
		{
			cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << endl;
		}
		else
		{
			// solver.Reset();
			auto start = chrono::high_resolution_clock::now();
			unsigned int best_move = solver.FindBestMove(P);
			int score = solver.Solve(P);
			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double, milli> duration = end - start;

			cout << line
					  << ": " << P.nbMoves() << " moves, "
					  << "Score: " << score
					  << ", Nodes: " << solver.GetNodeCount()
					  << ", Time: " << duration.count()
					  << " ms, Best move: column " << best_move + 1 << "\n";
		}
	}
}

void printConnectFourBoard(const string &moves)
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
			cerr << "Invalid column: " << moves[i] << endl;
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
			cerr << "Column " << col + 1 << " is already full!" << endl;
			return;
		}

		// Place the piece (alternate between 'R' and 'Y')
		board[row][col] = (i % 2 == 0) ? 'x' : 'o';
	}

	// Print the board
	for (int i = 0; i < ROWS; i++)
	{
		cout << "|";
		for (int j = 0; j < COLS; j++)
		{
			cout << board[i][j] << "|";
		}
		cout << endl;
	}

	// Print column numbers
	cout << " ";
	for (int j = 1; j <= COLS; j++)
	{
		cout << j << " ";
	}
	cout << endl;
}

int startGame()
{
	Solver solver;

	string sequence = "444344535356";
	Position P;
	P.Play(sequence);

	cout << "Choose your side:\n"
			  << "[1]: Red\n"
			  << "[2]: Yellow\n"
			  << "Enter your choice: ";

	int choice;
	while (cin >> choice)
	{
		if (choice == 1)
		{
			break;
		}
		else if (choice == 2)
		{
			P.playCol(solver.FindBestMove(P));
			break;
		}
		else
		{
			cout << "Invalid choice\n"
					  << "Enter your choice: ";
		}
	}

	cout << "The game has started!\n";

	int player_move;
	while (1)
	{
		printConnectFourBoard(sequence);
		cout << "Enter your move: column: ";
		cin >> player_move;

		while (player_move < 1 || player_move > Position::WIDTH)
		{
			cout << "Invalid move\nEnter your move: ";
			cin >> player_move;
		}

		if (P.IsWinningMove(player_move - 1))
		{
			sequence += to_string(player_move);
			printConnectFourBoard(sequence);
			cout << "You win!\n";
			break;
		}

		sequence += to_string(player_move);
		P.playCol(player_move - 1);

		int ai_move = solver.FindBestMove(P);
		if (P.IsWinningMove(ai_move))
		{
			cout << "Bot has played: column " << ai_move + 1 << endl;
			sequence += to_string(ai_move + 1);
			printConnectFourBoard(sequence);
			cout << "You lose!\n";
			break;
		}
		P.playCol(ai_move);
		sequence += to_string(ai_move + 1);
		cout << "Bot has played: column " << ai_move + 1 << endl;
	}

	return 0;
}

vector<vector<int>> moveSequenceToBoard(const string &sequence)
{
	vector<vector<int>> board(6, vector<int>(7, 0));
	int current_player = 1;

	for (char move_char : sequence)
	{
		int col = move_char - '1'; // Convert char to column index (0-6)

		if (col < 0 || col > 6)
		{
			cerr << "Invalid move in sequence: " << move_char << endl;
			return vector<vector<int>>(); // Return an empty board for invalid sequence
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
			cerr << "Column full in sequence: " << move_char << endl;
			return vector<vector<int>>(); // Return an empty board for invalid sequence
		}
	}

	return board;
}

bool isValidBoard(const vector<vector<int>> &curr_board,
					const vector<vector<int>> &target_board)
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

bool isValidConnect4Board(const vector<vector<int>> &board)
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

bool dfsFindSequence(const vector<vector<int>> &target_board,
					   vector<vector<int>> &curr_board,
					   vector<int> &move_seq,
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
				if (dfsFindSequence(target_board, curr_board, move_seq,
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

string boardToMoveSequence(const vector<vector<int>> &board)
{
	// First, verify the board is valid Connect 4 board
	if (!isValidConnect4Board(board))
	{
		return ""; // Invalid board configuration
	}

	vector<int> move_seq;
	vector<vector<int>> curr_board(6, vector<int>(7, 0));

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
	if (dfsFindSequence(board, curr_board, move_seq, 1, remaining))
	{
		string result;
		for (int move : move_seq)
		{
			result += to_string(move);
		}
		return result;
	}

	// If player 1 starting didn't work, try player 2
	move_seq.clear();
	curr_board = vector<vector<int>>(6, vector<int>(7, 0));
	remaining = piece_count;
	if (dfsFindSequence(board, curr_board, move_seq, 2, remaining))
	{
		string result;
		for (int move : move_seq)
		{
			result += to_string(move);
		}
		return result;
	}

	return ""; // No valid sequence found
}

int testSequenceToBoard()
{
	string test_sequence;

	cout << "Enter a move sequence (e.g., 1234567): ";
	cin >> test_sequence;

	cout << "Testing sequence: " << test_sequence << "\n";
	vector<vector<int>> board = moveSequenceToBoard(test_sequence);

	if (board.empty())
	{
		cout << "Invalid sequence!\n";
		return 1;
	}

	cout << "\nFinal board state:\n";
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			cout << board[i][j] << " ";
		}
		cout << "\n";
	}

	return 0;
}

void testBoardInput()
{
	vector<vector<int>> board(6, vector<int>(7, 0));

	cout << "Enter the Connect 4 board state (6 rows, 7 columns)\n"
			  << "Use 0 for empty, 1 for player 1, 2 for player 2\n"
			  << "Enter each row from top to bottom:\n";

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			cin >> board[i][j];
			if (board[i][j] < 0 || board[i][j] > 2)
			{
				cout << "Invalid input\n";
				return;
			}
		}
	}

	string sequence = boardToMoveSequence(board);

	cout << "Move sequence: " << sequence << "\n";

	Position P;
	if (P.Play(sequence) != sequence.size())
	{
		cout << "Invalid sequence generated!\n";
	}
	else
	{
		Solver solver;
		solver.Reset();
		unsigned int best_move = solver.FindBestMove(P);
		int score = solver.Solve(P);

		cout << "Current position analysis:\n";
		cout << "Best move: column " << best_move + 1 << "\n";
		cout << "Position score: " << score << "\n";
	}
}

int runTestForBoardToSequence()
{
	ifstream testStream("tests/begin_hard.txt");
	ofstream resultStream("results_1.txt");

	if (!testStream || !resultStream)
	{
		cerr << "Cannot open test file.";
		return 1;
	}

	string line;
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

		vector<vector<int>> board = moveSequenceToBoard(line);

		if (board.empty())
		{
			resultStream << "Invalid move sequence!\n\n";
			continue;
		}

		// Convert board to move sequence
		auto start_seq = chrono::high_resolution_clock::now();
		string generated_sequence = boardToMoveSequence(board);
		auto end_seq = chrono::high_resolution_clock::now();
		chrono::duration<double, milli> duration_seq = end_seq - start_seq;

		if (generated_sequence.empty())
		{
			resultStream << "ERROR: Could not generate sequence from board\n\n";
			continue;
		}

		vector<vector<int>> final_board = moveSequenceToBoard(generated_sequence);

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

	cout << "Results written to results.txt\n";
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
			cerr << "Argument not found.\n";
			return 1;
		}
	}
	else
	{
		findMoveAndCalculateScore();
	}

	return 0;
}
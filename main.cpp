#include "headers/Solver.hpp"

#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std;

int runTest()
{
	Solver solver;
	ifstream testStream("tests/10_moves.test");

	if (!testStream)
	{
		cerr << "Cannot open test file.";
		return 1;
	}

	solver.LoadBook("depth_11_scores.book");


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
	solver.LoadBook("depth_11_scores.book");


	string line;
	while (getline(cin, line))
	{
		Position P;

		if (P.Play(line) != line.size())
		{
			cerr << "Invalid move: " << line << endl;
		}
		else
		{
			if (P.nbMoves() == 11) solver.Reset();
			auto start = chrono::high_resolution_clock::now();

			int score;
			unsigned int best_move;

			score = solver.Solve(P);
			best_move = solver.FindBestMove(P);

			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double, milli> duration = end - start;
			cout << line
				 << ": " << P.nbMoves() << " moves, "
				 << "Score: " << score
				 << ", Nodes: " << solver.GetNodeCount()
				 << ", Time: " << duration.count() << " ms"
				 << ", Best move: column " << best_move + 1 << "\n";
		}
	}
}

void continuouslyFindMoveAndCalculateScore()
{
	Solver solver;
	solver.LoadBook("depth_11_scores.book");

	string current_sequence;
	Position P;
	string line;
	while (getline(cin, line))
	{
		if (P.Play(line) != line.size())
		{
			cerr << "Invalid move: " << line << endl;
		}
		else
		{
			if (P.nbMoves() == 11) solver.Reset();
			current_sequence += line;
			auto start = chrono::high_resolution_clock::now();

			int score;
			unsigned int best_move;

			score = solver.Solve(P);
			best_move = solver.FindBestMove(P);

			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double, milli> duration = end - start;
			cout << current_sequence
				 << ": " << P.nbMoves() << " moves, "
				 << "Score: " << score
				 << ", Nodes: " << solver.GetNodeCount()
				 << ", Time: " << duration.count() << " ms"
				 << ", Best move: column " << best_move + 1 << "\n";
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

void startGame()
{
	Solver solver;
	solver.LoadBook("depth_11_scores.book");


	string sequence = "";
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
}

// void botVersusBot()
// {
// 	Position P;
// 	string current_sequence = "";

// 	Solver solver;
// 	bool turn = 0;
// 	while (1)
// 	{
// 		int move = solver.FindBestMove(P);
// 		if (P.IsWinningMove(move))
// 		{
// 			cout << ""
// 			break;
// 		}
// 		if (P.CanPlay(move))
// 		{

// 		}
// 	}
// }

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--test") == 0)
		{
			// The current test is tests/10_moves.test
			runTest();
		}
		else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--find_move") == 0)
		{
			findMoveAndCalculateScore();
		}
		else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--continuous_find") == 0)
		{
			continuouslyFindMoveAndCalculateScore();
		}
		else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--play") == 0)
		{
			startGame();
		}
		else
		{
			cerr << "Argument not found.\n";
			return 1;
		}
	}
	else
	{
		startGame();
	}

	return 0;
}
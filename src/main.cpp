#include "header/RequestHandler.hpp"
#include "header/Solver.hpp"

#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace std;

int runTest()
{
	Solver solver;
	ifstream testStream("tests/begin_hard.test");

	if (!testStream)
	{
		cerr << "Cannot open test file.";
		return 1;
	}

	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

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
	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

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
			int score;
			unsigned int best_move;
			auto start = chrono::high_resolution_clock::now();
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
			fflush(stdout);
		}
	}
}

void continuouslyFindMoveAndCalculateScore()
{
	Solver solver;
	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

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

void startPlayerVsBotGame()
{
	Solver solver;
	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

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
			int best_move = solver.FindBestMove(P);
			P.PlayCol(best_move);
			sequence += to_string(best_move + 1);
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
		P.PlayCol(player_move - 1);

		int ai_move = solver.FindBestMove(P);
		if (P.IsWinningMove(ai_move))
		{
			cout << "Bot has played: column " << ai_move + 1 << endl;
			sequence += to_string(ai_move + 1);
			printConnectFourBoard(sequence);
			cout << "You lose!\n";
			break;
		}
		P.PlayCol(ai_move);
		sequence += to_string(ai_move + 1);
		cout << "Bot has played: column " << ai_move + 1 << endl;
	}
}

void startBotGame()
{
	Solver solver;
	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

	cout << "\n<------------------>\n"
		 << "THE GAME HAS STARTED\n"
		 << "<------------------>\n\n";

	string sequence = "";
	Position P;
	P.Play(sequence);

	int move = -1;
	string player_name = "";
	bool is_red_turn = false;
	while (1)
	{
		is_red_turn = (sequence.size() + 1) % 2;
		cout << "Moves: " << sequence.size() << "\n";
		printConnectFourBoard(sequence);

		if (is_red_turn)
			player_name = "Red";
		else
			player_name = "Yellow";

		cout << player_name << " is thinking...\n";

		auto start = chrono::high_resolution_clock::now();
		move = solver.FindBestMove(P);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double, milli> duration = end - start;

		cout << player_name << " has played: column " << move + 1 << ", " << duration.count() << " ms.\n";

		if (P.IsWinningMove(move))
		{
			sequence += to_string(move + 1);
			printConnectFourBoard(sequence);
			cout << player_name << " won!\n";
			return;
		}

		P.PlayCol(move);
		sequence += to_string(move + 1);
	}
}

void startTraining()
{
	ofstream hard_moves_stream("hard_moves.txt");
	unordered_set<string> seen_lines;

	Solver solver;
	solver.LoadBookAndWarmup("data/depth_12_scores_7x6.book", "data/warmup.book");

	cout << "\n<------------------>\n"
		 << "THE GAME HAS STARTED\n"
		 << "<------------------>\n\n";

	string initial_sequence = "44444";
	string sequence = initial_sequence;
	Position P;
	P.Play(sequence);

	int move;
	string player_name;
	bool is_red_turn;
	while (1)
	{
		if (P.nbMoves() == 15)
		{
			sequence = initial_sequence;
			P = Position();
			P.Play(sequence);
		}

		is_red_turn = (sequence.size() + 1) % 2;
		cout << "Position: " << sequence << ", " << sequence.size() << " moves\n";

		if (is_red_turn)
			player_name = "Red";
		else
			player_name = "Yellow";

		cout << player_name << " is thinking...\n";

		auto start = chrono::high_resolution_clock::now();
		// Play randomly at the 7th move of the board
		// if (P.nbMoves() == 7)
		// 	move = solver.RandomMove();
		// else
		move = solver.FindBestMove(P);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double, milli> duration = end - start;

		if (duration.count() >= 2000 && seen_lines.find(sequence) == seen_lines.end())
		{
			cout << "HARD MOVE FOUND: " << sequence << "\n";
			seen_lines.insert(sequence);
			hard_moves_stream << sequence << "\n";
			hard_moves_stream.flush();
		}

		cout << player_name << " has played: column " << move + 1 << ", " << duration.count() << " ms.\n";

		if (P.IsWinningMove(move))
		{
			sequence += to_string(move + 1);
			printConnectFourBoard(sequence);
			cout << player_name << " won!\n";
			return;
		}

		P.PlayCol(move);
		sequence += to_string(move + 1);
	}
}

void handleAPIRequest(string ip, int port)
{
	RequestHandler requestHandler(ip, port);
	requestHandler.run();
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--test") == 0)
		{
			runTest();
		}
		else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--find") == 0)
		{
			findMoveAndCalculateScore();
		}
		else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--cfind") == 0)
		{
			continuouslyFindMoveAndCalculateScore();
		}
		else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--play") == 0)
		{
			startPlayerVsBotGame();
		}
		else if (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--botgame") == 0)
		{
			startBotGame();
		}
		else if (strcmp(argv[1], "-tr") == 0 || strcmp(argv[1], "--training") == 0)
		{
			startTraining();
		}
		else if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--web") == 0)
		{
			handleAPIRequest("0.0.0.0", 8112);
		}
		else
		{
			cerr << "Argument not found.\n";
			return 1;
		}
	}
	else
	{
		startPlayerVsBotGame();
	}

	return 0;
}
#include "header/RequestHandler.hpp"
#include "header/Solver.hpp"
#include "header/Game.hpp"
#include "lib/argparse.hpp"

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

	solver.GetReady();

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
	solver.GetReady();

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
		}
	}
}

void continuouslyFindMoveAndCalculateScore()
{
	Solver solver;
	solver.GetReady();

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

void startTraining()
{
	ofstream hard_moves_stream("hard_moves.txt");
	unordered_set<string> seen_lines;

	Solver solver;
	solver.GetReady();

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

		P.PlayCol(move);
		sequence += to_string(move + 1);
	}
}

void handleAPIRequest(string ip, int port)
{
	RequestHandler requestHandler(ip, port);
	requestHandler.Run();
}

int main(int argc, char **argv)
{
	argparse::ArgumentParser program("c4ai", "1.0", argparse::default_arguments::help);

	program.add_argument("-f", "--find").help("Find best move for a given sequence").flag();
	program.add_argument("-c", "--cfind").help("Continuously find best move as the user inputs").flag();
	program.add_argument("-t", "--test").help("Run the tests").flag();
	program.add_argument("-p", "--play").help("Play a game with our bot").flag();
	program.add_argument("-b", "--botgame").help("See a match between 2 bots").flag();
	program.add_argument("-tr", "--train").help("Perform a training session to find hard moves").flag();
	program.add_argument("-w", "--web").help("Handle API requests").flag();

	program.add_description("Connect four AI by Tralalero Tralala");

	try
	{
		program.parse_args(argc, argv);
		if (argc == 1)
		{
			std::cout << program;
			std::exit(0);
		}
	}
	catch (const std::exception &err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}

	int flag_count = 0;

	if (program["-f"] == true)
		++flag_count;
	if (program["-c"] == true)
		++flag_count;
	if (program["-t"] == true)
		++flag_count;
	if (program["-p"] == true)
		++flag_count;
	if (program["-b"] == true)
		++flag_count;
	if (program["-tr"] == true)
		++flag_count;
	if (program["-w"] == true)
		++flag_count;

	if (flag_count != 1)
	{
		std::cerr << "Error: You must specify exactly one option.\n";
		std::cerr << program;
		std::exit(1);
	}

	if (program["-t"] == true)
		runTest();
	else if (program["-f"] == true)
		findMoveAndCalculateScore();
	else if (program["-c"] == true)
		continuouslyFindMoveAndCalculateScore();
	else if (program["-p"] == true)
	{
		Game game;
		game.StartPlayerVsBotGame();
	}
	else if (program["-b"] == true)
	{
		Game game;
		game.StartBotGame();
	}
	else if (program["-tr"] == true)
		startTraining();
	else if (program["-w"] == true)
		handleAPIRequest("0.0.0.0", 8112);

	return 0;
}
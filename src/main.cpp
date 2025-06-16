#include "argparse.hpp"

#include "RequestHandler.hpp"
#include "Solver.hpp"
#include "Game.hpp"

#include <unordered_set>
#include <utility>

using namespace std;

int runTest(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	ifstream testStream("tests/10_moves.test");

	if (!testStream)
	{
		cerr << "Cannot open test file.";
		return 1;
	}

	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);;

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
			const int best_move = solver.FindBestMove(P);
			auto end = chrono::high_resolution_clock::now();
			chrono::duration<double, milli> duration = end - start;

			const int score = solver.Solve(P);

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

void findMoveAndCalculateScore(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);;

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
			auto start = chrono::high_resolution_clock::now();
			const int score = solver.Solve(P);
			const int best_move = solver.FindBestMove(P);
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

void continuouslyFindMoveAndCalculateScore(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);;

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

			const int score = solver.Solve(P);
			const unsigned int best_move = solver.FindBestMove(P);

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

void startTraining(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	ofstream hard_moves_stream("hard_moves.txt");
	unordered_set<string> seen_lines;

	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);;

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
	while (true)
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

void handleAPIRequest(string ip, const int port, const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);;
	RequestHandler requestHandler(solver, move(ip), port);
	requestHandler.Run();
}

int main(const int argc, char **argv)
{
	argparse::ArgumentParser program("c4ai", "1.0", argparse::default_arguments::help);

	program.add_argument("-f", "--find")
		.help("Find best move for a given sequence")
		.flag();
	program.add_argument("-c", "--cfind")
		.help("Continuously find best move as the user inputs")
		.flag();
	program.add_argument("-t", "--test")
		.help("Run the tests")
		.flag();
	program.add_argument("-p", "--play")
		.help("Play a game with our bot")
		.flag();
	program.add_argument("-b", "--botgame")
		.help("See a match between 2 bots")
		.flag();
	program.add_argument("-tr", "--train")
		.help("Perform a training session to find hard moves")
		.flag();
	program.add_argument("-w", "--web")
		.help("Handle API requests")
		.flag();

	// Named arguments (optional)
	program.add_argument("--opening-book")
		.help("Path to the opening book file")
		.default_value(string("data/depth_1-12_binary.book"));

	program.add_argument("--warmup-book")
		.help("Path to the warmup book file")
		.default_value(string("data/warmup_binary.book"));

	// Positional args (optional fallback)
	program.add_argument("opening_positional")
		.help("Positional opening book path")
		.nargs(0, 1);  // Accepts zero or one

	program.add_argument("warmup_positional")
		.help("Positional warmup book path")
		.nargs(0, 1);  // Accepts zero or one

	program.add_description("Connect four solver by adnope");

	try
	{
		program.parse_args(argc, argv);
		if (argc == 1)
		{
			cout << program;
			exit(0);
		}
	}
	catch (const exception &err)
	{
		cerr << err.what() << endl;
		cerr << program;
		exit(1);
	}

	// Determine final paths
	string OPENING_BOOK_PATH = program.is_used("opening_positional")
		? program.get("opening_positional")
		: program.get("--opening-book");

	string WARMUP_BOOK_PATH = program.is_used("warmup_positional")
		? program.get("warmup_positional")
		: program.get("--warmup-book");

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
		cerr << "Error: You must specify exactly one option.\n";
		cerr << program;
		exit(1);
	}

	if (program["-t"] == true)
		runTest(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
	else if (program["-f"] == true)
		findMoveAndCalculateScore(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
	else if (program["-c"] == true)
		continuouslyFindMoveAndCalculateScore(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
	else if (program["-p"] == true)
	{
		Solver solver;
		solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
		Game game(solver);
		game.StartPlayerVsBotGame();
	}
	else if (program["-b"] == true)
	{
		Solver solver;
		solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
		Game game(solver);
		game.StartBotGame();
	}
	else if (program["-tr"] == true)
		startTraining(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
	else if (program["-w"] == true)
		handleAPIRequest("0.0.0.0", 8112, OPENING_BOOK_PATH, WARMUP_BOOK_PATH);

	return 0;
}
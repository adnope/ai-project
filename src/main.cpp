#include "argparse.hpp"

#include "RequestHandler.hpp"
#include "Solver.hpp"
#include "Game.hpp"

#include <unordered_set>
#include <utility>

using namespace std;

void findMoveAndCalculateScore(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);

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

void startTraining(const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	ofstream hard_moves_stream("hard_moves.txt", std::ios::app);
	unordered_set<string> seen_lines;

	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);

	cout << "\nTRAINING SESSION STARTED!\n\n";

	string initial_sequence = "44444";
	string sequence = initial_sequence;
	Position P;
	P.Play(sequence);

	int move;
	string player_name;
	bool is_red_turn;
	while (true)
	{
		if (P.nbMoves() == 20)
		{
			sequence = initial_sequence;
			P = Position();
			P.Play(sequence);
		}

		is_red_turn = (sequence.size() + 1) % 2;
		// cout << "Position: " << sequence << ", " << sequence.size() << " moves\n";

		if (is_red_turn)
			player_name = "Red";
		else
			player_name = "Yellow";

		// cout << player_name << " is thinking...\n";

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
			cout << "HARD MOVE FOUND: " << sequence << ", " << duration.count() << " ms.\n";
			seen_lines.insert(sequence);
			hard_moves_stream << sequence << "\n";
			hard_moves_stream.flush();

			Position hard_pos;
			hard_pos.Play(sequence);
			for (int col = 0; col < Position::WIDTH; ++col)
			{
				Position next_pos(hard_pos);
				if (next_pos.CanPlay(col))
				{
					next_pos.PlayCol(col);
					uint64_t key = next_pos.Key3();
					uint8_t score = solver.Solve(next_pos) - Position::MIN_SCORE + 1;
					solver.transTable.PutOpeningMove(key, score);
				}
			}
		}
		P.PlayCol(move);
		sequence += to_string(move + 1);
	}
}

void handleAPIRequest(string ip, const int port, const string OPENING_BOOK_PATH, const string WARMUP_BOOK_PATH)
{
	Solver solver;
	solver.GetReady(OPENING_BOOK_PATH, WARMUP_BOOK_PATH);
	RequestHandler requestHandler(solver, move(ip), port);
	requestHandler.Run();
}

int main(const int argc, char **argv)
{
	argparse::ArgumentParser program("c4ai", "1.0", argparse::default_arguments::help);

	const vector<string> modes = {
		"find", "play", "botgame", "training", "server"};

	program.add_argument("mode")
		.help("Mode to run (find, play, botgame, training, server)")
		.required()
		.action([&modes](const string &value)
				{
            if (find(modes.begin(), modes.end(), value) == modes.end()) {
                throw runtime_error("Invalid mode '" + value + "'.");
            }
            return value; });

	program.add_argument("opening_book")
		.nargs(0, 1)
		.help("Optional opening book path")
		.default_value(string("data/opening.book")); // Default book contains all moves of depth 1 to depth 12

	program.add_argument("warmup_book")
		.nargs(0, 1)
		.help("Optional warmup book path")
		.default_value(string("data/warmup2.book"));

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error &err)
	{
		std::cerr << err.what() << "\n\n"
				  << program << std::endl;
		return 1;
	}

	const string mode = program.get<string>("mode");
	string opening_book = program.get<string>("opening_book");
	string warmup_book = program.get<string>("warmup_book");

	if (mode == modes[0])
	{
		findMoveAndCalculateScore(opening_book, warmup_book);
	}
	else if (mode == modes[1])
	{
		Solver solver;
		solver.GetReady(opening_book, warmup_book);
		Game game(solver);
		game.StartPlayerVsBotGame();
	}
	else if (mode == modes[2])
	{
		Solver solver;
		solver.GetReady(opening_book, warmup_book);
		Game game(solver);
		game.StartBotGame();
	}
	else if (mode == modes[3])
	{
		startTraining(opening_book, warmup_book);
	}
	else if (mode == modes[4])
	{
		handleAPIRequest("0.0.0.0", 8112, opening_book, warmup_book);
	}

	return 0;
}
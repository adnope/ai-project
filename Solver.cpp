#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include "headers/TranspositionTable.hpp"
#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/MoveSorter.hpp"

class Solver
{
private:
	unsigned long long nodeCount;

	// Use a column order to set priority for exploring nodes (columns tend to affect the game more the more they are near the middle)
	int columnOrder[Position::WIDTH];

	/**
	 * Reccursively score connect 4 position using negamax variant of alpha-beta algorithm.
	 * @param: alpha and beta, the window [alpha, beta] is used to narrow down states whose values are within the window
	 *
	 * @return the exact score, an upper or lower bound score depending of the case:
	 * - if actual score of position <= alpha then actual score <= return value <= alpha
	 * - if actual score of position >= beta then beta <= return value <= actual score
	 * - if alpha <= actual score <= beta then return value = actual score
	 */
	int negamax(const Position &P, int alpha, int beta)
	{
		assert(alpha < beta);
		assert(!P.canWinNext());

		nodeCount++;
		
		uint64_t next = P.possibleNonLosingMoves();
		if (next == 0)
			return -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2; // opponent wins since there are no possbile non-losing move

		if (P.nbMoves() >= Position::WIDTH * Position::HEIGHT - 2)
			return 0; // draw game

		// min is used for narrowing down the window (min means the smallest number of moves needed for the opponent to win)
		int min = -(Position::WIDTH * Position::HEIGHT - 2 - P.nbMoves()) / 2;
		if (alpha < min)
		{
			alpha = min; // no need to explore nodes whose values smaller than min
			if (alpha >= beta)
				return alpha; // prune the exploration if the [alpha;beta] window is empty.
		}

		// max is the smallest number of moves needed for the current player to win, also used to narrow down window.
		int max = (Position::WIDTH * Position::HEIGHT - 1 - P.nbMoves()) / 2;
		if (int val = transTable.get(P.key())) // check if the current state is in transTable or not, if it is, retrieve the value
			max = val + Position::MIN_SCORE - 1;

		if (beta > max)
		{
			beta = max; // no need to explore nodes whose values greater than max
			if (alpha >= beta)
				return beta; // prune the exploration if the [alpha;beta] window is empty.
		}

		MoveSorter moves;
		for (int i = Position::WIDTH; i--;)
			if (uint64_t move = next & Position::column_mask(columnOrder[i]))
				moves.add(move, P.moveScore(move));

		while (uint64_t next = moves.getNext())
		{
			Position P2(P);
			P2.play(next);							 // It's opponent turn in P2 position after current player plays x column.
			int score = -negamax(P2, -beta, -alpha); // explore opponent's score within [-beta;-alpha] windows:
			// no need to have good precision for score better than beta (opponent's score worse than -beta)
			// no need to check for score worse than alpha (opponent's score worse better than -alpha)

			if (score >= beta)
				return score; // prune the exploration if we find a possible move better than what we were looking for.
			if (score > alpha)
				alpha = score; // reduce the [alpha;beta] window for next exploration, as we only
							   // need to search for a position that is better than the best so far.
		}

		// save the upper bound of the position, minus MIN_SCORE and +1 to make sure the lowest value is 1
		transTable.put(P.key(), alpha - Position::MIN_SCORE + 1);
		return alpha;
	}

public:
	TranspositionTable transTable;

	int solve(const Position &P)
	{
		if (P.canWinNext()) // check if win in one move as the Negamax function does not support this case.
			return (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;

		int min = -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2;
		int max = (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;

		while (min < max)
		{ // iteratively narrow the min-max exploration window
			int med = min + (max - min) / 2;
			if (med <= 0 && min / 2 < med)
				med = min / 2;
			else if (med >= 0 && max / 2 > med)
				med = max / 2;
			int r = negamax(P, med, med + 1); // use a null depth window to know if the actual score is greater or smaller than med
			if (r <= med)
				max = r;
			else
				min = r;
		}
		return min;
	}

	unsigned int findBestMove(const Position &P)
	{
		int best_col;
		int best_score = -100;
		for (int col = 0; col < Position::WIDTH; ++col)
		{
			if (P.canPlay(col))
			{
				if (P.isWinningMove(col))
				{
					return col;
				}
				Position P2(P);
				P2.playCol(col);
				int score = -solve(P2);
				if (score > best_score)
				{
					best_score = score;
					best_col = col;
				}
			}
		}
		return best_col;
	}

	unsigned long long getNodeCount()
	{
		return nodeCount;
	}

	void reset()
	{
		nodeCount = 0;
		transTable.reset();
	}

	Solver() : nodeCount{0}, transTable(8388593)
	{ // 8388593 prime = 64MB of transposition table
		reset();
		for (int i = 0; i < Position::WIDTH; i++)
			columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
		// initialize the column exploration order, starting with center columns
		// example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
	}
};

int runTest()
{
	Solver solver;
	std::ifstream testStream("tests/begin_medium_test.txt");

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

	std::string sequence = "444433323556";
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
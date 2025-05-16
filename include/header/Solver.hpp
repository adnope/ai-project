#pragma once

#include "TranspositionTable.hpp"
#include "Position.hpp"
#include "MoveSorter.hpp"
#include "OpeningBook.hpp"

#include <random>
#include <chrono>
#include <sstream>
#include <map>
#include <algorithm>
#include <thread>

const char *OPENING_BOOK_PATH = "data/depth_1-12_binary.book";
const char *WARMUP_BOOK_PATH = "data/warmup_binary.book";

class Solver
{
private:
	unsigned long long nodeCount;

	// Use a column order to set priority for exploring nodes (columns tend to affect the game more the more they are near the middle)
	int columnOrder[Position::WIDTH];

	/**
	 * Recursively score connect 4 position using negamax variant of alpha-beta algorithm.
	 * @param: alpha and beta, the window [alpha, beta] is used to narrow down states whose values are within the window
	 *
	 * @return the exact score, an upper or lower bound score depending on the case:
	 * - if actual score of position <= alpha then actual score <= return value <= alpha
	 * - if actual score of position >= beta then beta <= return value <= actual score
	 * - if alpha <= actual score <= beta then return value = actual score
	 */
	int Negamax(const Position &P, int alpha, int beta)
	{
		assert(alpha < beta);
		// assert(!P.CanWinNext());

		nodeCount++;

		uint64_t next = P.PossibleNonLosingMoves();
		if (next == 0)
			return -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2; // opponent wins since there are no possible non-losing move

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
		if (int val = int(transTable.Get(P.Key3()))) // check if the current state is in transTable or not, if it is, retrieve the value
			max = val + Position::MIN_SCORE - 1;

		if (beta > max)
		{
			beta = max; // no need to explore nodes whose values greater than max
			if (alpha >= beta)
				return beta; // prune the exploration if the [alpha;beta] window is empty.
		}

		MoveSorter moves;
		for (int i = Position::WIDTH; i--;)
			if (uint64_t move = next & Position::ColumnMask(columnOrder[i]))
				moves.Add(move, P.MoveScore(move));

		while (uint64_t next = moves.GetNext())
		{
			Position P2(P);
			P2.Play(next);
			int score = -Negamax(P2, -beta, -alpha);

			if (score >= beta)
				return score; // prune the exploration
			if (score > alpha)
				alpha = score; // reduce the [alpha;beta] window
		}

		// save the upper bound of the position, minus MIN_SCORE and +1 to make sure the lowest value is 1
		transTable.Put(P.Key3(), alpha - Position::MIN_SCORE + 1);
		return alpha;
	}

public:
	TranspositionTable transTable;
	OpeningBook book = OpeningBook(&transTable);

	int Solve(const Position &P)
	{
		if (transTable.Get(P.Key3()) != 0)
		{
			return int(transTable.Get(P.Key3())) + Position::MIN_SCORE - 1;
		}
		if (P.CanWinNext()) // check if win in one move as the Negamax function does not support this case.
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
			int r = Negamax(P, med, med + 1); // use a null depth window to know if the actual score is greater or smaller than med
			if (r <= med)
				max = r;
			else
				min = r;
		}
		return min;
	}

	int FindBestMove(const Position &P)
	{
		if (P.isEmpty())
			return (Position::WIDTH + 1) / 2 - 1;
		std::vector<int> best_cols;
		int best_score = -100;
		for (int col = 0; col < Position::WIDTH; ++col)
		{
			if (P.CanPlay(col))
			{
				if (P.IsWinningMove(col))
				{
					return col;
				}
				Position P2(P);
				P2.PlayCol(col);
				int score = -Solve(P2);

				if (score > best_score)
				{
					best_score = score;
					best_cols.clear();
					best_cols.push_back(col);
				}
				else if (score == best_score)
				{
					best_cols.push_back(col);
				}
			}
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, best_cols.size() - 1);
		return best_cols[dist(gen)];
	}

	std::vector<std::vector<int>> Analyze(const Position &P)
	{
		// Simulate a timeout move
		// std::this_thread::sleep_for(std::chrono::seconds(10));
		// return {{3}, {1,4}, {0, 6, 2, 5}};

		std::vector<std::vector<int>> ranked_moves;
		std::map<int, std::vector<int>, std::greater<>> score_to_cols;

		std::random_device rd;
		std::mt19937 g(rd());

		if (P.isEmpty())
		{
			int middle_col = (Position::WIDTH + 1) / 2 - 1;
			ranked_moves.push_back({middle_col});
			return ranked_moves;
		}

		std::vector<int> winning_cols;
		for (int col = 0; col < Position::WIDTH; ++col)
		{
			if (P.CanPlay(col) && P.IsWinningMove(col))
			{
				winning_cols.push_back(col);
			}
		}

		if (!winning_cols.empty())
		{
			std::shuffle(winning_cols.begin(), winning_cols.end(), g);
			ranked_moves.push_back(winning_cols);
			return ranked_moves;
		}

		for (int col = 0; col < Position::WIDTH; ++col)
		{
			if (P.CanPlay(col))
			{
				Position P2(P);
				P2.PlayCol(col);
				int score = -Solve(P2);

				score_to_cols[score].push_back(col);
			}
		}

		for (const auto &entry : score_to_cols)
		{
			std::vector<int> cols = entry.second;
			std::shuffle(cols.begin(), cols.end(), g);
			ranked_moves.push_back(cols);
		}

		return ranked_moves;
	}

	int RandomMove()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, Position::WIDTH - 1);
		return dist(gen);
	}

	unsigned long long GetNodeCount()
	{
		return nodeCount;
	}

	void LoadBook(const char* openingbook_path)
	{
		auto start = std::chrono::high_resolution_clock::now();
		book.load(openingbook_path);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;
		std::cout << "Opening book: loaded " << transTable.GetOpeningTableSize() << " moves in " << duration.count() / 1000 << " seconds.\n";
		std::cout.flush();
	}

	void Warmup()
	{
		LoadBook(WARMUP_BOOK_PATH);
	}

	void GetReady()
	{
		LoadBook(OPENING_BOOK_PATH);
		Warmup();
	}

	int GetDefaultFirstMove() const
	{
		constexpr int DEFAULT_FIRST_MOVE = 3;
		return DEFAULT_FIRST_MOVE;
	}

	void Reset()
	{
		nodeCount = 0;
		transTable.Reset();
	}

	// memoization table size: 2^23: 8388617, 2^24: 16777259, 2^25: 33554467, 2^26: 67108879, 2^27: 134217757
	Solver() : nodeCount{0}, transTable(8388617) // 2^23 entries
	{
		Reset();
		for (int i = 0; i < Position::WIDTH; i++)
			columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
		// initialize the column exploration order, starting with center columns
		// example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
	}
};
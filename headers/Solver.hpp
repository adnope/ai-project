#pragma once

#include "TranspositionTable.hpp"
#include "Position.hpp"
#include "MoveSorter.hpp"
#include "OpeningBook.hpp"
#include <random>

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
	int Negamax(const Position &P, int alpha, int beta)
	{
		assert(alpha < beta);
		assert(!P.CanWinNext());

		nodeCount++;

		uint64_t next = P.PossibleNonLosingMoves();
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

	void LoadBook(const std::string& filename)
	{
		auto start = std::chrono::high_resolution_clock::now();
		book.load(filename);
		auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Opening book loaded: " << duration.count() / 1000 << " seconds.\n";
	}

	void Warmup(const std::string& filename)
	{
		auto start = std::chrono::high_resolution_clock::now();
        std::string line;
        std::string move;
        int score;
        int count = 0;
        std::ifstream ifs(filename);
        while (getline(ifs, line))
        {
            std::istringstream iss(line);
            iss >> move >> score;
            Position P;
            P.Play(move);
            transTable.Put(P.Key3(), uint8_t(score - Position::MIN_SCORE + 1));
            count++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Warmup complete: " << duration.count() / 1000 << " seconds.\n";
	}

	void LoadBookAndWarmup(const std::string &book_name, const std::string &warmup_book_name)
	{
		LoadBook(book_name);
		Warmup(warmup_book_name);
	}

	void Reset()
	{
		nodeCount = 0;
		transTable.Reset();
	}

	Solver() : nodeCount{0}, transTable(67108879) // 2^26 entries, ~1GB in RAM
	{
		Reset();
		for (int i = 0; i < Position::WIDTH; i++)
			columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
		// initialize the column exploration order, starting with center columns
		// example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
	}
};
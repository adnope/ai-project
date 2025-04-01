#include "Position.hpp"
#include "TranspositionTable.hpp"
#include <cassert>
#include <iostream>
#include <utility>
#include <climits>

class Solver
{
private:
	int columnOrder[Position::WIDTH];
	TranspositionTable transTable;

	int negamax(const Position &P, int alpha, int beta)
	{
		assert(alpha < beta);

		if (P.nbMoves() == Position::WIDTH * Position::HEIGHT) // check for draw game
			return 0;

		for (int x = 0; x < Position::WIDTH; x++) // check if current player can win next move
			if (P.canPlay(x) && P.isWinningMove(x))
				return (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;

		int max = (Position::WIDTH * Position::HEIGHT - 1 - P.nbMoves()) / 2; // upper bound of our score as we cannot win immediately
		if (int val = transTable.get(P.key()))
			max = val + Position::MIN_SCORE - 1;

		if (beta > max)
		{
			beta = max; // there is no need to keep beta above our max possible score.
			if (alpha >= beta)
				return beta; // prune the exploration if the [alpha;beta] window is empty.
		}

		for (int x = 0; x < Position::WIDTH; x++) // compute the score of all possible next move and keep the best one
			if (P.canPlay(columnOrder[x]))
			{
				Position P2(P);
				P2.play(columnOrder[x]);				 // It's opponent turn in P2 position after current player plays x column.
				int score = -negamax(P2, -beta, -alpha); // explore opponent's score within [-beta;-alpha] windows:
														 // no need to have good precision for score better than beta (opponent's score worse than -beta)
														 // no need to check for score worse than alpha (opponent's score worse better than -alpha)

				if (score >= beta)
					return score; // prune the exploration if we find a possible move better than what we were looking for.
				if (score > alpha)
					alpha = score; // reduce the [alpha;beta] window for next exploration, as we only
								   // need to search for a position that is better than the best so far.
			}

		transTable.put(P.key(), alpha - Position::MIN_SCORE + 1); // save the upper bound of the position
		return alpha;
	}

public:
	Solver() : transTable(16777186)
	{ // 16777186 prime = 128MB of transposition table
		reset();
		for (int i = 0; i < Position::WIDTH; i++)
			columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
	}

	int solve_iterative_deepening(const Position &P, bool weak = false)
	{
		int min = -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2;
		int max = (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;
		if (weak)
		{
			min = -1;
			max = 1;
		}

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

	int calculateScore(const Position &P)
	{
		int score = negamax(P, -Position::WIDTH * Position::HEIGHT / 2, Position::WIDTH * Position::HEIGHT / 2);
		// int score = solve_iterative_deepening(P, true);
		// std::cout << "Current board score: " << score << std::endl;
		return score;
	}

	std::pair<int, int> findBestMove(const Position &P)
	{
		int best_score = INT_MIN;
		int best_move = -1;

		for (int col = 0; col < Position::WIDTH; col++)
		{
			if (P.canPlay(col))
			{
				if (P.isWinningMove(col))
				{
					best_score = (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;
					return std::make_pair(col + 1, best_score);
				}
				else
				{
					Position P2(P);
					P2.play(col);

					int score = -negamax(P2, -Position::WIDTH * Position::HEIGHT / 2, Position::WIDTH * Position::HEIGHT / 2);

					if (score > best_score)
					{
						best_score = score;
						best_move = col;
					}
				}
			}
		}

		// std::cout << "Best move: column " << best_move + 1 << ", move score: " << best_score << std::endl;
		return std::make_pair(best_move + 1, best_score);
	}

	std::vector<int> analyzeAllMoves(const Position &P)
	{
		std::vector<int> scores(Position::WIDTH);
		for (int col = 0; col < Position::WIDTH; col++)
		{
			if (P.canPlay(col))
			{
				if (P.isWinningMove(col))
					scores[col] = (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;
				else
				{
					Position P2(P);
					P2.play(col);
					scores[col] = -negamax(P2, -Position::WIDTH * Position::HEIGHT / 2, Position::WIDTH * Position::HEIGHT / 2);
				}
			}
		}
		return scores;
	}

	void reset()
	{
		transTable.reset();
	}
};
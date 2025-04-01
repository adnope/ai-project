#ifndef POSITION_HPP
#define POSITION_HPP

#include <string>
#include <cstdint>
#include <cassert>
#include <iostream>

class Position
{
public:
	static const int WIDTH = 7;
	static const int HEIGHT = 6;
	static const int MIN_SCORE = -(WIDTH * HEIGHT) / 2 + 3;
	static const int MAX_SCORE = (WIDTH * HEIGHT + 1) / 2 - 3;

	static_assert(WIDTH < 10, "Board's width must be less than 10");
	static_assert(WIDTH * (HEIGHT + 1) <= 64, "Board does not fit in 64bits bitboard");

	bool canPlay(int col) const
	{
		return (mask & top_mask(col)) == 0;
	}

	void play(int col)
	{
		current_position ^= mask;
		mask |= mask + bottom_mask(col);
		moves++;
	}

	unsigned int play(std::string seq)
	{
		for (unsigned int i = 0; i < seq.size(); i++)
		{
			int col = seq[i] - '1';
			if (col < 0 || col >= Position::WIDTH || !canPlay(col) || isWinningMove(col))
			{
				if (isWinningMove(col)) {
					if (moves % 2 == 0) {
						play(col);
						return 200;
					}
					else if (moves % 2 == 1) {
						play(col);
						return 300;
					}
				}
				return i; // invalid move
			}
			play(col);
		}
		return seq.size();
	}

	bool isWinningMove(int col) const
	{
		uint64_t pos = current_position;
		pos |= (mask + bottom_mask(col)) & column_mask(col);
		return alignment(pos);
	}

	int nbMoves() const
	{
		return moves;
	}

	uint64_t key() const
	{
		return current_position + mask;
	}

	Position() : current_position{0}, mask{0}, moves{0} {}

private:
	uint64_t current_position;
	uint64_t mask;
	unsigned int moves;

	static bool alignment(uint64_t pos)
	{
		// horizontal
		uint64_t m = pos & (pos >> (HEIGHT + 1));
		if (m & (m >> (2 * (HEIGHT + 1))))
			return true;

		// diagonal 1
		m = pos & (pos >> HEIGHT);
		if (m & (m >> (2 * HEIGHT)))
			return true;

		// diagonal 2
		m = pos & (pos >> (HEIGHT + 2));
		if (m & (m >> (2 * (HEIGHT + 2))))
			return true;

		// vertical;
		m = pos & (pos >> 1);
		if (m & (m >> 2))
			return true;

		return false;
	}

	// STATIC BITMAPS
	// return a bitmask containg a single 1 corresponding to the top cel of a given column
	static uint64_t top_mask(int col)
	{
		return (UINT64_C(1) << (HEIGHT - 1)) << col * (HEIGHT + 1);
	}

	// return a bitmask containg a single 1 corresponding to the bottom cell of a given column
	static uint64_t bottom_mask(int col)
	{
		return UINT64_C(1) << col * (HEIGHT + 1);
	}

	// return a bitmask 1 on all the cells of a given column
	static uint64_t column_mask(int col)
	{
		return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
	}
};

#endif

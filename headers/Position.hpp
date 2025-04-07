#pragma once

#include <cstdint>
#include <assert.h>
#include <string>

/**
 * A class storing a Connect 4 position.
 * Functions are relative to the current player to play.
 * Position containing aligment are not supported by this class.
 *
 * A binary bitboard representationis used.
 * Each column is encoded on HEIGH+1 bits.
 *
 * Example of bit order to encode for a 7x6 board
 * .  .  .  .  .  .  .
 * 5 12 19 26 33 40 47
 * 4 11 18 25 32 39 46
 * 3 10 17 24 31 38 45
 * 2  9 16 23 30 37 44
 * 1  8 15 22 29 36 43
 * 0  7 14 21 28 35 42
 *
 * Position is stored as
 * - a bitboard "mask" with 1 on any color stones
 * - a bitboard "current_player" with 1 on stones of current player
 *
 * "current_player" bitboard can be transformed into a compact and non ambiguous key
 * by adding an extra bit on top of the last non empty cell of each column.
 * This allow to identify all the empty cells whithout needing "mask" bitboard
 *
 * current_player "x" = 1, opponent "o" = 0
 * board     position  mask      key       bottom
 *           0000000   0000000   0000000   0000000
 * .......   0000000   0000000   0001000   0000000
 * ...o...   0000000   0001000   0010000   0000000
 * ..xx...   0011000   0011000   0011000   0000000
 * ..ox...   0001000   0011000   0001100   0000000
 * ..oox..   0000100   0011100   0000110   0000000
 * ..oxxo.   0001100   0011110   1101101   1111111
 *
 * current_player "o" = 1, opponent "x" = 0
 * board     position  mask      key       bottom
 *           0000000   0000000   0001000   0000000
 * ...x...   0000000   0001000   0000000   0000000
 * ...o...   0001000   0001000   0011000   0000000
 * ..xx...   0000000   0011000   0000000   0000000
 * ..ox...   0010000   0011000   0010100   0000000
 * ..oox..   0011000   0011100   0011010   0000000
 * ..oxxo.   0010010   0011110   1110011   1111111
 *
 * key is an unique representation of a board key = position + mask + bottom
 * in practice, as bottom is constant, key = position + mask is also a
 * non-ambigous representation of the position.
 */

constexpr static uint64_t bottom(int width, int height)
{
	return width == 0 ? 0 : bottom(width - 1, height) | 1LL << (width - 1) * (height + 1);
}

class Position
{
public:
	static const int WIDTH = 7;
	static const int HEIGHT = 6;
	static const int MIN_SCORE = -(WIDTH * HEIGHT) / 2 + 3;
	static const int MAX_SCORE = (WIDTH * HEIGHT + 1) / 2 - 3;

	static_assert(WIDTH < 10, "Board's width must be less than 10");
	static_assert(WIDTH * (HEIGHT + 1) <= 64, "Board does not fit in 64bits bitboard");

	// return a bitmask 1 on all the cells of a given column
	static uint64_t column_mask(int col)
	{
		return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
	}

	bool canPlay(int col) const
	{
		return (mask & top_mask(col)) == 0;
	}

	void play(uint64_t move)
	{
		current_position ^= mask;
		mask |= move;
		moves++;
	}

	void playCol(int col)
	{
		play((mask + bottom_mask_col(col)) & column_mask(col));
	}

	unsigned int play(std::string seq)
	{
		for (unsigned int i = 0; i < seq.size(); i++)
		{
			int col = seq[i] - '1';
			if (col < 0 || col >= Position::WIDTH || !canPlay(col) || isWinningMove(col))
				return i; // invalid move
			playCol(col);
		}
		return seq.size();
	}

	bool canWinNext() const
	{
		return winning_position() & possible();
	}

	bool isWinningMove(int col) const
	{
		return winning_position() & possible() & column_mask(col);
	}

	int nbMoves() const
	{
		return moves;
	}

	uint64_t key() const
	{
		return current_position + mask;
	}

	uint64_t possibleNonLosingMoves() const
	{
		assert(!canWinNext());
		uint64_t possible_mask = possible();
		uint64_t opponent_win = opponent_winning_position();
		uint64_t forced_moves = possible_mask & opponent_win;
		if (forced_moves)
		{
			if (forced_moves & (forced_moves - 1)) // check if there is more than one forced move
				return 0;						   // the opponnent has two winning moves and you cannot stop him
			else
				possible_mask = forced_moves; // enforce to play the single forced move
		}
		return possible_mask & ~(opponent_win >> 1); // avoid to play below an opponent winning spot
	}

	int moveScore(uint64_t move) const
	{
		return countSetBits(compute_winning_position(current_position | move, mask));
	}

	Position() : current_position{0}, mask{0}, moves{0} {}

private:
	uint64_t current_position;
	uint64_t mask;
	unsigned int moves; // number of moves played since the beinning of the game.

	const static uint64_t bottom_mask_full = bottom(WIDTH, HEIGHT);
	const static uint64_t board_mask = bottom_mask_full * ((1LL << HEIGHT) - 1);

	// return a bitmask containg a single 1 corresponding to the top cel of a given column
	static uint64_t top_mask(int col)
	{
		return (UINT64_C(1) << (HEIGHT - 1)) << col * (HEIGHT + 1);
	}

	// return a bitmask containg a single 1 corresponding to the bottom cell of a given column
	static uint64_t bottom_mask_col(int col)
	{
		return UINT64_C(1) << col * (HEIGHT + 1);
	}

	uint64_t possible() const
	{
		return (mask + bottom_mask_full) & board_mask;
	}

	uint64_t winning_position() const
	{
		return compute_winning_position(current_position, mask);
	}

	uint64_t opponent_winning_position() const
	{
		return compute_winning_position(current_position ^ mask, mask);
	}

	static uint64_t compute_winning_position(uint64_t position, uint64_t mask)
	{
		// vertical;
		uint64_t r = (position << 1) & (position << 2) & (position << 3);

		// horizontal
		uint64_t p = (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
		r |= p & (position << 3 * (HEIGHT + 1));
		r |= p & (position >> (HEIGHT + 1));
		p = (position >> (HEIGHT + 1)) & (position >> 2 * (HEIGHT + 1));
		r |= p & (position << (HEIGHT + 1));
		r |= p & (position >> 3 * (HEIGHT + 1));

		// diagonal 1
		p = (position << HEIGHT) & (position << 2 * HEIGHT);
		r |= p & (position << 3 * HEIGHT);
		r |= p & (position >> HEIGHT);
		p = (position >> HEIGHT) & (position >> 2 * HEIGHT);
		r |= p & (position << HEIGHT);
		r |= p & (position >> 3 * HEIGHT);

		// diagonal 2
		p = (position << (HEIGHT + 2)) & (position << 2 * (HEIGHT + 2));
		r |= p & (position << 3 * (HEIGHT + 2));
		r |= p & (position >> (HEIGHT + 2));
		p = (position >> (HEIGHT + 2)) & (position >> 2 * (HEIGHT + 2));
		r |= p & (position << (HEIGHT + 2));
		r |= p & (position >> 3 * (HEIGHT + 2));

		return r & (board_mask ^ mask);
	}

	static unsigned int countSetBits(uint64_t num)
	{
		unsigned int c = 0;
		for (c = 0; num; c++)
			num &= num - 1;
		return c;
	}
};
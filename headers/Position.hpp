#pragma once

#include <cstdint>
#include <cassert>
#include <string>

constexpr static uint64_t Bottom(int width, int height)
{
	return width == 0 ? 0 : Bottom(width - 1, height) | 1LL << (width - 1) * (height + 1);
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
	static uint64_t ColumnMask(int col)
	{
		return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
	}

	bool CanPlay(int col) const
	{
		return (mask & TopMask(col)) == 0;
	}

	void Play(uint64_t move)
	{
		current_position ^= mask;
		mask |= move;
		moves++;
	}

	void playCol(int col)
	{
		Play((mask + BottomMaskCol(col)) & ColumnMask(col));
	}

	unsigned int Play(std::string seq)
	{
		for (unsigned int i = 0; i < seq.size(); i++)
		{
			int col = seq[i] - '1';
			if (col < 0 || col >= Position::WIDTH || !CanPlay(col) || IsWinningMove(col))
				return i; // invalid move
			playCol(col);
		}
		return seq.size();
	}

	bool CanWinNext() const
	{
		return WinningPosition() & Possible();
	}

	bool IsWinningMove(int col) const
	{
		return WinningPosition() & Possible() & ColumnMask(col);
	}

	int nbMoves() const
	{
		return moves;
	}

	uint64_t Key() const
	{
		return current_position + mask;
	}

	uint64_t PossibleNonLosingMoves() const
	{
		assert(!CanWinNext());
		uint64_t possible_mask = Possible();
		uint64_t opponent_win = OpponentWinningPosition();
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

	int MoveScore(uint64_t move) const
	{
		return CountSetBits(ComputeWinningPosition(current_position | move, mask));
	}

	// uint64_t Key3() const
	// {
	// 	uint64_t key_forward = 0;
	// 	for (int i = 0; i < Position::WIDTH; i++)
	// 		PartialKey3(key_forward, i);

	// 	uint64_t key_reverse = 0;
	// 	for (int i = Position::WIDTH; i--;)
	// 		PartialKey3(key_reverse, i);

	// 	return key_forward < key_reverse ? key_forward / 3 : key_reverse / 3;
	// }

	uint64_t Key3() const {
		uint64_t key_forward = 0;
		for(int i = 0; i < Position::WIDTH; i++) PartialKey3(key_forward, i);  // compute key in increasing order of columns
	
		uint64_t key_reverse = 0;
		for(int i = Position::WIDTH; i--;) PartialKey3(key_reverse, i);  // compute key in decreasing order of columns
	
		return key_forward < key_reverse ? key_forward / 3 : key_reverse / 3; // take the smallest key and divide per 3 as the last base3 digit is always 0
	}

	// void PartialKey3(uint64_t &key, int col) const
	// {
	// 	for (uint64_t pos = UINT64_C(1) << (col * (Position::HEIGHT + 1)); pos & mask; pos <<= 1)
	// 	{
	// 		key *= 3;
	// 		if (pos & current_position)
	// 			key += 1;
	// 		else
	// 			key += 2;
	// 	}
	// 	key *= 3;
	// }

	void PartialKey3(uint64_t &key, int col) const {
		for(uint64_t pos = UINT64_C(1) << (col * (Position::HEIGHT + 1)); pos & mask; pos <<= 1) {
		  key *= 3;
		  if(pos & current_position) key += 1;
		  else key += 2;
		}
		key *= 3;
	  }

	Position() : current_position{0}, mask{0}, moves{0} {}

private:
	uint64_t current_position;
	uint64_t mask;
	unsigned int moves;

	const static uint64_t bottom_mask_full = Bottom(WIDTH, HEIGHT);
	const static uint64_t board_mask = bottom_mask_full * ((1LL << HEIGHT) - 1);

	// return a bitmask containg a single 1 corresponding to the top cel of a given column
	static uint64_t TopMask(int col)
	{
		return (UINT64_C(1) << (HEIGHT - 1)) << col * (HEIGHT + 1);
	}

	// return a bitmask containg a single 1 corresponding to the bottom cell of a given column
	static uint64_t BottomMaskCol(int col)
	{
		return UINT64_C(1) << col * (HEIGHT + 1);
	}

	uint64_t Possible() const
	{
		return (mask + bottom_mask_full) & board_mask;
	}

	uint64_t WinningPosition() const
	{
		return ComputeWinningPosition(current_position, mask);
	}

	uint64_t OpponentWinningPosition() const
	{
		return ComputeWinningPosition(current_position ^ mask, mask);
	}

	static uint64_t ComputeWinningPosition(uint64_t position, uint64_t mask)
	{
		// vertical;
		uint64_t result = (position << 1) & (position << 2) & (position << 3);

		// horizontal
		uint64_t temp_pos = (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
		result |= temp_pos & (position << 3 * (HEIGHT + 1));
		result |= temp_pos & (position >> (HEIGHT + 1));
		temp_pos = (position >> (HEIGHT + 1)) & (position >> 2 * (HEIGHT + 1));
		result |= temp_pos & (position << (HEIGHT + 1));
		result |= temp_pos & (position >> 3 * (HEIGHT + 1));

		// diagonal 1
		temp_pos = (position << HEIGHT) & (position << 2 * HEIGHT);
		result |= temp_pos & (position << 3 * HEIGHT);
		result |= temp_pos & (position >> HEIGHT);
		temp_pos = (position >> HEIGHT) & (position >> 2 * HEIGHT);
		result |= temp_pos & (position << HEIGHT);
		result |= temp_pos & (position >> 3 * HEIGHT);

		// diagonal 2
		temp_pos = (position << (HEIGHT + 2)) & (position << 2 * (HEIGHT + 2));
		result |= temp_pos & (position << 3 * (HEIGHT + 2));
		result |= temp_pos & (position >> (HEIGHT + 2));
		temp_pos = (position >> (HEIGHT + 2)) & (position >> 2 * (HEIGHT + 2));
		result |= temp_pos & (position << (HEIGHT + 2));
		result |= temp_pos & (position >> 3 * (HEIGHT + 2));

		return result & (board_mask ^ mask);
	}

	static unsigned int CountSetBits(uint64_t num)
	{
		unsigned int c = 0;
		for (c = 0; num; c++)
			num &= num - 1;
		return c;
	}
};
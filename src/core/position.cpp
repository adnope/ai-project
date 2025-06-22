#include "position.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

Position::Position(const std::vector<std::vector<int>> &board)
    : current_position{0}, mask{0}, num_moves{0} {
  for (const auto &v : board) {
    for (const int i : v) {
      if (i == 1 || i == 2) {
        num_moves++;
      }
    }
  }

  const int current_player = (num_moves % 2 == 0) ? 1 : 2;

  for (size_t row = 0; row < board.size(); ++row) {
    for (size_t col = 0; col < board[0].size(); ++col) {
      if (board[row][col] == 1 || board[row][col] == 2) {
        const uint64_t move = UINT64_C(1) << (7 * col + 5 - row);
        mask |= move;
        if (board[row][col] == current_player) {
          current_position |= move;
        }
      }
    }
  }
}

bool Position::CanPlay(const int col) const {
  return (mask & TopMask(col)) == 0;
}

void Position::Play(const uint64_t move) {
  current_position ^= mask;
  mask |= move;
  num_moves++;
}

void Position::PlayCol(const int col) {
  Play((mask + BottomMaskCol(col)) & ColumnMask(col));
}

unsigned int Position::Play(const std::string &seq) {
  for (unsigned int i = 0; i < seq.size(); i++) {
    const int col = seq[i] - '1';
    if (col < 0 || col >= Position::WIDTH || !CanPlay(col) ||
        IsWinningMove(col)) {
      return i; // invalid move
    }
    PlayCol(col);
  }
  return seq.size();
}

uint64_t Position::PossibleNonLosingMoves() const {
  assert(!CanWinNext());
  uint64_t possible_mask = Possible();
  const uint64_t opponent_win = OpponentWinningPosition();
  const uint64_t forced_moves = possible_mask & opponent_win;
  if (forced_moves != 0) {
    if ((forced_moves & (forced_moves - 1)) != 0) {
      // check if there is more than one forced move
      return 0; // the opponent has two winning moves and you cannot stop him
    }
    possible_mask = forced_moves;
    // enforce to play the single forced move
  }
  return possible_mask & ~(opponent_win >> 1);
  // avoid to play below an opponent winning spot
}

uint64_t Position::Key3() const {
  uint64_t key_forward = 0;
  for (int i = 0; i < Position::WIDTH; i++) {
    PartialKey3(key_forward, i); // compute key in increasing order of columns
  }

  uint64_t key_reverse = 0;
  for (int i = Position::WIDTH; i-- != 0;) {
    PartialKey3(key_reverse, i); // compute key in decreasing order of columns
  }

  return key_forward < key_reverse ? key_forward / 3 : key_reverse / 3;
  // take the smallest key and divide per 3 as the last base3 digit is always 0
}

uint64_t Position::ComputeWinningPosition(const uint64_t position,
                                          const uint64_t mask) {
  // vertical;
  uint64_t result = (position << 1) & (position << 2) & (position << 3);

  // horizontal
  uint64_t temp_pos =
      (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
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

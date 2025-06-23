#pragma once

#include <cassert>
#include <climits>
#include <cstdint>
#include <string>
#include <vector>

constexpr static uint64_t Bottom(const int width, const int height) {
  return width == 0
             ? 0
             : Bottom(width - 1, height) | 1LL << (width - 1) * (height + 1);
}

// Representation of a game state, using 2 main bitmask: mask and
// current_position Example: board 0  0  0  0  0  0  0 0  0  0  1  0  0  0 0  0
// 0  2  0  0  0 0  0  1  1  0  0  0 0  0  2  2  0  0  0 0  0  1  1  0  0  0
// mask
// 0  0  0  0  0  0  0
// 0  0  0  1  0  0  0
// 0  0  0  1  0  0  0
// 0  0  1  1  0  0  0
// 0  0  1  1  0  0  0
// 0  0  1  1  0  0  0
// current_position
// 0  0  0  0  0  0  0
// 0  0  0  1  0  0  0
// 0  0  0  0  0  0  0
// 0  0  1  1  0  0  0
// 0  0  0  0  0  0  0
// 0  0  1  1  0  0  0

class Position {
public:
  static constexpr int WIDTH = 7;
  static constexpr int HEIGHT = 6;
  static constexpr int MIN_SCORE = (-(WIDTH * HEIGHT) / 2) + 3;
  static constexpr int MAX_SCORE = ((WIDTH * HEIGHT + 1) / 2) - 3;

  static_assert(WIDTH * (HEIGHT + 1) <= static_cast<int>(sizeof(uint64_t) * CHAR_BIT),
                "Board does not fit in 64bits bitboard");

  Position() : current_position{0}, mask{0}, num_moves{0} {}

  explicit Position(const std::vector<std::vector<int>> &board);

  // return a bitmask 1 on all the cells of a given column
  static uint64_t ColumnMask(const int col) {
    return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
  }

  bool CanPlay(int col) const;

  void Play(uint64_t move);

  void PlayCol(int col);

  unsigned int Play(const std::string &seq);

  bool CanWinNext() const { return (WinningPosition() & Possible()) != 0; }

  bool IsWinningMove(const int col) const {
    return (WinningPosition() & Possible() & ColumnMask(col)) != 0;
  }

  int NumMoves() const { return num_moves; }

  uint64_t PossibleNonLosingMoves() const;

  int MoveScore(const uint64_t move) const {
    return CountSetBits(ComputeWinningPosition(current_position | move, mask));
  }

  uint64_t Key3() const;

  bool isEmpty() const { return (mask == 0); }

  uint64_t GetMask() const { return mask; }

  uint64_t GetCurrentPosition() const { return current_position; }

private:
  static constexpr uint64_t bottom_mask_full = Bottom(WIDTH, HEIGHT);
  static constexpr uint64_t board_mask =
      bottom_mask_full * ((1LL << HEIGHT) - 1);

  uint64_t current_position;
  uint64_t mask;
  int num_moves;

  // return a bitmask containing a single 1 corresponding to the top cell
  // of a given column
  static uint64_t TopMask(const int col) {
    return (UINT64_C(1) << (HEIGHT - 1)) << col * (HEIGHT + 1);
  }

  // return a bitmask containing a single 1 corresponding to the bottom cell
  // of a given column
  static uint64_t BottomMaskCol(const int col) {
    return UINT64_C(1) << col * (HEIGHT + 1);
  }

  static uint64_t ComputeWinningPosition(uint64_t position, uint64_t mask);

  static int CountSetBits(const uint64_t num) {
    return __builtin_popcountll(num);
  }

  uint64_t Possible() const { return (mask + bottom_mask_full) & board_mask; }

  uint64_t WinningPosition() const {
    return ComputeWinningPosition(current_position, mask);
  }

  uint64_t OpponentWinningPosition() const {
    return ComputeWinningPosition(current_position ^ mask, mask);
  }

  void PartialKey3(uint64_t &key, const int col) const {
    for (uint64_t pos = UINT64_C(1) << (col * (Position::HEIGHT + 1));
         (pos & mask) != 0; pos <<= 1) {
      key *= 3;
      if ((pos & current_position) != 0) {
        key += 1;
      } else {
        key += 2;
      }
    }
    key *= 3;
  }
};

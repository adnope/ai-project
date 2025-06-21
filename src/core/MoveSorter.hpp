#pragma once

#include <array>

#include "Position.hpp"

/**
 * A class to predetermine the best column to explore for the searching algorithm.
 * There is an array storing a move and its corresponding score.
 * When adding a move to the array, it's sorted so that entries[size-1] always
 * have the best score.
 * The getNext() function is used to get the best move.
 */
class MoveSorter {
public:
  void Add(uint64_t move, int score);

  uint64_t GetNext();

  void Reset() {
    size = 0;
  }

private:
  unsigned int size = 0;

  struct Entry {
    uint64_t move;
    int score;
  };

  std::array<Entry, Position::WIDTH> entries = {};
};

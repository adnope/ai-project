#include "move_sorter.hpp"

#include <cstdint>

void MoveSorter::Add(const uint64_t move, const int score) {
  unsigned int pos = size++;
  for (; pos != 0 && entries[pos - 1].score > score; --pos) {
    entries[pos] = entries[pos - 1];
  }

  entries[pos].move = move;
  entries[pos].score = score;
}

uint64_t MoveSorter::GetNext() {
  if (size != 0) {
    return entries[--size].move;
  }
  return 0;
}
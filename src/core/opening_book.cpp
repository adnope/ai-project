#include "opening_book.hpp"

#include <cstdint>
#include <fstream>

void OpeningBook::load(const std::string &book_file) const {
  std::ifstream binary_file(book_file, std::ios::binary);
  uint64_t move_key;
  uint8_t score;

  while (
      binary_file.read(reinterpret_cast<char *>(&move_key), sizeof(move_key)) &&
      binary_file.read(reinterpret_cast<char *>(&score), sizeof(score))) {
    T->PutOpeningMove(move_key, score);
  }
}

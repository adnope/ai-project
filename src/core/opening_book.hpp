#pragma once

#include <string>

#include "transposition_table.hpp"

class OpeningBook {
public:
  explicit OpeningBook(TranspositionTable *table) : T(table) {}

  void load(const std::string &book_file) const;

private:
  TranspositionTable *T;
};

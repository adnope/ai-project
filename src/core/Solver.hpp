#pragma once

#include "TranspositionTable.hpp"
#include "Position.hpp"
#include "MoveSorter.hpp"
#include "OpeningBook.hpp"

class Solver {
public:
  static constexpr int DEFAULT_FIRST_MOVE = 3;

  Solver() : transTable(TABLE_SIZE) {
    Reset();
    for (int i = 0; i < Position::WIDTH; i++) {
      columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
    }
    // initialize the column exploration order, starting with center columns
    // example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
  }

  int Solve(const Position &P);

  int FindBestMove(const Position &P);

  std::vector<std::vector<int> > Analyze(const Position &P);

  static int RandomMove();

  void LoadOpeningBook(const std::string &OPENING_BOOK_PATH) const {
    book.load(OPENING_BOOK_PATH);
  }

  void Warmup(const std::string &WARMUP_BOOK_PATH) const {
    book.load(WARMUP_BOOK_PATH);
  }

  void GetReady(const std::string &OPENING_BOOK_PATH,
                const std::string &WARMUP_BOOK_PATH) const;

  void Reset() {
    nodeCount = 0;
    transTable.Reset();
  }

  uint64_t GetNodeCount() const {
    return nodeCount;
  }

  TranspositionTable &GetTranspositionTable() {
    return transTable;
  }

private:
  // memoization table size: 2^23: 8388617, 2^24: 16777259,
  // 2^25: 33554467, 2^26: 67108879, 2^27: 134217757
  static constexpr int TABLE_SIZE = 8388617;
  TranspositionTable transTable;
  OpeningBook book = OpeningBook(&transTable);
  uint64_t nodeCount = 0;

  // Use a column order to set priority for exploring nodes (columns tend to
  // affect the game more the more they are near the middle)
  std::array<int, Position::WIDTH> columnOrder;

  int Negamax(const Position &P, int alpha, int beta);
};

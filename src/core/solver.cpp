#include "solver.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include "move_sorter.hpp"

/**
   * Recursively score connect 4 position using negamax & alpha-beta algorithm.
   * @param P position to calculate score
   * @param alpha, beta: alpha and beta, the window [alpha, beta] is used to
   * narrow down states whose values are within the window
   * @return the exact score, an upper or lower bound score depending on the
   * case:
   * - if actual score <= alpha then actual score <= return value <= alpha
   * - if actual score >= beta then beta <= return value <= actual score
   * - if alpha <= actual score <= beta then return value = actual score
   */
int Solver::Negamax(const Position &P, int alpha, int beta) {
  assert(alpha < beta);
  assert(!P.CanWinNext());

  nodeCount++;

  const uint64_t next = P.PossibleNonLosingMoves();
  if (next == 0) {
    // opponent wins since there are no possible non-losing move
    return -((Position::WIDTH * Position::HEIGHT) - P.NumMoves()) / 2;
  }

  if (P.NumMoves() >= Position::WIDTH * Position::HEIGHT - 2) {
    // draw game
    return 0;
  }

  // min is used for narrowing down the window (min means the smallest number
  // of moves needed for the opponent to win)
  const int min = -((Position::WIDTH * Position::HEIGHT) - 2 - P.NumMoves()) /
                  2;
  if (alpha < min) {
    alpha = min; // no need to explore nodes whose values smaller than min
    if (alpha >= beta) {
      return alpha;
    }
    // prune the exploration if the [alpha;beta] window is empty.
  }

  // max is the smallest number of moves needed for the current player to win,
  // also used to narrow down window.
  int max = (Position::WIDTH * Position::HEIGHT - 1 - P.NumMoves()) / 2;
  if (const int val = static_cast<int>(transTable.Get(P.Key3()))) {
    // check if the current state is in transTable or not, if it is, retrieve
    // the value
    max = val + Position::MIN_SCORE - 1;
  }

  if (beta > max) {
    beta = max; // no need to explore nodes whose values greater than max
    if (alpha >= beta) {
      return beta;
    }
    // prune the exploration if the [alpha;beta] window is empty.
  }

  MoveSorter moves;
  for (int i = Position::WIDTH; i-- != 0;) {
    if (const uint64_t move = next & Position::ColumnMask(columnOrder[i])) {
      moves.Add(move, P.MoveScore(move));
    }
  }

  while (const uint64_t next_move = moves.GetNext()) {
    Position P2(P);
    P2.Play(next_move);
    const int score = -Negamax(P2, -beta, -alpha);

    if (score >= beta) {
      return score; // prune the exploration
    }
    alpha = std::max(score, alpha); // reduce the [alpha;beta] window
  }

  // save the upper bound of the position, minus MIN_SCORE and +1 to make
  // sure the lowest value is 1
  transTable.Put(P.Key3(), alpha - Position::MIN_SCORE + 1);
  return alpha;
}

int Solver::Solve(const Position &P) {
  if (transTable.Get(P.Key3()) != 0) {
    return static_cast<int>(transTable.Get(P.Key3())) + Position::MIN_SCORE -
           1;
  }
  if (P.CanWinNext()) {
    // check if win in one move as the Negamax function does not support this
    // case.
    return (Position::WIDTH * Position::HEIGHT + 1 - P.NumMoves()) / 2;
  }

  int min = -((Position::WIDTH * Position::HEIGHT) - P.NumMoves()) / 2;
  int max = (Position::WIDTH * Position::HEIGHT + 1 - P.NumMoves()) / 2;

  while (min < max) {
    // iteratively narrow the min-max exploration window
    int med = min + ((max - min) / 2);
    if (med <= 0 && min / 2 < med) {
      med = min / 2;
    } else if (med >= 0 && max / 2 > med) {
      med = max / 2;
    }
    const int r = Negamax(P, med, med + 1);
    // use a null depth window to know if the actual score is greater or
    // smaller than med
    if (r <= med) {
      max = r;
    } else {
      min = r;
    }
  }
  return min;
}

int Solver::FindBestMove(const Position &P) {
  if (P.isEmpty()) {
    return ((Position::WIDTH + 1) / 2) - 1;
  }
  std::vector<int> best_cols;
  int best_score = INT_MIN;
  for (int col = 0; col < Position::WIDTH; ++col) {
    if (P.CanPlay(col)) {
      if (P.IsWinningMove(col)) {
        return col;
      }
      Position P2(P);
      P2.PlayCol(col);
      const int score = -Solve(P2);

      if (score > best_score) {
        best_score = score;
        best_cols.clear();
        best_cols.push_back(col);
      } else if (score == best_score) {
        best_cols.push_back(col);
      }
    }
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, static_cast<int>
                                       (best_cols.size() - 1));
  return best_cols[dist(gen)];
}

std::vector<std::vector<int> > Solver::Analyze(const Position &P) {
  std::vector<std::vector<int> > ranked_moves;
  std::map<int, std::vector<int>, std::greater<> > score_to_cols;

  std::random_device rd;
  std::mt19937 g(rd());

  if (P.isEmpty()) {
    int middle_col = ((Position::WIDTH + 1) / 2) - 1;
    ranked_moves.push_back({middle_col});
    return ranked_moves;
  }

  std::vector<int> winning_cols;
  for (int col = 0; col < Position::WIDTH; ++col) {
    if (P.CanPlay(col) && P.IsWinningMove(col)) {
      winning_cols.push_back(col);
    }
  }

  if (!winning_cols.empty()) {
    std::shuffle(winning_cols.begin(), winning_cols.end(), g);
    ranked_moves.push_back(winning_cols);
    return ranked_moves;
  }

  for (int col = 0; col < Position::WIDTH; ++col) {
    if (P.CanPlay(col)) {
      Position P2(P);
      P2.PlayCol(col);
      int score = -Solve(P2);

      score_to_cols[score].push_back(col);
    }
  }

  for (auto &[score, cols]: score_to_cols) {
    std::shuffle(cols.begin(), cols.end(), g);
    ranked_moves.push_back(cols);
  }

  return ranked_moves;
}

int Solver::RandomMove() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, Position::WIDTH - 1);
  return dist(gen);
}

void Solver::GetReady(const std::string &OPENING_BOOK_PATH,
                      const std::string &WARMUP_BOOK_PATH) const {
  using hr_clock = std::chrono::high_resolution_clock;
  const auto open_start = hr_clock::now();
  LoadOpeningBook(OPENING_BOOK_PATH);
  const auto open_end = hr_clock::now();
  const std::chrono::duration<double> open_taken = open_end - open_start;
  const size_t open_num_moves = transTable.GetOpeningTableSize();

  const auto warmup_start = hr_clock::now();
  Warmup(WARMUP_BOOK_PATH);
  const auto warmup_end = hr_clock::now();
  const std::chrono::duration<double> warmup_taken = warmup_end - warmup_start;
  const size_t warmup_num_moves =
      transTable.GetOpeningTableSize() - open_num_moves;

  std::cout << "Opening book: loaded " << open_num_moves << " moves in " <<
      open_taken.count() << " seconds.\n";
  std::cout << "Warmup book: loaded " << warmup_num_moves << " moves in " <<
      warmup_taken.count() << " seconds.\n";
  std::cout.flush();
}

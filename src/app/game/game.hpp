#pragma once

#include <string>

#include "core/solver.hpp"

class Game {
public:
  explicit Game(Solver solver) : solver(std::move(solver)) {}

  void StartPlayerVsBotGame();

  void StartBotGame();

private:
  Solver solver;

  static void printConnectFourBoard(const std::string &sequence);
};

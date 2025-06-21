#pragma once

#include <string>

#include "Solver.hpp"

class Game {
public:
  explicit Game(Solver solver) : solver(std::move(solver)) {
  }

  void StartPlayerVsBotGame();

  void StartBotGame();

private:
  Solver solver;

  static void printConnectFourBoard(const std::string &sequence);
};

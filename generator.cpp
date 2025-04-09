#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/TranspositionTable.hpp"
#include "headers/Solver.hpp"
#include "headers/MoveSorter.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>

std::string CalculateScore(const std::string& line)
{
    Solver solver;
    Position P;

    if (P.play(line) != line.size())
    {
        std::cerr << "Invalid move at move " << (P.nbMoves() + 1) << ": \"" << line << "\"" << std::endl;
    }

    solver.reset();
    int score = solver.solve(P);

    std::ostringstream result;
    result << line << " " << score;
    std::cout << result.str() << "\n";

    return result.str();
}

void generateAllMoves(int length) {
    std::ofstream outfile("moves_score.txt", std::ios::app); 
    std::vector<std::string> buffer;
    int validCount = 0;

    long long total = 1;
    for (int i = 0; i < length; ++i)
        total *= 7;

    for (long long i = 0; i < total; ++i) {
        long long num = i;
        std::string moves;
        std::vector<int> columnCount(7, 0);

        for (int j = 0; j < length; ++j) {
            int digit = num % 7;
            columnCount[digit]++;
            if (columnCount[digit] > 6) {
                moves.clear();
                break;
            }
            moves += ('1' + digit);
            num /= 7;
        }

        if (moves.empty()) continue;
        std::reverse(moves.begin(), moves.end());

        if (std::stoi(moves) <= 1111122162) continue;

        std::string line = CalculateScore(moves);
        if (!line.empty()) {
            buffer.push_back(line);
            validCount++;
        }

        if (buffer.size() == 100) {
            for (const std::string& entry : buffer)
                outfile << entry << "\n";
            buffer.clear();
            outfile.flush();
        }
    }

    for (const std::string& entry : buffer)
        outfile << entry << "\n";
    outfile.flush();
    outfile.close();
}

void generate_opening_book() {
  static constexpr int BOOK_SIZE = 27; // store 2^BOOK_SIZE positions in the book
  static constexpr int DEPTH = 14;     // max depth of every position to be stored
  static constexpr double LOG_3 = 1.58496250072; // log2(3)
  TranspositionTable* table = new TranspositionTable(1<<BOOK_SIZE);

  long long count = 1;
  for(std::string line; getline(std::cin, line); count++) {
    if(line.length() == 0) break; // empty line = end of input
    std::istringstream iss(line);
    std::string moves;
    getline(iss, moves, ' '); // read position before first space character
    int score;
    iss >> score;

    Position P;
    if(iss.fail() || !iss.eof()
        || P.play(moves) != moves.length()
        || score < Position::MIN_SCORE || score > Position::MAX_SCORE) {  // a valid line is a position a space and a valid score
      std::cerr << "Invalid line (line ignored): " << line << std::endl;
      continue;
    }
    
    table->put(table->encodeMoves(moves), score - Position::MIN_SCORE + 1);
    if(count % 1000000 == 0) std::cerr << count << std::endl;
  }  

  OpeningBook book{Position::WIDTH, Position::HEIGHT, DEPTH, table};

  std::ostringstream book_file;
  book_file << Position::WIDTH << "x" << Position::HEIGHT << ".book";
  book.save(book_file.str());
}

int main(int argc, char** argv) {
//   for (int i = 1; i <= 10; i++) {
//     generateAllMoves(i);
//   }
  generate_opening_book();
}
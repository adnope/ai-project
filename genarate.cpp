#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

void generate_opening_book() {
  static constexpr int BOOK_SIZE = 27; // store 2^BOOK_SIZE positions in the book
  static constexpr int DEPTH = 14;     // max depth of every position to be stored
  static constexpr double LOG_3 = 1.58496250072; // log2(3)
  TranspositionTable* table = new TranspositionTable(1<<BOOK_SIZE);

  long long count = 1;
  for(std::string line; getline(std::cin, line); count++) {
    if(line.length() == 0) break; // empty line = end of input
    std::istringstream iss(line);
    std::string pos;
    getline(iss, pos, ' '); // read position before first space character
    int col;
    iss >> col;

    Position P;
    if(iss.fail() || !iss.eof()
        || P.play(pos) != pos.length()
        || col < Position::MIN_SCORE || col > Position::MAX_SCORE) {  // a valid line is a position a space and a valid score
      std::cerr << "Invalid line (line ignored): " << line << std::endl;
      continue;
    }
    
    table->put(table->encodeMoves(pos), col);
    if(count % 1000000 == 0) std::cerr << count << std::endl;
  }  

  OpeningBook book{Position::WIDTH, Position::HEIGHT, DEPTH, table};

  std::ostringstream book_file;
  book_file << Position::WIDTH << "x" << Position::HEIGHT << ".book";
  book.save(book_file.str());
}

int main(int argc, char** argv) {
  generate_opening_book();
}
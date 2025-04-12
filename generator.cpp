#include "headers/Position.hpp"
#include "headers/OpeningBook.hpp"
#include "headers/Solver.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <fstream>

std::ofstream moves("moves.txt", std::ios::app);

std::unordered_set<uint64_t> visited;
const int BUFFER_SIZE = 20;
std::vector<std::string> buffer;

int number_of_moves = 0;

void explore(const Position &P, char *pos_str, const int depth)
{
    uint64_t key = P.key3();
    if (!visited.insert(key).second)
        return; // already explored position

    int nb_moves = P.nbMoves();
    if (nb_moves <= depth)
    {
        // std::cout << pos_str << std::endl;
        moves << pos_str << std::endl;
        number_of_moves++;
    }
    if (nb_moves >= depth)
        return; // do not explore at further depth

    for (int i = 0; i < Position::WIDTH; i++) // explore all possible moves
        if (P.canPlay(i) && !P.isWinningMove(i))
        {
            Position P2(P);
            P2.playCol(i);
            pos_str[nb_moves] = '1' + i;
            explore(P2, pos_str, depth);
            pos_str[nb_moves] = 0;
        }
}

void flushBuffer(const std::string& filename) {
    std::ofstream out(filename, std::ios::app);
    for (const std::string& line : buffer) {
        out << line << "\n";
    }
    buffer.clear();
}

void calculateScore()
{
    std::ifstream moves_file("movex.txt");
    Solver solver;
    std::string line;

    while (getline(moves_file, line))
    {
        Position P;
        if (P.play(line) != line.length()) {
            std::cerr << "Invalid line (ignored): " << line << std::endl;
            continue;
        }

        int score = solver.solve(P);
        std::string line_with_score = line + " " + std::to_string(score);
        std::cout<<line_with_score << "\n";
        buffer.emplace_back(line_with_score);

        if (buffer.size() >= BUFFER_SIZE) {
            flushBuffer("moves_withx.txt");
        }
    }

    if (!buffer.empty()) {
        flushBuffer("moves_withx.txt");
    }
}

void generate_opening_book() {
    static constexpr int BOOK_SIZE = 27; 
    static constexpr double LOG_3 = 1.58496250072; 
    static constexpr double DEPth = 14;
    TranspositionTable* table = new TranspositionTable(1<<BOOK_SIZE);

    long long count = 1;
    for(std::string line; getline(std::cin, line); count++) {
        if(line.length() == 0) break;
        std::istringstream iss(line);
        std::string pos;
        getline(iss, pos, ' ');
        int score;
        iss >> score;

        Position P;
        if(iss.fail() || !iss.eof()
            || P.play(pos) != pos.length()
            || score < Position::MIN_SCORE || score > Position::MAX_SCORE) {  
        std::cerr << "Invalid line (line ignored): " << line << std::endl;
        continue;
        }
        table->put(P.key3(), score - Position::MIN_SCORE + 1);
        if(count % 1000000 == 0) std::cerr << count << std::endl;
    }

    OpeningBook book{Position::WIDTH, Position::HEIGHT, table};

    std::ostringstream book_file;
    book_file << Position::WIDTH << "x" << Position::HEIGHT << ".book";
    book.save(book_file.str());
}


int main(int argc, char **argv)
{
    if (argc > 1)
    {
		if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--explore") == 0)
		{
            int depth = atoi(argv[1]);
            char pos_str[depth + 1] = {0};
            explore(Position(), pos_str, depth);
            std::cout << "Number of moves: " << number_of_moves;
		}
		else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--calculateScore") == 0)
		{
			calculateScore();
		}
        else generate_opening_book();
    }
    return 0;
}
    
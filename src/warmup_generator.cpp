#include "header/Position.hpp"
#include "header/Solver.hpp"

#include <unordered_set>

using namespace std;

void removeDuplicateLines(const string &file_name)
{
    ifstream input_file(file_name);
    if (!input_file.is_open())
        return;

    vector<string> unique_lines;
    unordered_set<string> seen_lines;
    string line;

    while (getline(input_file, line))
    {
        if (seen_lines.insert(line).second)
        {
            unique_lines.push_back(line);
        }
    }
    input_file.close();

    ofstream output_file(file_name, ios::trunc);
    if (!output_file.is_open())
        return;

    for (const auto &uniqueLine : unique_lines)
    {
        output_file << uniqueLine << "\n";
    }
    output_file.close();
}

void genMoves(string input_file)
{
    Solver solver;
    solver.GetReady();

    string line;
    ofstream ofs("data/warmup.book", ios::app);

    ifstream ifs(input_file);
    int count = 0;
    while (getline(ifs, line))
    {
        count++;
        Position P;
        P.Play(line);
        for (int i = 0; i <= 6; ++i)
        {
            Position P2(P);
            if (P2.CanPlay(i))
            {
                P2.PlayCol(i);
                int score = solver.Solve(P2);
                string line2 = line;
                line2 += to_string(i + 1);
                ofs << line2 << " " << score << "\n";
            }
        }
        ofs.flush();
        cout << "Line " << count << " processed.\n";
    }
}

int main()
{
    removeDuplicateLines("data/hard_moves.txt");
    genMoves("data/hard_moves.txt");
    removeDuplicateLines("data/warmup.book");
    return 0;
}

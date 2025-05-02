#include "header/Position.hpp"
#include "header/Solver.hpp"
#include <unordered_set>

using namespace std;

void removeDuplicateLines(const string &file_name)
{
    ifstream inFile(file_name);
    if (!inFile.is_open())
        return;

    vector<string> uniqueLines;
    unordered_set<string> seenLines;
    string line;

    while (getline(inFile, line))
    {
        if (seenLines.insert(line).second)
        {
            uniqueLines.push_back(line);
        }
    }
    inFile.close();

    ofstream outFile(file_name, ios::trunc);
    if (!outFile.is_open())
        return;

    for (const auto &uniqueLine : uniqueLines)
    {
        outFile << uniqueLine << "\n";
    }
    outFile.close();
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

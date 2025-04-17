#include "headers/Position.hpp"
#include "headers/Solver.hpp"
#include <fstream>
#include <unordered_set>
#include <iostream>
#include <chrono>

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

void loadOpeningBook(Solver &solver, string book_name)
{
    auto start = chrono::high_resolution_clock::now();
    solver.LoadBook(book_name);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    cout << "Loaded opening book in " << duration.count() / 1000 << " seconds.\n";
}

void genMoves(string input_file)
{
    Solver solver;
    loadOpeningBook(solver, "depth_12_scores_7x6.book");
    string line;
    ofstream ofs("warmup.book", ios::app);

    ifstream ifs(input_file);
    while (getline(ifs, line))
    {
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
    }
}

int main(int argc, char const *argv[])
{
    removeDuplicateLines("hard_moves.txt");
    genMoves("hard_moves.txt");
    removeDuplicateLines("warmup.book");
    return 0;
}

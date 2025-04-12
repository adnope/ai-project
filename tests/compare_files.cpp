#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char const *argv[])
{
    const std::string moves_path = "move_score_3.txt";
    const std::string result_path = "moves_with_scores_133001-399000.txt";

    std::ifstream moves_stream(moves_path);
    std::ifstream result_stream(result_path);

    int error_count = 0;

    int line_count = 0;
    std::string move;
    std::string result;
    std::string result_move;
    while (getline(moves_stream, move) && getline(result_stream, result))
    {
        line_count++;
        std::istringstream iss(result);
        getline(iss, result_move, ' ');

        if (result_move != move)
        {
            std::cout << "Line " << line_count << " mismatched!";
            error_count++;
        }
    }


    std::cout << line_count << " lines checked, " << error_count << " lines mismatched\n";

    return 0;
}

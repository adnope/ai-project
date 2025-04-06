#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include "headers/TranspositionTable.hpp"
#include "headers/Position.hpp"
#include <vector>
#include <string>

class Solver
{
private:
	unsigned long long nodeCount;

	// Use a column order to set priority for exploring nodes (columns tend to affect the game more the more they are near the middle)
	int columnOrder[Position::WIDTH];

	TranspositionTable transTable;

	/**
	 * Reccursively score connect 4 position using negamax variant of alpha-beta algorithm.
	 * @param: alpha and beta, the window [alpha, beta] is used to narrow down states whose values are within the window
	 *
	 * @return the exact score, an upper or lower bound score depending of the case:
	 * - if actual score of position <= alpha then actual score <= return value <= alpha
	 * - if actual score of position >= beta then beta <= return value <= actual score
	 * - if alpha <= actual score <= beta then return value = actual score
	 */
	int negamax(const Position &P, int alpha, int beta)
	{
		assert(alpha < beta);
		assert(!P.canWinNext());

		nodeCount++;

		uint64_t next = P.possibleNonLosingMoves();
		if (next == 0)
			return -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2; // opponent wins since there are no possbile non-losing move

		if (P.nbMoves() >= Position::WIDTH * Position::HEIGHT - 2)
			return 0; // draw game

		// min is used for narrowing down the window (min means the smallest number of moves needed for the opponent to win)
		int min = -(Position::WIDTH * Position::HEIGHT - 2 - P.nbMoves()) / 2;
		if (alpha < min)
		{
			alpha = min; // no need to explore nodes whose values smaller than min
			if (alpha >= beta)
				return alpha; // prune the exploration if the [alpha;beta] window is empty.
		}

		// max is the smallest number of moves needed for the current player to win, also used to narrow down window.
		int max = (Position::WIDTH * Position::HEIGHT - 1 - P.nbMoves()) / 2;
		if (int val = transTable.get(P.key())) // check if the current state is in transTable or not, if it is, retrieve the value
			max = val + Position::MIN_SCORE - 1;

		if (beta > max)
		{
			beta = max; // no need to explore nodes whose values greater than max
			if (alpha >= beta)
				return beta; // prune the exploration if the [alpha;beta] window is empty.
		}

		// Compute values of each column to see which is best to move
		for (int x = 0; x < Position::WIDTH; x++)
			if (next & Position::column_mask(columnOrder[x]))
			{
				Position P2(P);
				P2.play(columnOrder[x]);
				int score = -negamax(P2, -beta, -alpha); // Basically how negamax works, read the wiki

				if (score >= beta)
					return score; // prune the search if we find a move better than the current best
				if (score > alpha)
					alpha = score; // reduce the [alpha;beta] window
			}

		// save the upper bound of the position, minus MIN_SCORE and +1 to make sure the lowest value is 1
		transTable.put(P.key(), alpha - Position::MIN_SCORE + 1);
		return alpha;
	}

public:
	int solve(const Position &P)
	{
		if (P.canWinNext()) // check if win in one move as the Negamax function does not support this case.
			return (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;

		int min = -(Position::WIDTH * Position::HEIGHT - P.nbMoves()) / 2;
		int max = (Position::WIDTH * Position::HEIGHT + 1 - P.nbMoves()) / 2;

		while (min < max)
		{ // iteratively narrow the min-max exploration window
			int med = min + (max - min) / 2;
			if (med <= 0 && min / 2 < med)
				med = min / 2;
			else if (med >= 0 && max / 2 > med)
				med = max / 2;
			int r = negamax(P, med, med + 1); // use a null depth window to know if the actual score is greater or smaller than med
			if (r <= med)
				max = r;
			else
				min = r;
		}
		return min;
	}

	unsigned int findBestMove(const Position &P)
	{
		int best_col;
		int best_score = -100;
		for (int col = 0; col < Position::WIDTH; ++col)
		{
			if (P.canPlay(col))
			{
				if (P.isWinningMove(col))
				{
					return col;
				}
				Position P2(P);
				P2.play(col);
				int score = -solve(P2);
				if (score > best_score)
				{
					best_score = score;
					best_col = col;
				}
			}
		}
		return best_col;
	}

	unsigned long long getNodeCount()
	{
		return nodeCount;
	}

	void reset()
	{
		nodeCount = 0;
		transTable.reset();
	}

	Solver() : nodeCount{0}, transTable(8388593)
	{ // 8388593 prime = 64MB of transposition table
		reset();
		for (int i = 0; i < Position::WIDTH; i++)
			columnOrder[i] = Position::WIDTH / 2 + (1 - 2 * (i % 2)) * (i + 1) / 2;
		// initialize the column exploration order, starting with center columns
		// example for WIDTH=7: columnOrder = {3, 4, 2, 5, 1, 6, 0}
	}
};

int runTest()
{
	Solver solver;
	std::ifstream testStream("tests/begin_medium_test.txt");

	if (!testStream)
	{
		std::cerr << "Cannot open test file.";
		return 1;
	}

	std::string line;
	int correct_score;

	int l = 0;
	while (testStream >> line && testStream >> correct_score)
	{
		Position P;
		if (P.play(line) != line.size())
		{
			std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
		}
		else
		{
			solver.reset();
			auto start = std::chrono::high_resolution_clock::now();
			unsigned int best_move = solver.findBestMove(P);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end - start;

			int score = solver.solve(P);

			std::cout << line
					  << ": " << P.nbMoves() << " moves, "
					  << "Score: " << score
					  << ", Nodes: " << solver.getNodeCount()
					  << ", Time: " << duration.count()
					  << " ms, Best move: column " << best_move + 1 << " - ";

			if (score == correct_score)
				std::cout << "[Correct!]" << std::endl;
			else
				std::cout << "[INCORRECT]";
		}
		l++;
	}

	return 0;
}

void findMoveAndCalculateScore() {
	Solver solver;

	std::string line;

	for (int l = 1; std::getline(std::cin, line); l++)
	{
		Position P;
		if (P.play(line) != line.size())
		{
			std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
		}
		else
		{
			solver.reset();
			auto start = std::chrono::high_resolution_clock::now();
			unsigned int best_move = solver.findBestMove(P);
			int score = solver.solve(P);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> duration = end - start;

			std::cout << line
					  << ": " << P.nbMoves() << " moves, "
					  << "Score: " << score
					  << ", Nodes: " << solver.getNodeCount()
					  << ", Time: " << duration.count()
					  << " ms, Best move: column " << best_move + 1 << "\n";
		}
	}
}


std::vector<std::vector<int>> move_sequence_to_board(const std::string& sequence) {
    std::vector<std::vector<int>> board(6, std::vector<int>(7, 0));
    int current_player = 1;
    
    for (char move_char : sequence) {
        int col = move_char - '1'; // Convert char to column index (0-6)
        
        if (col < 0 || col > 6) {
            std::cerr << "Invalid move in sequence: " << move_char << std::endl;
            return std::vector<std::vector<int>>(); // Return an empty board for invalid sequence
        }
        
        // Find the lowest empty row in the column
        int row = 5;
        while (row >= 0 && board[row][col] != 0) {
            --row;
        }
        
        // If the column is not full, place the piece
        if (row >= 0) {
            board[row][col] = current_player;
            current_player = (current_player == 1) ? 2 : 1;
        } else {
            std::cerr << "Column full in sequence: " << move_char << std::endl;
            return std::vector<std::vector<int>>(); // Return an empty board for invalid sequence
        }
    }
    
    return board;
}



bool is_valid_board(const std::vector<std::vector<int>>& curr_board, 
                   const std::vector<std::vector<int>>& target_board) {
    // Check if current board could lead to target board
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 7; c++) {
            if (curr_board[r][c] != 0 && curr_board[r][c] != target_board[r][c]) {
                return false;
            }
        }
    }
    return true;
}

bool is_valid_connect4_board(const std::vector<std::vector<int>>& board) {
    // Check if pieces are properly stacked due to gravity
    for (int col = 0; col < 7; col++) {
        for (int row = 4; row >= 0; row--) {
            if (board[row][col] != 0 && board[row+1][col] == 0) {
                return false; // Floating piece found
            }
        }
    }
    return true;
}

bool dfs_find_sequence(const std::vector<std::vector<int>>& target_board,
                      std::vector<std::vector<int>>& curr_board,
                      std::vector<int>& move_seq,
                      int curr_player,
                      int& piece_count) {
    
    // Base case: if we've placed all the pieces in the target board
    if (piece_count == 0) {
        return curr_board == target_board;
    }

    // Try each column
    for (int col = 0; col < 7; ++col) {
        // Find lowest empty row in this column
        int row = 5;
        while (row >= 0 && curr_board[row][col] != 0) {
            --row;
        }

        // If column is not full
        if (row >= 0) {
            // Make the move only if it matches target board
            if (target_board[row][col] == curr_player) {
                curr_board[row][col] = curr_player;
                move_seq.push_back(col + 1);
                piece_count--;

                // Recursively try next move with other player
                if (dfs_find_sequence(target_board, curr_board, move_seq, 
                                    curr_player == 1 ? 2 : 1, piece_count)) {
                    return true;
                }

                // If this path didn't work, undo the move
                curr_board[row][col] = 0;
                move_seq.pop_back();
                piece_count++;
            }
        }
    }

    return false;
}

std::string board_to_move_sequence(const std::vector<std::vector<int>>& board) {
    // First, verify the board is valid Connect 4 board
    if (!is_valid_connect4_board(board)) {
        return ""; // Invalid board configuration
    }

    std::vector<int> move_seq;
    std::vector<std::vector<int>> curr_board(6, std::vector<int>(7, 0));

    // Count total pieces
    int piece_count = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (board[i][j] != 0) piece_count++;
        }
    }
    
    // Try starting with player 1
    int remaining = piece_count;
    if (dfs_find_sequence(board, curr_board, move_seq, 1, remaining)) {
        std::string result;
        for (int move : move_seq) {
            result += std::to_string(move);
        }
        return result;
    }
    
    // If player 1 starting didn't work, try player 2
    move_seq.clear();
    curr_board = std::vector<std::vector<int>>(6, std::vector<int>(7, 0));
    remaining = piece_count;
    if (dfs_find_sequence(board, curr_board, move_seq, 2, remaining)) {
        std::string result;
        for (int move : move_seq) {
            result += std::to_string(move);
        }
        return result;
    }

    return ""; // No valid sequence found
}

int testSequenceToBoard() {
    std::string test_sequence;

	std::cout << "Enter a move sequence (e.g., 1234567): ";
	std::cin >> test_sequence;
    
    std::cout << "Testing sequence: " << test_sequence << "\n";
    std::vector<std::vector<int>> board = move_sequence_to_board(test_sequence);
    
    if (board.empty()) {
        std::cout << "Invalid sequence!\n";
        return 1;
    }
    
    std::cout << "\nFinal board state:\n";
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            std::cout << board[i][j] << " ";
        }
        std::cout << "\n";
    }
    
    return 0;
}


void testBoardInput() {
    std::vector<std::vector<int>> board(6, std::vector<int>(7, 0));
    
    std::cout << "Enter the Connect 4 board state (6 rows, 7 columns)\n" 
			  << "Use 0 for empty, 1 for player 1, 2 for player 2\n" 
			<< "Enter each row from top to bottom:\n";
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            std::cin >> board[i][j];
            if (board[i][j] < 0 || board[i][j] > 2) {
                std::cout << "Invalid input\n";
                return;
            }
        }
    }
    

    std::string sequence = board_to_move_sequence(board);
    

    std::cout << "Move sequence: " << sequence << "\n";
    

    Position P;
    if (P.play(sequence) != sequence.size()) {
        std::cout << "Invalid sequence generated!\n";
    } else {
        Solver solver;
        solver.reset();
        unsigned int best_move = solver.findBestMove(P);
        int score = solver.solve(P);
        
        std::cout << "Current position analysis:\n";
        std::cout << "Best move: column " << best_move + 1 << "\n";
        std::cout << "Position score: " << score << "\n";
    }
}


int runTestForBoardToSequence()
{
    std::ifstream testStream("tests/begin_hard.txt");
	std::ofstream resultStream("results_1.txt");


    if (!testStream || !resultStream)
    {
        std::cerr << "Cannot open test file.";
        return 1;
    }

    std::string line;
    int correct_score;
	int total_tests = 0;
    int successful_matches = 0;

	resultStream << "Connect 4 Board-to-Sequence Test Results\n";
    resultStream << "========================================\n\n";

   
    while (testStream >> line >> correct_score)
    {
		total_tests++;
		resultStream << "Test case #" << total_tests + 1 << "\n";
		resultStream << "-------------\n";
        resultStream << "Original sequence: " << line << "\n";


      
        std::vector<std::vector<int>> board = move_sequence_to_board(line);
       
        if (board.empty())
        {
            resultStream << "Invalid move sequence!\n\n";
            continue;
        }

        // Convert board to move sequence
        auto start_seq = std::chrono::high_resolution_clock::now();
        std::string generated_sequence = board_to_move_sequence(board);
        auto end_seq = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_seq = end_seq - start_seq;

		if (generated_sequence.empty()) {
            resultStream << "ERROR: Could not generate sequence from board\n\n";
            continue;
        }

        std::vector<std::vector<int>> final_board = move_sequence_to_board(generated_sequence);



		bool boards_match = (board == final_board);
		if (boards_match) successful_matches++;

		
		resultStream << "Board state:\n";
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 7; j++) {
				resultStream << board[i][j] << " ";
			}
			resultStream << "\n";
		}


        resultStream << "Boards match: " << (boards_match ? "Yes" : "No") << "\n";
        resultStream << "Conversion time: " << duration_seq.count() << " ms\n";
        resultStream << "----------------------------------------\n\n";
    }

	// Write summary
    resultStream << "\nTest Summary\n";
    resultStream << "============\n";
    resultStream << "Total tests run: " << total_tests << "\n";
    resultStream << "Successful matches: " << successful_matches << "\n";
    resultStream << "Failed matches: " << (total_tests - successful_matches) << "\n";
    resultStream << "Success rate: " << (successful_matches * 100.0 / total_tests) << "%\n";

    testStream.close();
    resultStream.close();

    std::cout << "Results written to results.txt\n";
    return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1 && argv[1][0] == '-')
	{
		switch (argv[1][1])
		{
			case 't':
				runTest();
				break;
			case 's':
				runTestForBoardToSequence();
				break;
			case 'q':  
				testSequenceToBoard();
				break;
			case 'f':
				findMoveAndCalculateScore();
				break;
			case 'b':
				testBoardInput();
				break;
			default:
				findMoveAndCalculateScore();
		}
	} else {
		testBoardInput();
	}

	return 0;
}
#pragma once

#include "Position.hpp"

/**
 * A class to predetermine the best column to explore for the searching algorithm.
 * There is an array storing a move and its corresponding score.
 * When adding a move to the array, it's sorted so that entries[size-1] always have the best score.
 * The getNext() function is used to get the best move.
 */
class MoveSorter
{
public:
    void add(uint64_t move, int score)
    {
        int pos = size;
        ++size;

        if (pos != 0)
        {
            for (; entries[pos - 1].score > score; --pos)
                entries[pos] = entries[pos - 1];
        }

        entries[pos].move = move;
        entries[pos].score = score;
    }

    uint64_t getNext()
    {
        if (size)
        {
            return entries[size - 1].move;
            --size;
        }
        else
            return 0;
    }

    void reset()
    {
        size = 0;
    }

    MoveSorter() : size{0}
    {
    }

private:
    unsigned int size;

    struct
    {
        uint64_t move;
        int score;
    } entries[Position::WIDTH];
};
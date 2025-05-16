#pragma once

#include "Position.hpp"
#include "TranspositionTable.hpp"

#include <iostream>
#include <fstream>
#include <string>

class OpeningBook
{
public:
    TranspositionTable *T;

    OpeningBook(TranspositionTable *table) : T(table) {}

    void load(const char* binarybook_name)
    {
        std::ifstream binary_file(binarybook_name, std::ios::binary);

        uint64_t move_key;
        uint8_t score;

        while (binary_file.read(reinterpret_cast<char*>(&move_key), sizeof(move_key)) &&
            binary_file.read(reinterpret_cast<char*>(&score), sizeof(score))) {
            T->PutOpeningMove(move_key, score);
        }
    }
};
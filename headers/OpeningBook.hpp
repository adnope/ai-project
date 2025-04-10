#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include "Position.hpp"
#include "TranspositionTable.hpp"

class OpeningBook {
public:
    static constexpr uint8_t NO_MOVE = 255;
    static constexpr size_t MAX_ENTRIES = 2'500'000; // ~20MB (8 bytes/entry)

    int width, height;
    TranspositionTable* T;

    OpeningBook(int width, int height, TranspositionTable* table)
        : width(width), height(height), T(table) {}

    void addPosition(uint64_t key, uint8_t bestMove) 
    {
        if (bestMove < Position::WIDTH) {
            T->put(key, bestMove);
        }
    }

    void save(const std::string& filename) const 
    {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) 
        {
            std::cerr << "Error: Can't open file " << filename << "\n";
            return;
        }

        size_t count = 0;
        for (unsigned int i = 0; i < T->getSize(); ++i) 
        {
            if (count >= MAX_ENTRIES) break;

            uint64_t key = T->getKeys()[i];
            uint8_t move = T->get(key);

            if (move != 0) 
            {
                // Write lower 7 bytes of key
                for (int j = 0; j < 7; ++j) 
                {
                    uint8_t byte = (key >> (8 * j)) & 0xFF;
                    ofs.write(reinterpret_cast<const char*>(&byte), 1);
                }

                ofs.write(reinterpret_cast<const char*>(&move), 1);
                ++count;
            }
        }

        ofs.close();
        std::cout << "Saved " << count << " positions to " << filename << "\n";
    }

    void load(const std::string& filename) 
    {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) 
        {
            std::cerr << "Error: Can't open file " << filename << "\n";
            return;
        }

        uint8_t buf[8];  // 7 bytes key + 1 byte move
        size_t loaded = 0;
        while (ifs.read(reinterpret_cast<char*>(buf), 8)) 
        {
            uint64_t key = 0;
            for (int i = 0; i < 7; ++i) {
                key |= (uint64_t(buf[i]) << (8 * i));
            }

            uint8_t move = buf[7];
            T->put(key, move);
            ++loaded;
        }

        ifs.close();
        std::cout << "Loaded " << loaded << " positions from " << filename << "\n";
    }

    uint8_t getBestMove(uint64_t key) const 
    {
        uint8_t move = T->get(key);
        return (move != 0) ? move : NO_MOVE;
    }

    bool has(uint64_t key) const 
    {
        return T->get(key) != 0;
    }
};
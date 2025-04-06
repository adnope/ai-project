#ifndef OPENING_BOOK_HPP
#define OPENING_BOOK_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include "Position.hpp"
#include "TranspositionTable.hpp"

class OpeningBook {
public:
    static constexpr uint8_t NO_MOVE = 255;

    int width, height, depth;
    TranspositionTable* T;

    OpeningBook(int width, int height, int maxDepth, TranspositionTable* table)
        : width(width), height(height), depth(maxDepth), T(table) {}

    void addPosition(uint64_t key, uint8_t bestMove) {
        if (bestMove < Position::WIDTH) {
            T->put(key, bestMove);
        }
    }

    void save(const std::string& filename) const {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            std::cout << "Error: Can't open file " << filename << "\n";
            return;
        }
    
        for (unsigned int i = 0; i < T->getSize(); ++i) {
            uint64_t key = T->getKeys()[i];
            uint8_t move = T->get(key);
    
            if (move != 0 && move < Position::WIDTH) {
                // Ghi 7 byte thấp của key
                for (int j = 0; j < 7; ++j) {
                    uint8_t byte = (key >> (8 * j)) & 0xFF;
                    ofs.write(reinterpret_cast<const char*>(&byte), 1);
                }
    
                ofs.write(reinterpret_cast<const char*>(&move), 1);
            }
        }
    
        ofs.close();
    }

    void load(const std::string& filename) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) {
            std::cout << "Error: Can't open file " << filename << "\n";
            return;
        }
    
        // if (!T) {
        //     T = new TranspositionTable(1 << 27);
        // }
    
        uint8_t buf[8];  // 7 byte key + 1 byte move
        while (ifs.read(reinterpret_cast<char*>(buf), 8)) {
            uint64_t key = 0;
            for (int i = 0; i < 7; ++i) {
                key |= (uint64_t(buf[i]) << (8 * i));
            }
    
            uint8_t move = buf[7];
            if (move <= Position::WIDTH) {
                T->put(key, move);
            }
        }
    
        ifs.close();
        std::cout<<"Load success\n";
    }
    

    uint8_t getBestMove(uint64_t key) const {
        uint8_t move = T->get(key);
        return ((int) move != 0) ? move : NO_MOVE;
    }

    bool has(uint64_t key) const {
        return T->get(key) != 0;
    }
};

#endif

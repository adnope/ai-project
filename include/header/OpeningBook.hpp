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

    void save(const std::string &filename) const
    {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs)
        {
            std::cerr << "Error: Can't open file " << filename << "\n";
            return;
        }

        size_t count = 0;
        for (unsigned int i = 0; i < T->GetSize(); ++i)
        {
            uint64_t key = T->GetKeys()[i];
            uint8_t move = T->Get(key);

            if (move != 0)
            {
                // Write lower 7 bytes of key
                for (int j = 0; j < 7; ++j)
                {
                    uint8_t byte = (key >> (8 * j)) & 0xFF;
                    ofs.write(reinterpret_cast<const char *>(&byte), 1);
                }
                ofs.write(reinterpret_cast<const char *>(&move), 1);
                ++count;
            }
        }

        ofs.close();
        std::cout << "Saved " << count << " positions to " << filename << "\n";
    }

    void load(const std::string &filename)
    {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs)
        {
            std::cerr << "Error: Can't open file " << filename << "\n";
            return;
        }

        uint8_t buf[8]; // 8 bytes key + 1 byte move
        size_t loaded = 0;
        while (ifs.read(reinterpret_cast<char *>(buf), 8))
        {
            uint64_t key = 0;
            for (int i = 0; i < 7; ++i)
            {
                key |= (uint64_t(buf[i]) << (8 * i));
            }
            uint8_t move = buf[7];
            T->Put(key, move);
            ++loaded;
        }

        ifs.close();
        std::cout << "Loaded " << loaded << " positions from " << filename << "\n";
        std::cout << "Collisions: " << T->collisions << "\n";
    }
};
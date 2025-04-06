#pragma once

#include <vector>
#include <cstring>
#include <cassert>
#include <cstdint>

/**
 * Transposition Table is a simple hash map with fixed storage size.
 * In case of collision we keep the last entry and overide the previous one.
 *
 * We use 56-bit keys and 8-bit non-null values
 */
class TranspositionTable
{
private:
	struct Entry // Size of an entry is 64 bits (8 bytes)
	{
		uint64_t key : 56; // use 56-bit keys
		uint8_t val;	   // use 8-bit values
	};

	std::vector<Entry> T;

	unsigned int index(uint64_t key) const
	{
		return key % T.size();
	}

public:
	TranspositionTable(unsigned int size) : T(size)
	{
		assert(size > 0);
	}

	/*
	 * Empty the Transition Table.
	 */
	void reset()
	{ // fill everything with 0, because 0 value means missing data
		memset(&T[0], 0, T.size() * sizeof(Entry));
	}

	/**
	 * Store a value for a given key
	 * @param key: 56-bit key
	 * @param value: non-null 8-bit value. null (0) value are used to encode missing data.
	 */
	void put(uint64_t key, uint8_t val)
	{
		assert(key < (1LL << 56));
		unsigned int i = index(key); // compute the index position
		T[i].key = key;				 // and overide any existing value.
		T[i].val = val;
	}

	/**
	 * Get the value of a key
	 * @param key
	 * @return 8-bit value associated with the key if present, 0 otherwise.
	 */
	uint8_t get(uint64_t key) const
	{
		assert(key < (1LL << 56));
		unsigned int i = index(key); // compute the index position
		if (T[i].key == key)
			return T[i].val; // and return value if key matches
		else
			return 0; // or 0 if missing entry
	}

    using partial_key_t = uint64_t;
    using value_t = uint8_t;

    const uint64_t* getKeys() const {
        return reinterpret_cast<const uint64_t*>(&T[0]);
    }

    const uint8_t* getValues() const {
        return reinterpret_cast<const uint8_t*>(&T[0].val);
    }

    size_t getSize() const {
        return T.size();
    }

    int getKeySize() const {
        return sizeof(partial_key_t);
    }

    int getValueSize() const {
        return sizeof(value_t);
    }

    uint64_t encodeMoves(const std::string& moves) const
        {
            uint64_t key = 0;
            for (char c : moves)
            {
                int move = c - '0'; // Chuyển ký tự thành số (0-6)
                key = (key << 3) | (move & 0x7); // Dịch trái 3 bit và OR vào
            }
            return key;
        }
};
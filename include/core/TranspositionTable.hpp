#pragma once

#include <vector>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <unordered_map>

class TranspositionTable
{
private:
	struct Entry
	{
		uint64_t key;
		uint8_t val;
	};

	std::unordered_map<uint64_t, uint8_t> opening_table;
	std::vector<Entry> memoi_table;

	unsigned int index(const uint64_t key) const
	{
		return key % memoi_table.size();
	}

public:
	int collisions = 0;

	TranspositionTable(size_t size) : memoi_table(size)
	{
		assert(size > 0);
		opening_table.reserve(10e6);
	}

	void Reset()
	{
		memset(&memoi_table[0], 0, memoi_table.size() * sizeof(Entry));
	}

	void Put(const uint64_t key, const uint8_t val)
	{
		unsigned int i = index(key);
		memoi_table[i].key = key;
		memoi_table[i].val = val;
	}

	uint8_t Get(const uint64_t key) const
	{
		if (opening_table.find(key) != opening_table.end())
		{
			return opening_table.at(key);
		}
		unsigned int i = index(key);
		return memoi_table[i].val;
	}

	void PutOpeningMove(const uint64_t key, const uint8_t score)
	{
		opening_table.emplace(key, score);
	}

	size_t GetMemoiTableSize() const
	{
		return memoi_table.size();
	}

	size_t GetOpeningTableSize() const
	{
		return opening_table.size();
	}
};

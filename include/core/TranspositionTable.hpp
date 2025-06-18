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

	size_t entries_count = 0;
	size_t collisions = 0;

public:

	TranspositionTable(size_t size) : memoi_table(size)
	{
		assert(size > 0);
		opening_table.reserve(10e6);
	}

	void Reset()
	{
		memset(&memoi_table[0], 0, memoi_table.size() * sizeof(Entry));
		entries_count = 0;
		collisions = 0;
	}

	void Put(uint64_t key, uint8_t val)
	{
		if (entries_count >= memoi_table.size() / 2) Reset();
		unsigned int i = index(key);
		while (memoi_table[i].key != 0 && memoi_table[i].key != key)
		{
			i = (i + 1) % memoi_table.size();
			collisions++;
		}
		if (memoi_table[i].key == 0)
		{
			entries_count++;
		}
		memoi_table[i].val = val;
		memoi_table[i].key = key;
	}

	uint8_t Get(uint64_t key) const
	{
		if (opening_table.find(key) != opening_table.end())
		{
			return opening_table.at(key);
		}
		unsigned int i = index(key);
		while (memoi_table[i].key != 0)
		{
			if (memoi_table[i].key == key)
				return memoi_table[i].val;
			i = (i + 1) % memoi_table.size();
		}
		return 0;
	}

	void PutOpeningMove(const uint64_t key, const uint8_t score)
	{
		opening_table.emplace(key, score);
	}

	size_t GetMemoiEntriesCount() const
	{
		return entries_count;
	}

	size_t GetNumOfCollisions() const
	{
		return collisions;
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
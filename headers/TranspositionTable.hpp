#pragma once

#include <vector>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <utility>

class TranspositionTable
{
private:
	std::unordered_map<uint64_t, uint8_t> T;

public:
	TranspositionTable(unsigned int size)
	{
		assert(size > 0);
		T.reserve(size);
	}

	uint64_t collisions = 0;

	void Reset()
	{
		T.clear();
	}

	void Put(uint64_t key, uint8_t val)
	{
		if (T.find(key) != T.end())
		{
			collisions++;
			return;
		}
		T.emplace(key, val);
	}

	uint8_t Get(uint64_t key) const
	{
		if (auto entry = T.find(key); entry != T.end())
		{
			return entry->second;
		}
		else
			return 0;
	}

	size_t GetSize() const
	{
		return T.size();
	}
};
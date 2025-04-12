#pragma once

#include <vector>
#include <cstring>
#include <cassert>
#include <cstdint>

class TranspositionTable
{
private:
	struct Entry
	{
		uint64_t key : 56;
		uint8_t val;
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

	void Reset()
	{
		memset(&T[0], 0, T.size() * sizeof(Entry));
	}

	void Put(uint64_t key, uint8_t val)
	{
		assert(key < (1LL << 56));
		unsigned int i = index(key);
		T[i].key = key;
		T[i].val = val;
	}

	uint8_t Get(uint64_t key) const
	{
		assert(key < (1LL << 56));
		unsigned int i = index(key);
		if (T[i].key == key)
			return T[i].val;
		else
			return 0;
	}

	using partial_key_t = uint64_t;
	using value_t = uint8_t;

	const uint64_t *GetKeys() const
	{
		return reinterpret_cast<const uint64_t *>(&T[0]);
	}

	const uint8_t *GetValues() const
	{
		return reinterpret_cast<const uint8_t *>(&T[0].val);
	}

	size_t GetSize() const
	{
		return T.size();
	}

	int GetKeySize() const
	{
		return sizeof(partial_key_t);
	}

	int GetValueSize() const
	{
		return sizeof(value_t);
	}
};
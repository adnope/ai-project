#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <cstring>
#include <cassert>
#include <cstdint>

/*
 * util functions to compute next prime at compile time
 */
constexpr uint64_t med(uint64_t min, uint64_t max)
{
    return (min + max) / 2;
}

constexpr bool has_factor(uint64_t n, uint64_t min, uint64_t max)
{
    return min * min > n ? false : // do not search for factor above sqrt(n)
               min + 1 >= max ? n % min == 0
                              : has_factor(n, min, med(min, max)) || has_factor(n, med(min, max), max);
}

// return next prime number greater or equal to n.
// n must be >= 2
constexpr uint64_t next_prime(uint64_t n)
{
    return has_factor(n, 2, n) ? next_prime(n + 1) : n;
}

template <unsigned int key_size, unsigned int value_size, unsigned int log_size>
class TranspositionTable
{
private:
    static_assert(key_size <= 64, "key_size is too large");
    static_assert(value_size <= 64, "value_size is too large");
    static_assert(log_size <= 64, "log_size is too large");

    template <int S>
    using uint_t =
        typename std::conditional<S <= 8, uint_least8_t,
                                  typename std::conditional<S <= 16, uint_least16_t,
                                                            typename std::conditional<S <= 32, uint_least32_t, uint_least64_t>::type>::type>::type;

    typedef uint_t<key_size - log_size> key_t; // Stores compressed key
    typedef uint_t<value_size> value_t;        // Stores value

    static const size_t size = next_prime(1 << log_size); // size of the transition table. Have to be odd to be prime with 2^sizeof(key_t)

    key_t *K;   // Array to store keys;
    value_t *V; // Array to store values;

    size_t index(uint64_t key) const
    {
        return key % size; // size is a static int of the form 2^n+1, compiler is able to optimize a little modulus
    }

public:
    TranspositionTable()
    {
        K = new key_t[size];
        V = new value_t[size];
        reset();
    }

    ~TranspositionTable()
    {
        delete[] K;
        delete[] V;
    }

    void reset()
    {
        memset(K, 0, size * sizeof(key_t));
        memset(V, 0, size * sizeof(value_t));
    }

    void put(uint64_t key, value_t value)
    {
        assert(key >> key_size == 0);
        assert(value >> value_size == 0);
        size_t pos = index(key);
        K[pos] = key;
        V[pos] = value;
    }

    value_t get(uint64_t key) const {
        assert(key >> key_size == 0);
        size_t pos = index(key);
        if(K[pos] == (key_t)key) return V[pos];
        else return 0;
    }
};

#endif
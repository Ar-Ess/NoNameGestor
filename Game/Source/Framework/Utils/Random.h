#pragma once

#include <cstdint>

class Random
{
    friend class App;

public:

    static uint64_t UInt(uint64_t min = 0, uint64_t max = UINT64_MAX);

    static int64_t Int(int64_t min = INT64_MIN, int64_t max = INT64_MAX);

    static double Float(double min = 0.0, double max = 1.0);

    static bool Bool();

private:

    Random() = delete;
    ~Random() = delete;
    Random(const Random& ) = delete;
    Random(Random&&) = delete;
    Random& operator=(const Random&) = delete;
    Random& operator=(Random&&) = delete;

private:

    static void Seed(uint64_t seed);
    static uint64_t Next(uint64_t& state);
    static uint64_t Range(uint64_t x, uint64_t min, uint64_t max);

private:

    static uint64_t seed;

};

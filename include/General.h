#pragma once
#include <precomp.h>

static uint32_t g_seed = 1;
uint32_t rand_xorshift(uint32_t seed);
float randf(float range = 1.0f);

template<typename T>
struct RandGen {
    T operator() () const;
};


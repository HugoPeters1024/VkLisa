#include <General.h>

uint32_t rand_xorshift(uint32_t seed) {
    // Xorshift algorithm from George Marsaglia's paper
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed;
}

float randf() {
    g_seed = rand_xorshift(g_seed);
    uint32_t ret = g_seed;
    // Faster on cuda probably
    //return g_seed * 2.3283064365387e-10f;

    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    ret &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    ret |= ieeeOne;                          // Add fractional part to 1.0

    float  f = reinterpret_cast<float&>(ret);       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}


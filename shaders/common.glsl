uint rand_xorshift(in uint seed) {
    // Xorshift algorithm from George Marsaglia's paper
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed;
}

uint g_seed;

void initRand(in uint seed) {
    g_seed = 23 * rand_xorshift(seed + rand_xorshift(17 * gl_GlobalInvocationID.x));
}

uint randu() {
    g_seed = rand_xorshift(g_seed);
    return g_seed;
}
float randf() {
    // Faster on GPU probably
    return randu() * 2.3283064365387e-10f;
}

struct Vertex {
    vec4 pos;
    vec4 color;
};

Vertex randVertex() {
    Vertex ret;
    ret.pos.x = randf();
    ret.pos.y = randf();
    ret.pos.r = randf();
    ret.pos.g = randf();
    ret.pos.b = randf();
    ret.pos.a = randf();
    return ret;
}

uint rand_xorshift(in uint seed) {
    // Xorshift algorithm from George Marsaglia's paper
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed;
}

uint g_seed;

void initRand(in uint seed, in uint id) {
    g_seed = 5 + 23 * rand_xorshift(666 + seed + rand_xorshift(17 * id));
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
    ret.color.r = randf();
    ret.color.g = randf();
    ret.color.b = randf();
    ret.color.a = randf() * 0.2f;
    return ret;
}

float brightness(vec3 col) {
    return dot(vec3(0.299, 0.587, 0.114), col);
}

#pragma once
#include <precomp.h>
#include <Comp.h>
#include <BufferTools.h>

struct EvolveInfo {
    Buffer* vertexBuffers[2];
    Buffer* parentBuffer;
};

struct Evolve {
    CompPipeline pipeline;
    VkDescriptorSet descriptorSets[2];
};

struct EvolveArgs {
    uint32_t nrVertices;
    uint32_t nrTrianglesPerInstance;
    uint32_t seed;
};

Evolve evolveCreate(Ctx& ctx, EvolveInfo& info);
void evolveDestroy(Ctx& ctx, Evolve& evolve);
void evolveRecord(Ctx& ctx, Evolve& evolve, EvolveArgs& args);





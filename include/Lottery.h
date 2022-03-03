#pragma once
#include <precomp.h>
#include <Comp.h>
#include <BufferTools.h>

struct LotteryInfo {
    Buffer* scoreBuffer;
    Buffer* parentBuffer;
};

struct Lottery {
    CompPipeline pipeline;
    VkDescriptorSet descriptorSet;
};

struct LotteryArgs {
    uint32_t nrInstances;
    uint32_t seed;
};

Lottery lotteryCreate(Ctx& ctx, LotteryInfo& info);
void lotteryDestroy(Ctx& ctx, Lottery& evolve);
void lotteryRecord(Ctx& ctx, Lottery& evolve, LotteryArgs& args);





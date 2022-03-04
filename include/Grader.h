#pragma once
#include <precomp.h>
#include <Comp.h>
#include <BufferTools.h>

struct GraderInfo {
    Image* gridImage;
    Buffer* scoreBuffer;
};

struct Grader {
    GraderInfo info;
    CompPipeline pipeline;
    VkDescriptorSet descriptorSet;
};

struct GraderArgs {
    uint32_t nrInstancesWidth;
    uint32_t nrInstancesHeight;
    uint32_t instanceWidth;
    uint32_t instanceHeight;
};

Grader graderCreate(Ctx& ctx, GraderInfo& info);
void graderDestroy(Ctx& ctx, Grader& grader);
void graderRecord(Ctx& ctx, Grader& grader, GraderArgs& args);





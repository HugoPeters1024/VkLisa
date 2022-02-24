#pragma once
#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>

struct RastPipelineInfo {
    const char* vsPath;
    const char* fsPath;
    RenderPass* renderPass;
};

struct RastPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

RastPipeline rastPipelineCreate(const Ctx&, const RastPipelineInfo&);
void rastPipelineDestroy(const Ctx&, RastPipeline&);


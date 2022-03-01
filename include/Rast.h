#pragma once
#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Primitives.h>

struct RastPipelineInfo {
    const char* vsPath;
    const char* fsPath;
    RenderPass* renderPass;
    VertexDescription* vertexDescription;
    std::vector<VkPushConstantRange> pushConstantRanges;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
};

struct RastPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

RastPipeline rastPipelineCreate(const Ctx&, const RastPipelineInfo&);
void rastPipelineDestroy(const Ctx&, RastPipeline&);


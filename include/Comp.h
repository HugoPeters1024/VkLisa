#pragma once
#include <precomp.h>
#include <Ctx.h>

typedef std::unordered_map<uint32_t, void*> CompResourceBindings;


struct CompInfo {
    const char* compShaderPath;
    std::unordered_map<uint32_t, VkDescriptorType> bindingDescription;
    VkPushConstantRange* pushConstantRange;
};

struct CompPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorType> bindingDescription;
};

CompPipeline compCreate(const Ctx& ctx, const CompInfo&);
void compDestroy(const Ctx& ctx, const CompPipeline& comp);
VkDescriptorSet compCreateDescriptorSet(const Ctx& ctx, const CompPipeline& comp, const CompResourceBindings& bindings);

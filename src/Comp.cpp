#include <Comp.h>

CompPipeline compCreate(const Ctx& ctx, const CompInfo& info) {
    CompPipeline ret{};
    ret.bindingDescription = info.bindingDescription;

    auto bufferBinding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0);
    auto resultBinding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
    std::vector<VkDescriptorSetLayoutBinding> bindings { bufferBinding, resultBinding };
    auto descriptorInfo = vks::initializers::descriptorSetLayoutCreateInfo(bindings);

    vkCheck(vkCreateDescriptorSetLayout(ctx.device, &descriptorInfo, nullptr, &ret.descriptorSetLayout));

    auto layoutInfo = vks::initializers::pipelineLayoutCreateInfo(&ret.descriptorSetLayout);

    if (info.pushConstantRange) {
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = info.pushConstantRange;
    }

    vkCheck(vkCreatePipelineLayout(ctx.device, &layoutInfo, nullptr, &ret.pipelineLayout));

    auto shader = vks::tools::loadShader(info.compShaderPath, ctx.device);

    auto pipelineInfo = vks::initializers::computePipelineCreateInfo(ret.pipelineLayout);
    pipelineInfo.stage = vks::initializers::pipelineShaderStageCreateInfo(shader, VK_SHADER_STAGE_COMPUTE_BIT);
    vkCheck(vkCreateComputePipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline));

    vkDestroyShaderModule(ctx.device, shader, nullptr);
    return ret;
}

void compDestroy(const Ctx& ctx, const CompPipeline& comp) {
    vkDestroyDescriptorSetLayout(ctx.device, comp.descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(ctx.device, comp.pipelineLayout, nullptr);
    vkDestroyPipeline(ctx.device, comp.pipeline, nullptr);
}

VkDescriptorSet compCreateDescriptorSet(const Ctx& ctx, const CompPipeline& comp, const CompResourceBindings& bindings) {
    logger::debug("Creating descriptor set for compute shader");
    VkDescriptorSet ret;
    auto allocInfo = vks::initializers::descriptorSetAllocateInfo(ctx.descriptorPool, &comp.descriptorSetLayout, 1);
    vkCheck(vkAllocateDescriptorSets(ctx.device, &allocInfo, &ret));

    for(const auto& binding : bindings) {
        auto descr = comp.bindingDescription.at(binding.first);
        if (descr == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            auto buffer = reinterpret_cast<VkBuffer>(binding.second);
            auto bufferInfo = vks::initializers::descriptorBufferInfo(buffer);
            auto writeInfo = vks::initializers::writeDescriptorSet(ret, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding.first, &bufferInfo);
            vkUpdateDescriptorSets(ctx.device, 1, &writeInfo, 0, nullptr);
        }
    }

    return ret;
}

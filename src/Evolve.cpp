#include <Evolve.h>

Evolve evolveCreate(Ctx& ctx, EvolveInfo& info) {
    Evolve ret{};

    auto pushConstant = vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(EvolveArgs), 0);
    CompInfo compInfo {
        .compShaderPath = "./shaders_bin/evolve.comp.spv",
        .bindingDescription = {
            { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        },
        .pushConstantRange = &pushConstant,
    };
    ret.pipeline = compCreate(ctx, compInfo);

    CompResourceBindings bindings0 {
        { 0, info.vertexBuffers[0]->buffer },
        { 1, info.vertexBuffers[1]->buffer },
        { 2, info.parentBuffer->buffer },
    };
    ret.descriptorSets[0] = compCreateDescriptorSet(ctx, ret.pipeline, bindings0);

    CompResourceBindings bindings1 {
        { 0, info.vertexBuffers[1]->buffer },
        { 1, info.vertexBuffers[0]->buffer },
        { 2, info.parentBuffer->buffer },
    };
    ret.descriptorSets[1] = compCreateDescriptorSet(ctx, ret.pipeline, bindings1);

    return ret;
}

void evolveDestroy(Ctx& ctx, Evolve& evolve) {
    compDestroy(ctx, evolve.pipeline);
}

void evolveRecord(Ctx& ctx, Evolve& evolve, EvolveArgs& args) {
    auto& cmdBuffer = ctx.frameCtx.cmdBuffer;
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, evolve.pipeline.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, evolve.pipeline.pipelineLayout, 0, 1, &evolve.descriptorSets[ctx.frameCtx.frameIdx%2], 0, nullptr);
    vkCmdPushConstants(cmdBuffer, evolve.pipeline.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(EvolveArgs), &args);
    vkCmdDispatch(cmdBuffer, args.nrVertices/256+1, 1, 1);
    auto barrier = vks::initializers::memoryBarrier();
    barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 0, nullptr);
}

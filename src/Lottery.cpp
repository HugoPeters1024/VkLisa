#include <Lottery.h>

Lottery lotteryCreate(Ctx& ctx, LotteryInfo& info) {
    Lottery ret{};

    auto pushConstant = vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(LotteryArgs), 0);
    CompInfo compInfo {
        .compShaderPath = "./shaders_bin/lottery.comp.spv",
        .bindingDescription = {
            { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        },
        .pushConstantRange = &pushConstant,
    };
    ret.pipeline = compCreate(ctx, compInfo);

    CompResourceBindings bindings {
        { 0, info.scoreBuffer->buffer },
        { 1, info.parentBuffer->buffer },
    };
    ret.descriptorSet = compCreateDescriptorSet(ctx, ret.pipeline, bindings);

    return ret;
}

void lotteryDestroy(Ctx& ctx, Lottery& lottery) {
    compDestroy(ctx, lottery.pipeline);
}

void lotteryRecord(Ctx& ctx, Lottery& lottery, LotteryArgs& args) {
    assert(args.nrInstances <= 1024 && "Instances must be handled in the same workgroup");
    auto& cmdBuffer = ctx.frameCtx.cmdBuffer;
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lottery.pipeline.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, lottery.pipeline.pipelineLayout, 0, 1, &lottery.descriptorSet, 0, nullptr);
    vkCmdPushConstants(cmdBuffer, lottery.pipeline.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LotteryArgs), &args);
    vkCmdDispatch(cmdBuffer, 1, 1, 1);
    auto barrier = vks::initializers::memoryBarrier();
    barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 0, nullptr);
}

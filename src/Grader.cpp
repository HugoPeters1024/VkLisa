#include <Grader.h>

Grader graderCreate(Ctx& ctx, GraderInfo& info) {
    Grader ret{};
    ret.info = info;

    assert(info.gridImage->width % 32 == 0);
    assert(info.gridImage->height % 32 == 0);

    auto pushConstant = vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(GraderArgs), 0);
    CompInfo compInfo {
        .compShaderPath = "./shaders_bin/grader.comp.spv",
        .bindingDescription = {
            { 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
            { 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        },
        .pushConstantRange = &pushConstant,
    };
    ret.pipeline = compCreate(ctx, compInfo);

    CompResourceBindings bindings {
        { 0, info.gridImage->view },
        { 1, info.goal->view },
        { 2, info.scoreBuffer->buffer },
    };
    ret.descriptorSet = compCreateDescriptorSet(ctx, ret.pipeline, bindings);
    return ret;
}

void graderDestroy(Ctx& ctx, Grader& grader) {
    compDestroy(ctx, grader.pipeline);
}

void graderRecord(Ctx& ctx, Grader& grader, GraderArgs& args) {
    assert(args.instanceWidth % 32 == 0);

    auto& cmdBuffer = ctx.frameCtx.cmdBuffer;
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, grader.pipeline.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, grader.pipeline.pipelineLayout, 0, 1, &grader.descriptorSet, 0, nullptr);
    vkCmdPushConstants(cmdBuffer, grader.pipeline.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GraderArgs), &args);
    uint gridWidth = args.instanceWidth * args.nrInstancesWidth;
    uint gridHeight = args.instanceHeight * args.nrInstancesHeight;
    vkCmdDispatch(cmdBuffer, gridWidth/32, gridHeight/32, 1);

    auto barrier = vks::initializers::memoryBarrier();
    barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    auto imageBarrier = vks::initializers::imageMemoryBarrier(grader.info.gridImage->image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 1, &imageBarrier);

}

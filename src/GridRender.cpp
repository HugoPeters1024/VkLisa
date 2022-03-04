#include <GridRender.h>

GridRender gridRenderCreate(Ctx& ctx, GridRenderInfo& info) {
    GridRender gridRender{ .info = info };


    RenderPassInfo renderPassInfo{
        .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        .images = {info.target},
    };
    gridRender.renderPass = renderPassCreate(ctx, renderPassInfo);

    auto vertexDescription = Vertex::getVertexDescription();
    RastPipelineInfo rastInfo {
        .vsPath = "./shaders_bin/grid.vert.spv",
        .fsPath = "./shaders_bin/grid.frag.spv",
        .renderPass = &gridRender.renderPass,
        .vertexDescription = &vertexDescription,
        .pushConstantRanges = {
            vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GridRenderArgs), 0),
        },
    };
    gridRender.pipeline = rastPipelineCreate(ctx, rastInfo);


    return gridRender;
}

void gridRenderDestroy(Ctx& ctx, GridRender& gridRender) {
    rastPipelineDestroy(ctx, gridRender.pipeline);
    renderPassDestroy(ctx, gridRender.renderPass);
}

void grindRenderRecord(Ctx& ctx, GridRender& gridRender, GridRenderArgs& push) {
    VkCommandBuffer cmdBuffer = ctx.frameCtx.cmdBuffer;

    VkClearValue clearColor { .color = {0.0f, 0.0f, 0.0f, 0.0f}, };
    auto renderPassInfo = vks::initializers::renderPassBeginInfo(
            gridRender.renderPass.renderPass,
            gridRender.renderPass.framebuffers[0]);
    renderPassInfo.renderArea.extent = {gridRender.info.target.width, gridRender.info.target.height};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkDeviceSize offset = 0;
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gridRender.pipeline.pipeline);
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &gridRender.info.buffers[ctx.frameCtx.frameIdx%2]->buffer, &offset);
    vkCmdPushConstants(cmdBuffer, gridRender.pipeline.pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GridRenderArgs), &push);
    vkCmdDraw(cmdBuffer, 3 * push.nrTriangles, 1, 0, 0);
    vkCmdEndRenderPass(cmdBuffer);
}

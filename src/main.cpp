#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = ctxCreate();

    RenderPassInfo renderPassInfo{};
    auto renderPass = renderPassCreate(ctx, renderPassInfo);

    RastPipelineInfo rastInfo {
        .vsPath = "./shaders_bin/quad.vert.spv",
        .fsPath = "./shaders_bin/quad.frag.spv",
        .renderPass = &renderPass,
    };

    auto pipeline = rastPipelineCreate(ctx, rastInfo);

    auto cmdBuffer = ctxAllocCmdBuffer(ctx);
    while (!ctxWindowShouldClose(ctx)) {
        auto frameCtx = ctxBeginFrame(ctx);
        vkCheck(vkResetCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        auto beginInfo = vks::initializers::commandBufferBeginInfo();
        vkCheck(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

        VkClearValue clearColor { .color = {0.0f, 0.0f, 0.0f, 1.0f}, };
        auto renderPassInfo = vks::initializers::renderPassBeginInfo(renderPass.renderPass, renderPass.framebuffers[frameCtx.imageIdx]);
        renderPassInfo.renderArea.extent = {ctx.window.width,ctx.window.height};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmdBuffer);
        vkCheck(vkEndCommandBuffer(cmdBuffer));

        ctxEndFrame(ctx, cmdBuffer);
    }

    ctxFinish(ctx);
    rastPipelineDestroy(ctx, pipeline);
    renderPassDestroy(ctx, renderPass);
    ctxDestroy(ctx);
    return 0;
}

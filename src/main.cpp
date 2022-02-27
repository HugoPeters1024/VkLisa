#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <BufferTools.h>

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = ctxCreate();

    RenderPassInfo renderPassInfo{};
    auto renderPass = renderPassCreate(ctx, renderPassInfo);

    auto vertexDescription = Vertex2D4C::getVertexDescription();
    RastPipelineInfo rastInfo {
        .vsPath = "./shaders_bin/quad.vert.spv",
        .fsPath = "./shaders_bin/quad.frag.spv",
        .renderPass = &renderPass,
        .vertexDescription = &vertexDescription
    };
    auto pipeline = rastPipelineCreate(ctx, rastInfo);

    std::vector<Vertex2D4C> vertexData(3*100);
    for(auto i=0; i<vertexData.size(); i++) {
        vertexData[i] = RandGen<Vertex2D4C>()();
    }

    auto vertexBuffer = buffertools::create_buffer_D_data(ctx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexData.size() * sizeof(Vertex2D4C), vertexData.data());

    auto cmdBuffer = ctxAllocCmdBuffer(ctx);
    double ping;
    uint32_t frameCounter = 0;
    while (!ctxWindowShouldClose(ctx)) {
        ping = glfwGetTime();

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
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBuffer.buffer, &offset);
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
        vkCmdDraw(cmdBuffer, vertexData.size(), 1, 0, 0);
        vkCmdEndRenderPass(cmdBuffer);
        vkCheck(vkEndCommandBuffer(cmdBuffer));

        ctxEndFrame(ctx, cmdBuffer);

        if (frameCounter % 100 == 0) {
            double fps = 1.0f / (glfwGetTime() - ping);
            logger::info("FPS: {}", fps);
        }
        
        frameCounter++;
    }

    ctxFinish(ctx);
    buffertools::destroyBuffer(ctx, vertexBuffer);
    rastPipelineDestroy(ctx, pipeline);
    renderPassDestroy(ctx, renderPass);
    ctxDestroy(ctx);
    return 0;
}

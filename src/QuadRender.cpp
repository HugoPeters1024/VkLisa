#include <QuadRender.h>

QuadRender quadRenderCreate(Ctx& ctx, QuadRenderInfo& info) {
    QuadRender quadRender{};

    RenderPassInfo renderPassInfo {
        .beforeLayout = info.beforeLayout,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .images = ctx.window.swapchainImages,
    };
    quadRender.renderPass = renderPassCreate(ctx, renderPassInfo);


    auto samplerInfo = vks::initializers::samplerCreateInfo(1.0f);
    samplerInfo.anisotropyEnable = VK_FALSE;
    vkCheck(vkCreateSampler(ctx.device, &samplerInfo, nullptr, &quadRender.sampler));

    auto textBinding = vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    auto descriptorLayoutInfo = vks::initializers::descriptorSetLayoutCreateInfo(&textBinding, 1);
    vkCheck(vkCreateDescriptorSetLayout(ctx.device, &descriptorLayoutInfo, nullptr, &quadRender.descriptorLayout));

    auto allocInfo = vks::initializers::descriptorSetAllocateInfo(ctx.descriptorPool, &quadRender.descriptorLayout, 1);
    vkCheck(vkAllocateDescriptorSets(ctx.device, &allocInfo, &quadRender.descriptorSet));

    auto imageInfo = vks::initializers::descriptorImageInfo(quadRender.sampler, info.srcImage.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    auto writeInfo = vks::initializers::writeDescriptorSet(quadRender.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &imageInfo);
    vkUpdateDescriptorSets(ctx.device, 1, &writeInfo, 0, nullptr);

    RastPipelineInfo rastInfo {
        .vsPath = "./shaders_bin/quad.vert.spv",
        .fsPath = "./shaders_bin/quad.frag.spv",
        .renderPass = &quadRender.renderPass,
        .descriptorSetLayouts = {quadRender.descriptorLayout},
    };
    quadRender.pipeline = rastPipelineCreate(ctx, rastInfo);

    return quadRender;
}

void quadRenderDestroy(Ctx& ctx, QuadRender& quadRender) {
    vkDestroySampler(ctx.device, quadRender.sampler, nullptr);
    vkDestroyDescriptorSetLayout(ctx.device, quadRender.descriptorLayout, nullptr);
    rastPipelineDestroy(ctx, quadRender.pipeline);
    renderPassDestroy(ctx, quadRender.renderPass);
}

void quadRenderRecord(Ctx& ctx, QuadRender& quadRender) {
    VkCommandBuffer cmdBuffer = ctx.frameCtx.cmdBuffer;

    VkClearValue clearColor { .color = {0.0f, 0.0f, 0.0f, 0.0f}, };
    auto renderPassInfo = vks::initializers::renderPassBeginInfo(
            quadRender.renderPass.renderPass,
            quadRender.renderPass.framebuffers[ctx.frameCtx.imageIdx]);
    renderPassInfo.renderArea.extent = {ctx.window.width,ctx.window.height};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, quadRender.pipeline.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, quadRender.pipeline.pipelineLayout, 0, 1, &quadRender.descriptorSet, 0, nullptr);
    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmdBuffer);
} 


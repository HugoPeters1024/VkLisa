#include <RenderPass.h>

RenderPass renderPassCreate(const Ctx& ctx, const RenderPassInfo& info) {
    auto colorAttachment = vks::initializers::attachmentDescription(ctx.window.format, info.finalLayout);
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    auto attachmentReference = vks::initializers::attachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference,
    };

    VkSubpassDependency dependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    RenderPass pass{};
    vkCheck(vkCreateRenderPass(ctx.device, &renderPassInfo, nullptr, &pass.renderPass));

    pass.framebuffers.resize(info.images.size());
    for(uint32_t i=0; i<pass.framebuffers.size(); i++) {
        const Image& image = info.images[i];
        VkImageView attachments[1] { image.view };

        auto framebufferInfo = vks::initializers::framebufferCreateInfo();
        framebufferInfo.renderPass = pass.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = image.width;
        framebufferInfo.height = image.height;
        framebufferInfo.layers = 1;

        vkCheck(vkCreateFramebuffer(ctx.device, &framebufferInfo, nullptr, &pass.framebuffers[i]));
    }

    return pass;
}

void renderPassDestroy(const Ctx& ctx, RenderPass& renderPass) {
    for (auto framebuffer : renderPass.framebuffers) {
        vkDestroyFramebuffer(ctx.device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(ctx.device, renderPass.renderPass, nullptr);
}

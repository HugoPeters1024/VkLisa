#include <RenderPass.h>

RenderPass renderPassCreate(const Ctx& ctx, const RenderPassInfo&) {
    auto colorAttachment = vks::initializers::attachmentDescription(ctx.window.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    auto attachmentReference = vks::initializers::attachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentReference,
    };

    VkRenderPassCreateInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    RenderPass pass{};
    vkCheck(vkCreateRenderPass(ctx.device, &renderPassInfo, nullptr, &pass.renderPass));
    return pass;
}

void renderPassDestroy(const Ctx& ctx, RenderPass& renderPass) {
    vkDestroyRenderPass(ctx.device, renderPass.renderPass, nullptr);
}

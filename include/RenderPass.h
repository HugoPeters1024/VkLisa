#pragma once
#include <precomp.h>
#include <Types.h>
#include <Ctx.h>


struct RenderPassInfo {
    VkImageLayout beforeLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout;
    std::vector<Image> images;
};

struct RenderPass {
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
};

RenderPass renderPassCreate(const Ctx& ctx, const RenderPassInfo&);
void renderPassDestroy(const Ctx& ctx, RenderPass& renderPass);

#pragma once
#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>


struct QuadRenderInfo {
    Image srcImage;
    VkImageLayout beforeLayout;
};

struct QuadRender {
    RenderPass renderPass;
    RastPipeline pipeline;
    VkDescriptorSetLayout descriptorLayout;
    VkDescriptorSet descriptorSet;
    VkSampler sampler;
};

QuadRender quadRenderCreate(Ctx& ctx, QuadRenderInfo& info);
void quadRenderDestroy(Ctx& ctx, QuadRender& render);
void quadRenderRecord(Ctx& ctx, QuadRender& quadRender);

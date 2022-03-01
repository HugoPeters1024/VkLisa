#include <ImageTools.h>

Image createImageD(Ctx& ctx, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkFormat format, VkImageLayout initialLayout) {
    Image ret{};
    ret.width = width;
    ret.height = height;
    ret.format = format;
    auto imageCreateInfo = vks::initializers::imageCreateInfo(width, height, format, usage);
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_GPU_ONLY };
    vkCheck(vmaCreateImage(ctx.allocator, &imageCreateInfo, &allocInfo, &ret.image, &ret.memory, nullptr));

    auto viewInfo = vks::initializers::imageViewCreateInfo(ret.image, format, VK_IMAGE_ASPECT_COLOR_BIT);
    vkCheck(vkCreateImageView(ctx.device, &viewInfo, nullptr, &ret.view));

    ctxSingleTimeCommand(ctx, [&](VkCommandBuffer cmdBuffer) {
        auto barrier = vks::initializers::imageMemoryBarrier(ret.image, VK_IMAGE_LAYOUT_UNDEFINED, initialLayout);
        vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        });
    return ret;
}

void destroyImage(Ctx& ctx, Image& image) {
    vkDestroyImageView(ctx.device, image.view, nullptr);
    vmaDestroyImage(ctx.allocator, image.image, image.memory);
}

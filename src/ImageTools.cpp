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

Image loadImageD(Ctx& ctx, VkImageLayout initialLayout, const char* filename) {
    int width, height, nrChannels;
    float* pixels = stbi_loadf(filename, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (!pixels) {
        logger::error("Could not load image {}", filename);
        exit(1);
    }

    assert(nrChannels == 4 && "Image loading should produce a 4 channel buffer");

    VkDeviceSize imageSize = width * height * 4 * sizeof(float);
    Buffer stagingBuffer = buffertools::createBufferH(ctx, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize);

    void* data;
    vkCheck(vmaMapMemory(ctx.allocator, stagingBuffer.memory, &data));
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(ctx.allocator, stagingBuffer.memory);
    vmaFlushAllocation(ctx.allocator, stagingBuffer.memory, 0, imageSize);

    Image dst = createImageD(ctx, width, height, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    ctxSingleTimeCommand(ctx, [&](VkCommandBuffer cmdBuffer) {
        VkBufferImageCopy copyRegion = vks::initializers::imageCopy(width, height);
        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer.buffer, dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        auto barrier = vks::initializers::imageMemoryBarrier(dst.image, VK_IMAGE_LAYOUT_UNDEFINED, initialLayout);
    vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    });


    buffertools::destroyBuffer(ctx, stagingBuffer);

    return dst;
}

void destroyImage(Ctx& ctx, Image& image) {
    vkDestroyImageView(ctx.device, image.view, nullptr);
    vmaDestroyImage(ctx.allocator, image.image, image.memory);
}

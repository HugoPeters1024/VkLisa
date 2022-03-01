#include<BufferTools.h>

namespace buffertools {

Buffer createBufferH2D(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_TO_GPU };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer createBufferH2D_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto ret = createBufferH2D(ctx, usage, size);

    void* data_dst;
    vkCheck(vmaMapMemory(ctx.allocator, ret.memory, &data_dst));
    memcpy(data_dst, data, size);
    vmaUnmapMemory(ctx.allocator, ret.memory);
    return ret;
}

Buffer createBufferD(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_GPU_ONLY };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer createBufferD_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto staging = createBufferH_Data(ctx, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, data);
    auto dst = createBufferD(ctx, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);

    ctxSingleTimeCommand(ctx, [&](VkCommandBuffer cmdBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(cmdBuffer, staging.buffer, dst.buffer, 1, &copyRegion);
    });

    buffertools::destroyBuffer(ctx, staging);
    return dst;
}

Buffer createBufferH(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer createBufferH_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto ret = createBufferH(ctx, usage, size);

    void* data_dst;
    vkCheck(vmaMapMemory(ctx.allocator, ret.memory, &data_dst));
    memcpy(data_dst, data, size);
    vmaUnmapMemory(ctx.allocator, ret.memory);
    return ret;
}

void destroyBuffer(Ctx& ctx, Buffer& buffer) {
    vmaDestroyBuffer(ctx.allocator, buffer.buffer, buffer.memory);
}

}

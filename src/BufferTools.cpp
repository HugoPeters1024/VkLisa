#include<BufferTools.h>

namespace buffertools {

Buffer create_buffer_H2D(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_TO_GPU };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer create_buffer_H2D_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto ret = create_buffer_H2D(ctx, usage, size);

    void* data_dst;
    vkCheck(vmaMapMemory(ctx.allocator, ret.memory, &data_dst));
    memcpy(data_dst, data, size);
    vmaUnmapMemory(ctx.allocator, ret.memory);
    return ret;
}

Buffer create_buffer_D(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_GPU_ONLY };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer create_buffer_D_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto staging = create_buffer_H_data(ctx, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, data);
    auto dst = create_buffer_D(ctx, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);

    ctxSingleTimeCommand(ctx, [&](VkCommandBuffer cmdBuffer) {
        VkBufferCopy copyRegion{};
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(cmdBuffer, staging.buffer, dst.buffer, 1, &copyRegion);
    });

    buffertools::destroyBuffer(ctx, staging);
    return dst;
}

Buffer create_buffer_H(Ctx& ctx, VkBufferUsageFlags usage, size_t size) {
    auto bufferInfo = vks::initializers::bufferCreateInfo(usage, static_cast<VkDeviceSize>(size));
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    Buffer ret{};
    vkCheck(vmaCreateBuffer(ctx.allocator, &bufferInfo, &allocInfo, &ret.buffer, &ret.memory, nullptr));
    return ret;
}

Buffer create_buffer_H_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data) {
    auto ret = create_buffer_H(ctx, usage, size);

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

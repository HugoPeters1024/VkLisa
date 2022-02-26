#pragma once
#include <precomp.h>
#include <Ctx.h>

struct Buffer {
    VkBuffer buffer;
    VmaAllocation memory;
};

namespace buffertools {
    Buffer create_buffer_H2D(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer create_buffer_H2D_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    Buffer create_buffer_D(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer create_buffer_D_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    Buffer create_buffer_H(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer create_buffer_H_data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    void destroyBuffer(Ctx& ctx, Buffer& buffer);
}

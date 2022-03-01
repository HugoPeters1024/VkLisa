#pragma once
#include <precomp.h>
#include <Ctx.h>

struct Buffer {
    VkBuffer buffer;
    VmaAllocation memory;
};

namespace buffertools {
    Buffer createBufferH2D(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer createBufferH2D_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    Buffer createBufferD(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer createBufferD_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    Buffer createBufferH(Ctx& ctx, VkBufferUsageFlags usage, size_t size);
    Buffer createBufferH_Data(Ctx& ctx, VkBufferUsageFlags usage, size_t size, void* data);
    void destroyBuffer(Ctx& ctx, Buffer& buffer);
}

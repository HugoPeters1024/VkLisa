#pragma once
#include <precomp.h>
#include <Types.h>
#include <Ctx.h>
#include <BufferTools.h>

Image createImageD(Ctx& ctx, uint32_t width, uint32_t height, VkImageUsageFlags usage, VkFormat format, VkImageLayout imageLayout);
Image loadImageD(Ctx& ctx, VkImageLayout initialLayout, const char* filename);
void destroyImage(Ctx& ctx, Image& image);

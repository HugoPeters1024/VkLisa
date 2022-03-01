#pragma once
#include <precomp.h>

struct Image {
    VkImage image;
    VkImageView view;
    VmaAllocation memory;
    VkFormat format;
    uint32_t width;
    uint32_t height;
};

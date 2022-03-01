#pragma once
#include <precomp.h>

struct Image {
    VkImage image;
    VkImageView view;
    uint32_t width;
    uint32_t height;
};

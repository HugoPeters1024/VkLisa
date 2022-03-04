#define VMA_IMPLEMENTATION
#include <vks/vk_mem_alloc.hpp>

#define STB_IMAGE_IMPLEMENTATION
// TODO: why does it not compile with simd
#define STBI_NO_SIMD
#include <stb_image.h>

#include <precomp.h>

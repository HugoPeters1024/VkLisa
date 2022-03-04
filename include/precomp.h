#pragma once
#include <stdio.h>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>


#include <spdlog/spdlog.h>
namespace spdlog {
    template<typename T>
    inline void crash(const T &msg)
    {
        default_logger_raw()->error(msg);
        exit(1);
    }
}
namespace logger = spdlog;


#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vks/VulkanInitializers.hpp>
#include <vks/VulkanTools.h>
#define vkCheck(f) VK_CHECK_RESULT(f)
#include <vks/vk_mem_alloc.hpp>

#include <stb_image.h>


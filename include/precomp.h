#pragma once
#include <stdio.h>
#include <array>

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

#include <vks/VulkanInitializers.hpp>
#include <vks/VulkanTools.h>
#define vkCheck(f) VK_CHECK_RESULT(f)



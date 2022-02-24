#pragma once
#include <precomp.h>

struct Ctx {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VmaAllocator allocator;
    struct {
        GLFWwindow* glfwWindow;
        VkSurfaceKHR surface;
        VkFormat format;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
        uint32_t width;
        uint32_t height;
    } window;
    struct {
        VkQueue compute;
        VkQueue graphics;
        VkQueue present;
    } queues;
};

Ctx ctxCreate();
void ctxDestroy(Ctx&);
bool ctxWindowShouldClose(Ctx&);
void ctxUpdate(Ctx&);



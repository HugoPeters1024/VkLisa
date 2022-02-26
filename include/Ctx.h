#pragma once
#include <precomp.h>

enum CtxState { 
    CTX_STATE_APP_START,
    CTX_STATE_FRAME_STARTED,
    CTX_STATE_FRAME_SUBMITTED,
    CTX_STATE_FINISHED,
};

struct FrameCtx {
    uint32_t imageIdx;
    VkImage swapchainImage;
    VkImageView swapchainView;
};

struct Ctx {
    CtxState state;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VmaAllocator allocator;
    VkCommandPool commandPool;
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
        uint32_t computeFamily;
        VkQueue graphics;
        uint32_t graphicsFamily;
        VkQueue present;
        uint32_t presentFamily;
    } queues;

    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlightFence;
    FrameCtx frameCtx;
};


Ctx ctxCreate();
void ctxDestroy(Ctx&);
bool ctxWindowShouldClose(Ctx&);
FrameCtx& ctxBeginFrame(Ctx&);
void ctxEndFrame(Ctx&, VkCommandBuffer);
VkCommandBuffer ctxAllocCmdBuffer(Ctx&);
void ctxSingleTimeCommand(Ctx& ctx, std::function<void(VkCommandBuffer)>);
void ctxFinish(Ctx&);



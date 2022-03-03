#pragma once
#include <precomp.h>
#include <Types.h>

struct CtxInfo {
    uint32_t windowWidth = 640;
    uint32_t windowHeight = 480;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
};

enum CtxState { 
    CTX_STATE_APP_START,
    CTX_STATE_FRAME_STARTED,
    CTX_STATE_FRAME_SUBMITTED,
    CTX_STATE_FINISHED,
};

struct FrameCtx {
    uint32_t imageIdx;
    uint32_t frameIdx = -1;
    Image swapchainImage;
    VkCommandBuffer cmdBuffer;
};

struct Ctx {
    CtxInfo info;
    CtxState state;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VmaAllocator allocator;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    VkCommandBuffer cmdBuffer;
    struct {
        GLFWwindow* glfwWindow;
        VkSurfaceKHR surface;
        VkFormat imageFormat;
        VkSwapchainKHR swapchain;
        std::vector<Image> swapchainImages;
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


Ctx ctxCreate(CtxInfo& info);
void ctxDestroy(Ctx&);
bool ctxWindowShouldClose(Ctx&);
FrameCtx& ctxBeginFrame(Ctx&);
void ctxEndFrame(Ctx&, VkCommandBuffer);
VkCommandBuffer ctxAllocCmdBuffer(Ctx&);
void ctxSingleTimeCommand(Ctx& ctx, std::function<void(VkCommandBuffer)>);
void ctxFinish(Ctx&);



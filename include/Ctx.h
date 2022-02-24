#include <precomp.h>

struct Ctx {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VmaAllocator allocator;
    struct {
        GLFWwindow* glfwWindow;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
    } window;
    struct {
        VkQueue compute;
        VkQueue graphics;
        VkQueue present;
    } queues;
};

Ctx ctxCreate();
void ctxDestroy(Ctx&);


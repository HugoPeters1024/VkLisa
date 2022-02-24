#include "Ctx.h"

#ifdef NDEBUG
const bool enableValidation = false;
#else
const bool enableValidation = true;
#endif

// Private helpers
struct QueueFamilies {
    uint32_t compute;
    uint32_t graphics;
    uint32_t present;
};

struct SwapchainSupport {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void _initInstance(Ctx& ctx);
void _initWindow(Ctx& ctx);
void _initSurface(Ctx&);
void _initPhysicalDevice(Ctx&);
void _initDevice(Ctx&);
void _initAllocator(Ctx&);
void _initSwapchain(Ctx&);


QueueFamilies _queryQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapchainSupport _querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR _chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
VkExtent2D _chooseSwapchainExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

void glfwErrorCallback(int error, const char* description) {
    logger::error("GLFW says: {}", description);
}


// Public
Ctx ctxCreate() {
    Ctx ctx{};
    _initWindow(ctx);
    _initInstance(ctx);
    _initSurface(ctx);
    _initPhysicalDevice(ctx);
    _initDevice(ctx);
    _initAllocator(ctx);
    _initSwapchain(ctx);
    return ctx;
}

void ctxDestroy(Ctx& ctx) {
    logger::debug("destroying ctx");
    for (auto view : ctx.window.swapchainViews) {
        vkDestroyImageView(ctx.device, view, nullptr);
    }
    vkDestroySwapchainKHR(ctx.device, ctx.window.swapchain, nullptr);
    vkDestroyDevice(ctx.device, nullptr);
    vkDestroySurfaceKHR(ctx.instance, ctx.window.surface, nullptr);
    vkDestroyInstance(ctx.instance, nullptr);
    glfwDestroyWindow(ctx.window.glfwWindow);
    glfwTerminate();
}

// Private implementation
void _initInstance(Ctx& ctx) {
    auto appInfo = vks::initializers::applicationInfo(VK_MAKE_VERSION(1,0,0));

    std::vector<const char*> validationLayers{};
    if (enableValidation) {
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }

    std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    auto createInfo = vks::initializers::instanceInfo(&appInfo, validationLayers, extensions);

    uint32_t glfwExtensionCount;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;


    vkCheck(vkCreateInstance(&createInfo, nullptr, &ctx.instance));
    logger::trace("VkInstance created");

}

void _initWindow(Ctx& ctx) {
    if (!glfwInit()) {
        logger::crash("Unable to init glfw");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwSetErrorCallback(glfwErrorCallback);
    ctx.window.glfwWindow = glfwCreateWindow(640, 480, "CVulkan", nullptr, nullptr);
    if (!ctx.window.glfwWindow) {
        logger::crash("Unable to open window");
    }
}

void _initSurface(Ctx& ctx) {
    if (glfwCreateWindowSurface(ctx.instance, ctx.window.glfwWindow, nullptr, &ctx.window.surface) != VK_SUCCESS) {
        logger::crash("Failed to create window surface");
    }
}

void _initPhysicalDevice(Ctx& ctx) {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        logger::crash("No GPUs detected in this machine");
    }

    VkPhysicalDevice devices[deviceCount];
    VkPhysicalDeviceProperties properties[deviceCount];
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, devices);

    for (uint32_t i=0; auto device : devices) {
        vkGetPhysicalDeviceProperties(device, &properties[i]);
        logger::debug("Device {}: {}", i, properties[i].deviceName);
        i++;
    }

    logger::info("Picking device 0: {}", properties[0].deviceName);
    ctx.physicalDevice = devices[0];
}


void _initDevice(Ctx& ctx) {
    auto indices = _queryQueueFamilies(ctx.physicalDevice, ctx.window.surface);

    std::set<uint32_t> families {
        indices.compute,
        indices.graphics,
        indices.present,
    };

    float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos(families.size());

    for (uint32_t i=0; auto family : families) {
        queueInfos[i++] = vks::initializers::queueCreateInfo(family, &priority);
    };

    std::vector<const char*> deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VkPhysicalDeviceFeatures deviceFeatures{};

    auto deviceInfo = vks::initializers::deviceCreateInfo(queueInfos, deviceExtensions, &deviceFeatures);

    vkCheck(vkCreateDevice(ctx.physicalDevice, &deviceInfo, nullptr, &ctx.device));
    vkGetDeviceQueue(ctx.device, indices.compute, 0, &ctx.queues.compute);
    vkGetDeviceQueue(ctx.device, indices.graphics, 0, &ctx.queues.graphics);
    vkGetDeviceQueue(ctx.device, indices.present, 0, &ctx.queues.present);

    logger::debug("Created logical device");
}

void _initAllocator(Ctx& ctx) {
    VmaAllocatorCreateInfo createInfo {
        .flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
        .physicalDevice = ctx.physicalDevice,
        .device = ctx.device,
        .instance = ctx.instance,
        .vulkanApiVersion = VK_API_VERSION_1_2,
    };
    vmaCreateAllocator(&createInfo, &ctx.allocator);
    logger::debug("created VMA allocator");
}

void _initSwapchain(Ctx& ctx) {
    auto support = _querySwapchainSupport(ctx.physicalDevice, ctx.window.surface);
    auto format = _chooseSurfaceFormat(support.formats);
    auto mode = _choosePresentMode(support.presentModes);
    auto extent = _chooseSwapchainExtent(ctx.window.glfwWindow, support.capabilities);

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ctx.window.surface,
        .minImageCount = imageCount,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    vkCheck(vkCreateSwapchainKHR(ctx.device, &createInfo, nullptr, &ctx.window.swapchain));

    // Retrieve the images
    vkGetSwapchainImagesKHR(ctx.device, ctx.window.swapchain, &imageCount, nullptr);
    ctx.window.swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(ctx.device, ctx.window.swapchain, &imageCount, ctx.window.swapchainImages.data());

    // Create image views
    ctx.window.swapchainViews.resize(imageCount);
    for (uint32_t i=0; i<imageCount; ++i) {
        auto viewInfo = vks::initializers::imageViewCreateInfo(ctx.window.swapchainImages[i], format.format, VK_IMAGE_ASPECT_COLOR_BIT);
        vkCheck(vkCreateImageView(ctx.device, &viewInfo, nullptr, &ctx.window.swapchainViews[i]));
    }

    logger::trace("created swapchain");
}

QueueFamilies _queryQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilies indices{};
    std::optional<uint32_t> compute, graphics, present;

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
    VkQueueFamilyProperties families[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families);

    for (uint32_t i=0; auto family : families) {
        if (!compute.has_value() && family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            logger::debug("compute queue family index: {}", i);
            compute = i;
        }

        if (!graphics.has_value() && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            logger::debug("graphics queue family index: {}", i);
            graphics = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (!present.has_value() && presentSupport) {
            logger::debug("present queue family index: {}", i);
            present = i;
        }

        i++;
    }

    if (!compute.has_value() || !graphics.has_value() || !present.has_value()) {
        logger::crash("Not all 3 queue families present on device");
    }

    indices.compute = compute.value();
    indices.graphics = graphics.value();
    indices.present = present.value();
    return indices;
}

SwapchainSupport _querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupport support{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    support.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, support.formats.data());

    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);
    support.presentModes.reserve(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, support.presentModes.data());

    return support;
}

VkSurfaceFormatKHR _chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D _chooseSwapchainExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

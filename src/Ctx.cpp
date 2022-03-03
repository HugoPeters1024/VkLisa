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
void _initSyncObjects(Ctx&);
void _initCommandPool(Ctx&);
void _initDescriptorPool(Ctx& ctx);


QueueFamilies _queryQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapchainSupport _querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR _chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
VkExtent2D _chooseSwapchainExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

void glfwErrorCallback(int error, const char* description) {
    logger::error("GLFW says: {}", description);
}


// Public
Ctx ctxCreate(CtxInfo& info) {
    Ctx ctx{ 
        .info = info,
        .state = CTX_STATE_APP_START, 
    };
    _initWindow(ctx);
    _initInstance(ctx);
    _initSurface(ctx);
    _initPhysicalDevice(ctx);
    _initDevice(ctx);
    _initAllocator(ctx);
    _initSwapchain(ctx);
    _initSyncObjects(ctx);
    _initCommandPool(ctx);
    _initDescriptorPool(ctx);
    return ctx;
}

void ctxDestroy(Ctx& ctx) {
    assert(ctx.state == CTX_STATE_FINISHED && "Call finish before destroying the ctx");
    logger::debug("cleaning up");
    vkDestroyDescriptorPool(ctx.device, ctx.descriptorPool, nullptr);
    vmaDestroyAllocator(ctx.allocator);
    vkDestroyCommandPool(ctx.device, ctx.commandPool, nullptr);

    vkDestroyFence(ctx.device, ctx.inFlightFence, nullptr);
    vkDestroySemaphore(ctx.device, ctx.renderFinished, nullptr);
    vkDestroySemaphore(ctx.device, ctx.imageAvailable, nullptr);
    for (auto image : ctx.window.swapchainImages) {
        vkDestroyImageView(ctx.device, image.view, nullptr);
    }
    vkDestroySwapchainKHR(ctx.device, ctx.window.swapchain, nullptr);
    vkDestroyDevice(ctx.device, nullptr);
    vkDestroySurfaceKHR(ctx.instance, ctx.window.surface, nullptr);
    vkDestroyInstance(ctx.instance, nullptr);
    glfwDestroyWindow(ctx.window.glfwWindow);
    glfwTerminate();
}

bool ctxWindowShouldClose(Ctx& ctx) {
    return glfwWindowShouldClose(ctx.window.glfwWindow);
}

FrameCtx& ctxBeginFrame(Ctx& ctx) {
    vkWaitForFences(ctx.device, 1, &ctx.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.device, 1, &ctx.inFlightFence);
    assert(ctx.state == CTX_STATE_APP_START || ctx.state == CTX_STATE_FRAME_SUBMITTED);
    ctx.state = CTX_STATE_FRAME_STARTED;
    glfwPollEvents();

    ctx.frameCtx.swapchainImage = ctx.window.swapchainImages[0];
    ctx.frameCtx.cmdBuffer = ctx.cmdBuffer;
    ctx.frameCtx.frameIdx++;

    vkAcquireNextImageKHR(ctx.device, ctx.window.swapchain, UINT64_MAX, ctx.imageAvailable, VK_NULL_HANDLE, &ctx.frameCtx.imageIdx);

    vkCheck(vkResetCommandBuffer(ctx.frameCtx.cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
    return ctx.frameCtx;
}

void ctxEndFrame(Ctx& ctx, VkCommandBuffer cmdBuffer) {
    assert(ctx.state == CTX_STATE_FRAME_STARTED);
    ctx.state = CTX_STATE_FRAME_SUBMITTED;

    auto submitInfo = vks::initializers::submitInfo(&cmdBuffer);
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ctx.imageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &ctx.renderFinished;
    vkCheck(vkQueueSubmit(ctx.queues.graphics, 1, &submitInfo, ctx.inFlightFence));

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx.renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &ctx.window.swapchain,
        .pImageIndices = &ctx.frameCtx.imageIdx,
    };

    vkCheck(vkQueuePresentKHR(ctx.queues.present, &presentInfo));
}

VkCommandBuffer ctxAllocCmdBuffer(Ctx& ctx) {
    VkCommandBuffer cmdBuffer;
    auto allocInfo = vks::initializers::commandBufferAllocateInfo(ctx.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    vkCheck(vkAllocateCommandBuffers(ctx.device, &allocInfo, &cmdBuffer));
    return cmdBuffer;
}

void ctxSingleTimeCommand(Ctx& ctx, std::function<void(VkCommandBuffer)> f) {
    auto cmdBuffer = ctxAllocCmdBuffer(ctx);
    auto beginInfo = vks::initializers::commandBufferBeginInfo();
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkCheck(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
    f(cmdBuffer);
    vkCheck(vkEndCommandBuffer(cmdBuffer));

    auto submitInfo = vks::initializers::submitInfo(&cmdBuffer);
    vkQueueSubmit(ctx.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);

    vkCheck(vkQueueWaitIdle(ctx.queues.graphics));
    vkFreeCommandBuffers(ctx.device, ctx.commandPool, 1, &cmdBuffer);
}


void ctxFinish(Ctx& ctx) {
    logger::debug("Flushing all GPU commands in preperation of shutdown");
    vkCheck(vkDeviceWaitIdle(ctx.device));
    ctx.state = CTX_STATE_FINISHED;
}

// Private implementation
void _initInstance(Ctx& ctx) {
    auto appInfo = vks::initializers::applicationInfo(VK_MAKE_VERSION(1,2,0));

    std::vector<const char*> validationLayers{};
    if (enableValidation) {
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }

    std::vector<const char*> extensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    for (auto ext : ctx.info.extensions) {
        logger::info("Enabling extension: {}", ext);
        extensions.push_back(ext);
    }

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    ctx.window.glfwWindow = glfwCreateWindow(ctx.info.windowWidth, ctx.info.windowHeight, "CVulkan", nullptr, nullptr);
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

    VkPhysicalDeviceFeatures deviceFeatures{ 
    };

    // extra features
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT enabledAtomicsFeatures {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT,
        .shaderBufferFloat32AtomicAdd = VK_TRUE,
    };


    auto deviceInfo = vks::initializers::deviceCreateInfo(queueInfos, deviceExtensions, &deviceFeatures);
    deviceInfo.pNext = &enabledAtomicsFeatures;

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
    ctx.window.imageFormat = format.format;
    ctx.window.width = extent.width;
    ctx.window.height = extent.height;

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
    VkImage images[imageCount];
    vkGetSwapchainImagesKHR(ctx.device, ctx.window.swapchain, &imageCount, images);

    // Create image views
    VkImageView imageViews[imageCount];
    for (uint32_t i=0; i<imageCount; ++i) {
        auto viewInfo = vks::initializers::imageViewCreateInfo(images[i], format.format, VK_IMAGE_ASPECT_COLOR_BIT);
        vkCheck(vkCreateImageView(ctx.device, &viewInfo, nullptr, &imageViews[i]));
    }

    // Collect in the context state
    ctx.window.swapchainImages.resize(imageCount);
    for(uint32_t i=0; i<imageCount; i++) {
        ctx.window.swapchainImages[i] = {
            .image = images[i],
            .view = imageViews[i],
            .format = ctx.window.imageFormat,
            .width = ctx.info.windowWidth,
            .height = ctx.info.windowHeight,
        };
    }

    logger::trace("created swapchain");
}

void _initSyncObjects(Ctx& ctx) {
    auto semInfo = vks::initializers::semaphoreCreateInfo();
    vkCheck(vkCreateSemaphore(ctx.device, &semInfo, nullptr, &ctx.renderFinished));
    vkCheck(vkCreateSemaphore(ctx.device, &semInfo, nullptr, &ctx.imageAvailable));

    auto fenceInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    vkCheck(vkCreateFence(ctx.device, &fenceInfo, nullptr, &ctx.inFlightFence));
}

void _initCommandPool(Ctx& ctx) {
    auto createInfo = vks::initializers::commandPoolCreateInfo(ctx.queues.graphicsFamily);
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCheck(vkCreateCommandPool(ctx.device, &createInfo, nullptr, &ctx.commandPool));

    ctx.cmdBuffer = ctxAllocCmdBuffer(ctx);
}

void _initDescriptorPool(Ctx& ctx) {
    VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 100,
            .poolSizeCount = std::size(pool_sizes),
            .pPoolSizes = pool_sizes,
    };

    vkCheck(vkCreateDescriptorPool(ctx.device, &poolInfo, nullptr, &ctx.descriptorPool));
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
    // Force no vsync
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
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

#include "Ctx.h"

#ifdef NDEBUG
const bool enableValidation = false;
#else
const bool enableValidation = true;
#endif

// Private helpers
void _initInstance(Ctx& ctx);
void _initWindow(Ctx& ctx);
void _initPhysicalDevice(Ctx& ctx);

// Public
Ctx createCtx() {
    Ctx ctx{};
    _initInstance(ctx);
    _initWindow(ctx);
    _initPhysicalDevice(ctx);
    return ctx;
}

void destroyCtx(Ctx& ctx) {
    logger::debug("destroying ctx");
    vkDestroyInstance(ctx.instance, nullptr);
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
}

// Private implementation
void _initInstance(Ctx& ctx) {
    auto appInfo = vks::initializers::applicationInfo(VK_MAKE_VERSION(1,0,0));

    std::vector<const char*> validationLayers{};
    if (enableValidation) {
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    auto createInfo = vks::initializers::instanceInfo(&appInfo, validationLayers, {});

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

    ctx.window = glfwCreateWindow(640, 480, "CVulkan", nullptr, nullptr);
    if (!ctx.window) {
        logger::crash("Unable to open window");
        exit(1);
    }
}

void _initPhysicalDevice(Ctx& ctx) {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        logger::crash("No GPUs detected in this machine");
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, devices);

    uint32_t i = 0;
    for (auto device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        logger::debug("Device {}: {}", i++, properties.deviceName);
    }

    ctx.physicalDevice = devices[0];
}

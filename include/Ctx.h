#include <precomp.h>

struct Ctx {
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
};

Ctx createCtx();
void destroyCtx(Ctx&);


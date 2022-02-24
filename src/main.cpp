#include <precomp.h>
#include <Ctx.h>



int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = ctxCreate();

    while (!glfwWindowShouldClose(ctx.window.glfwWindow)) {
        glfwPollEvents();
    }

    ctxDestroy(ctx);
    return 0;
}

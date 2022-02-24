#include <precomp.h>
#include <Ctx.h>

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = createCtx();

    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
    }

    destroyCtx(ctx);
    return 0;
}

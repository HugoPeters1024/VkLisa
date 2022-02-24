#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = ctxCreate();

    RenderPassInfo renderPassInfo{};
    auto renderPass = renderPassCreate(ctx, renderPassInfo);

    RastPipelineInfo rastInfo {
        .vsPath = "./shaders_bin/quad.vert.spv",
        .fsPath = "./shaders_bin/quad.frag.spv",
        .renderPass = &renderPass,
    };

    auto pipeline = rastPipelineCreate(ctx, rastInfo);
    rastPipelineDestroy(ctx, pipeline);

    renderPassDestroy(ctx, renderPass);
    
    while (!ctxWindowShouldClose(ctx)) {
        ctxUpdate(ctx);
    }

    ctxDestroy(ctx);
    return 0;
}

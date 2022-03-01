#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <Comp.h>
#include <BufferTools.h>
#include <GridRender.h>

const uint32_t g_imageWidth = 128;
const uint32_t g_imageHeight = 128;
const uint32_t g_instancesWidth = 5;
const uint32_t g_instancesHeight = 5;
const uint32_t g_windowWidth = g_imageWidth * g_instancesWidth;
const uint32_t g_windowHeight = g_imageHeight * g_instancesHeight;


struct CompPushConstants {
    uint32_t nrVertices;
    uint32_t seed;
};

Ctx mkCtx() {
    CtxInfo info { 
        .windowWidth = g_windowWidth,
        .windowHeight = g_windowHeight,
        .extensions = {
            "GL_KHR_shader_subgroup",
            "GL_KHR_shader_subgroup_arithmetic",
        }
    };

    return ctxCreate(info);
}

void printSubgroupInfo(const Ctx& ctx) {
    VkPhysicalDeviceSubgroupProperties subgroupProperties{};
    subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

    VkPhysicalDeviceProperties2 physicalDeviceProperties{};
    physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties.pNext = &subgroupProperties;

    vkGetPhysicalDeviceProperties2(ctx.physicalDevice, &physicalDeviceProperties);

    logger::info("warp size: {}", subgroupProperties.subgroupSize);
}

CompPipeline mkCompPipeline(const Ctx& ctx) {
    auto pushInfo = vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(CompPushConstants), 0);
    CompInfo compInfo {
        .compShaderPath = "./shaders_bin/reduce.comp.spv",
        .bindingDescription = {
            { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        },
        .pushConstantRange = &pushInfo,
    };
    return compCreate(ctx, compInfo);
}

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = mkCtx();
    printSubgroupInfo(ctx);

    std::vector<Vertex> vertexData(3*9*10);
    for(auto i=0; i<vertexData.size(); i++) {
        vertexData[i] = Vertex {
            { randf(), randf(), 0, 0 },
            { randf(1.0f), randf(1.0f), randf(1.0f), randf(0.1f) }
        };
    }

    GridRenderInfo gridRenderInfo {
        .vertexData = &vertexData,
    };
    auto gridRender = gridRenderCreate(ctx, gridRenderInfo);

    GridPushConstants gridPush {
        .nrTriangles = static_cast<uint32_t>(vertexData.size() / 3),
        .nrInstanceWidth = g_instancesWidth,
        .nrInstancesHeight = g_instancesHeight,
    };

    uint zero = 0;
    auto resultBuffer = buffertools::createBufferD_Data(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 1 * sizeof(uint32_t), &zero);
    auto comp = mkCompPipeline(ctx);


    CompResourceBindings compBindings {
        { 0, gridRender.vertexBuffer.buffer },
        { 1, resultBuffer.buffer },
    };

    CompPushConstants compPush {
        .nrVertices = static_cast<uint32_t>(vertexData.size()),
        .seed = 1,
    };

    auto compDescriptorSet = compCreateDescriptorSet(ctx, comp, compBindings);

    double ping;
    uint32_t frameCounter = 0;
    while (!ctxWindowShouldClose(ctx)) {
        ping = glfwGetTime();

        auto frame = ctxBeginFrame(ctx);

        auto beginInfo = vks::initializers::commandBufferBeginInfo();
        vkCheck(vkBeginCommandBuffer(frame.cmdBuffer, &beginInfo));

        grindRenderRecord(ctx, gridRender, gridPush);
        vkCmdBindPipeline(frame.cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp.pipeline);
        vkCmdPushConstants(frame.cmdBuffer, comp.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CompPushConstants), &compPush);
        vkCmdBindDescriptorSets(frame.cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp.pipelineLayout, 0, 1, &compDescriptorSet, 0, nullptr);
        vkCmdDispatch(frame.cmdBuffer, vertexData.size()/256+1, 1, 1);

        vkCheck(vkEndCommandBuffer(frame.cmdBuffer));
        ctxEndFrame(ctx, frame.cmdBuffer);

        if (frameCounter % 10 == 0) {
            compPush.seed = 3*rand_xorshift(17*compPush.seed);
            double fps = 1.0f / (glfwGetTime() - ping);
            logger::info("FPS: {}", fps);
        }
        
        frameCounter++;
    }

    ctxFinish(ctx);
    gridRenderDestroy(ctx, gridRender);
    compDestroy(ctx, comp);
    buffertools::destroyBuffer(ctx, resultBuffer);
    ctxDestroy(ctx);
    return 0;
}

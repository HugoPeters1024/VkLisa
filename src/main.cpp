#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <Comp.h>
#include <BufferTools.h>
#include <GridRender.h>
#include <QuadRender.h>

const uint32_t g_imageWidth = 128;
const uint32_t g_imageHeight = 128;
const uint32_t g_trianglesPerInstance = 10;
const uint32_t g_instancesWidth = 8;
const uint32_t g_instancesHeight = 8;
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

int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    auto ctx = mkCtx();
    printSubgroupInfo(ctx);

    std::vector<Vertex> vertexData(3*g_instancesWidth*g_instancesHeight*g_trianglesPerInstance);
    for(auto i=0; i<vertexData.size(); i++) {
        vertexData[i] = Vertex {
            { randf(), randf(), 0, 0 },
            { randf(1.0f), randf(1.0f), randf(1.0f), randf(0.1f) }
        };
    }

    Image target = createImageD(
            ctx, ctx.window.width, ctx.window.height,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    GridRenderInfo gridRenderInfo {
        .vertexData = &vertexData,
        .target = target,
    };

    auto gridRender = gridRenderCreate(ctx, gridRenderInfo);

    GridPushConstants gridPush {
        .nrTriangles = static_cast<uint32_t>(vertexData.size() / 3),
        .nrInstanceWidth = g_instancesWidth,
        .nrInstancesHeight = g_instancesHeight,
    };

    QuadRenderInfo quadRenderInfo {
        .srcImage = target,
        .beforeLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    auto quadRender = quadRenderCreate(ctx, quadRenderInfo);

    auto compPushInfo = vks::initializers::pushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(CompPushConstants), 0);
    CompInfo compInfo {
        .compShaderPath = "./shaders_bin/reduce.comp.spv",
        .bindingDescription = {
            { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        },
        .pushConstantRange = &compPushInfo,
    };
    auto comp = compCreate(ctx, compInfo);

    uint zero = 0;
    std::vector<uint32_t> ints(1024*1024, 1);
    auto intBuffer = buffertools::createBufferD_Data(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, ints.size() * sizeof(uint32_t), ints.data());
    auto resultBuffer = buffertools::createBufferH2D_Data(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 1 * sizeof(uint32_t), &zero);


    CompResourceBindings compBindings {
        { 0, intBuffer.buffer },
        { 1, resultBuffer.buffer },
    };
    VkDescriptorSet compDescriptorSet = compCreateDescriptorSet(ctx, comp, compBindings);

    CompPushConstants compPush {
        .nrVertices = static_cast<uint32_t>(vertexData.size()),
        .seed = 1,
    };


    double ping;
    uint32_t frameCounter = 0;
    while (!ctxWindowShouldClose(ctx)) {
        ping = glfwGetTime();

        auto frame = ctxBeginFrame(ctx);

        auto beginInfo = vks::initializers::commandBufferBeginInfo();
        vkCheck(vkBeginCommandBuffer(frame.cmdBuffer, &beginInfo));

        grindRenderRecord(ctx, gridRender, gridPush);
        quadRenderRecord(ctx, quadRender);

        if (frameCounter == 0) {

            vkCmdBindPipeline(frame.cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp.pipeline);
            vkCmdPushConstants(frame.cmdBuffer, comp.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CompPushConstants), &compPush);
            vkCmdBindDescriptorSets(frame.cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp.pipelineLayout, 0, 1, &compDescriptorSet, 0, nullptr);
            vkCmdDispatch(frame.cmdBuffer, ints.size()/256+1, 1, 1);

            auto barrier = vks::initializers::memoryBarrier();
            barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            vkCmdPipelineBarrier(frame.cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_DEVICE_GROUP_BIT, 1, &barrier, 0, nullptr, 0, nullptr);
        }

        vkCheck(vkEndCommandBuffer(frame.cmdBuffer));
        ctxEndFrame(ctx, frame.cmdBuffer);

        if (frameCounter % 100 == 0) {
            compPush.seed = 3*rand_xorshift(17*compPush.seed);
            double fps = 1.0f / (glfwGetTime() - ping);
            logger::info("FPS: {}", fps);

            uint32_t* data;
            vmaMapMemory(ctx.allocator, resultBuffer.memory, (void**)&data);
            logger::info("result: {}", data[0]);
            vmaUnmapMemory(ctx.allocator, resultBuffer.memory);
        }
        
        frameCounter++;
    }

    ctxFinish(ctx);
    destroyImage(ctx, target);
    gridRenderDestroy(ctx, gridRender);
    quadRenderDestroy(ctx, quadRender);
    compDestroy(ctx, comp);
    buffertools::destroyBuffer(ctx, intBuffer);
    buffertools::destroyBuffer(ctx, resultBuffer);
    ctxDestroy(ctx);
    return 0;
}

#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <Comp.h>
#include <BufferTools.h>
#include <GridRender.h>
#include <QuadRender.h>
#include <Evolve.h>
#include <Lottery.h>

constexpr uint32_t g_imageWidth = 128;
constexpr uint32_t g_imageHeight = 128;
constexpr uint32_t g_trianglesPerInstance = 20;
constexpr uint32_t g_instancesWidth = 8;
constexpr uint32_t g_instancesHeight = 8;
constexpr uint32_t g_totalInstances = g_instancesWidth * g_instancesHeight;
constexpr uint32_t g_totalTriangles = g_totalInstances * g_trianglesPerInstance;
constexpr uint32_t g_windowWidth = g_imageWidth * g_instancesWidth;
constexpr uint32_t g_windowHeight = g_imageHeight * g_instancesHeight;

Ctx ctx;
struct {
    Image gridTarget;
    Buffer vertexBuffers[2];
    Buffer scoresBuffer;
    Buffer parentsBuffer;
} resources;


Ctx mkCtx();
void printSubgroupInfo(const Ctx& ctx);
void initResources();
Evolve initEvolve();
GridRender initGridRender();
QuadRender initQuadRender();
Lottery initLottery();


int main(int argc, char** argv) {
    logger::set_level(spdlog::level::trace);

    ctx = mkCtx();
    printSubgroupInfo(ctx);

    initResources();

    auto evolve = initEvolve();
    auto lottery = initLottery();
    auto gridRender = initGridRender();
    auto quadRender = initQuadRender();


    GridRenderArgs gridArgs {
        .nrTriangles = g_totalTriangles,
        .nrInstanceWidth = g_instancesWidth,
        .nrInstancesHeight = g_instancesHeight,
    };

    EvolveArgs evolveArgs{
        .nrVertices = 3 * g_totalTriangles,
        .nrTrianglesPerInstance = g_trianglesPerInstance,
    };

    LotteryArgs lotteryArgs {
        .nrInstances = g_totalInstances,
        .seed = 0,
    };

    double ping;
    uint32_t frameCounter = 0;
    while (!ctxWindowShouldClose(ctx)) {
        ping = glfwGetTime();

        auto frame = ctxBeginFrame(ctx);

        auto beginInfo = vks::initializers::commandBufferBeginInfo();
        vkCheck(vkBeginCommandBuffer(frame.cmdBuffer, &beginInfo));

        lotteryArgs.seed = rand_xorshift(evolveArgs.seed);
        lotteryRecord(ctx, lottery, lotteryArgs);

        evolveArgs.seed = rand_xorshift(7*frame.frameIdx),
        evolveRecord(ctx, evolve, evolveArgs);


        grindRenderRecord(ctx, gridRender, gridArgs);
        quadRenderRecord(ctx, quadRender);

        vkCheck(vkEndCommandBuffer(frame.cmdBuffer));
        ctxEndFrame(ctx, frame.cmdBuffer);

        if (frameCounter % 100 == 0) {
            double fps = 1.0f / (glfwGetTime() - ping);
            logger::info("FPS: {}", fps);
        }
        
        frameCounter++;
    }

    ctxFinish(ctx);
    for (auto& buffer : resources.vertexBuffers) {
        buffertools::destroyBuffer(ctx, buffer);
    }

    destroyImage(ctx, resources.gridTarget);
    evolveDestroy(ctx, evolve);
    gridRenderDestroy(ctx, gridRender);
    quadRenderDestroy(ctx, quadRender);
    ctxDestroy(ctx);
    return 0;
}

Ctx mkCtx() {
    CtxInfo info { 
        .windowWidth = g_windowWidth,
        .windowHeight = g_windowHeight,
        .extensions = {}
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

void initResources() {
    resources.gridTarget = createImageD(
            ctx, ctx.window.width, ctx.window.height,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Double buffered vertex buffers
    std::vector<Vertex> vertexData(3 * g_totalTriangles);
    resources.vertexBuffers[1] = buffertools::createBufferD_Data(ctx,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vertexData.size() * sizeof(Vertex), vertexData.data());

    for(auto i=0; i<vertexData.size(); i++) {
        vertexData[i] = Vertex {
            { randf(), randf(), 0, 0 },
            { randf(1.0f), randf(1.0f), randf(1.0f), randf(0.1f) }
        };
    }
    resources.vertexBuffers[0] = buffertools::createBufferD_Data(ctx,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        vertexData.size() * sizeof(Vertex), vertexData.data());


    // last element is used as the total
    std::vector<float> scores(g_totalInstances+1, 1.0f);
    std::vector<uint32_t> parents(g_totalInstances*2, 0);
    scores[g_totalInstances] = 0;
    resources.scoresBuffer = buffertools::createBufferD_Data(ctx,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        scores.size() * sizeof(uint32_t), scores.data());

    resources.parentsBuffer = buffertools::createBufferD_Data(ctx,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        parents.size() * sizeof(uint32_t), parents.data());
}


Evolve initEvolve() {
    EvolveInfo evolveInfo {
        .vertexBuffers = { &resources.vertexBuffers[0], &resources.vertexBuffers[1] },
        .parentBuffer = &resources.parentsBuffer,
    };
    return evolveCreate(ctx, evolveInfo);
}

GridRender initGridRender() {
    GridRenderInfo gridRenderInfo {
        .buffers = { &resources.vertexBuffers[0], &resources.vertexBuffers[1] },
        .target = resources.gridTarget,
    };

    return gridRenderCreate(ctx, gridRenderInfo);
}

QuadRender initQuadRender() {
    QuadRenderInfo quadRenderInfo {
        .srcImage = resources.gridTarget,
        .beforeLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    return quadRenderCreate(ctx, quadRenderInfo);
}

Lottery initLottery() {
    LotteryInfo info {
        .scoreBuffer = &resources.scoresBuffer,
        .parentBuffer = &resources.parentsBuffer,
    };

    return lotteryCreate(ctx, info);
}

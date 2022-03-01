#pragma once
#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <BufferTools.h>

struct GridRenderInfo {
    std::vector<Vertex>* vertexData;
};

struct GridPushConstants { 
    uint32_t nrTriangles;
    uint32_t nrInstanceWidth;
    uint32_t nrInstancesHeight;
};

struct GridRender {
    GridRenderInfo info;
    RenderPass renderPass;
    RastPipeline pipeline;
    Buffer vertexBuffer;
};

GridRender gridRenderCreate(Ctx& ctx, GridRenderInfo& info);
void gridRenderDestroy(Ctx& ctx, GridRender& gridRender);
void grindRenderRecord(Ctx& ctx, GridRender& gridRender, GridPushConstants& push);

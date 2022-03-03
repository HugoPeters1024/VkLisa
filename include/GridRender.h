#pragma once
#include <precomp.h>
#include <Ctx.h>
#include <RenderPass.h>
#include <Rast.h>
#include <BufferTools.h>
#include <ImageTools.h>

struct GridRenderInfo {
    Buffer* buffers[2];
    Image target;
};

struct GridRenderArgs { 
    uint32_t nrTriangles;
    uint32_t nrInstanceWidth;
    uint32_t nrInstancesHeight;
};

struct GridRender {
    GridRenderInfo info;
    RenderPass renderPass;
    RastPipeline pipeline;
};

GridRender gridRenderCreate(Ctx& ctx, GridRenderInfo& info);
void gridRenderDestroy(Ctx& ctx, GridRender& gridRender);
void grindRenderRecord(Ctx& ctx, GridRender& gridRender, GridRenderArgs& push);

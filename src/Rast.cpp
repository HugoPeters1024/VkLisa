#include <Rast.h>

void _ensureInfoComplete(const RastPipelineInfo& info) {
    assert(info.fsPath);
    assert(info.vsPath);
    assert(info.renderPass);
}

RastPipeline rastPipelineCreate(const Ctx& ctx, const RastPipelineInfo& info) {
    _ensureInfoComplete(info);
    RastPipeline ret{};

    // pipeline layout
    auto pipelineLayoutInfo = vks::initializers::pipelineLayoutCreateInfo();
    pipelineLayoutInfo.pushConstantRangeCount = info.pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = info.pushConstantRanges.data();
    pipelineLayoutInfo.setLayoutCount = info.descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = info.descriptorSetLayouts.data();
    vkCheck(vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &ret.pipelineLayout));


    logger::debug("Loading vertex shader: {}", info.vsPath);
    auto vertShader = vks::tools::loadShader(info.vsPath, ctx.device);
    logger::debug("Loading fragment shader: {}", info.fsPath);
    auto fragShader = vks::tools::loadShader(info.fsPath, ctx.device);

    auto vertStage = vks::initializers::pipelineShaderStageCreateInfo(vertShader, VK_SHADER_STAGE_VERTEX_BIT);
    auto fragStage = vks::initializers::pipelineShaderStageCreateInfo(fragShader, VK_SHADER_STAGE_FRAGMENT_BIT);
    
    std::array<VkPipelineShaderStageCreateInfo,2> stages { vertStage, fragStage };

    // What kind of vertex data
    auto vertexInputInfo = vks::initializers::pipelineVertexInputStateCreateInfo({},{});

    if (info.vertexDescription) {
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &info.vertexDescription->bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = info.vertexDescription->attributeDescriptions.size();
        vertexInputInfo.pVertexAttributeDescriptions = info.vertexDescription->attributeDescriptions.data();
    }

    // Kind of geometry
    auto inputAssembly = vks::initializers::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    // Viewport & Scissors
    auto viewPort = vks::initializers::viewport(ctx.window.width, ctx.window.height, 0.0f, 1.0f);
    auto scissors = vks::initializers::rect2D(ctx.window.width, ctx.window.height, 0, 0);

    auto viewportState = vks::initializers::pipelineViewportStateCreateInfo(&viewPort, &scissors);
    auto rasterizer = vks::initializers::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    // no multisampling
    auto multisampling = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

    // all colors no blending for output attachment
    auto colorBlendAttachment = vks::initializers::pipelineColorBlendAttachmentState(
            vks::tools::VK_COLOR_COMPONENT_FLAG_ALL, VK_TRUE);

    // only 1 attachment
    auto colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);


    auto pipelineInfo = vks::initializers::pipelineCreateInfo(ret.pipelineLayout, info.renderPass->renderPass);
    pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDynamicState = nullptr;
    vkCheck(vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.pipeline));
    logger::debug("rasterization pipeline created");


    vkDestroyShaderModule(ctx.device, vertShader, nullptr);
    vkDestroyShaderModule(ctx.device, fragShader, nullptr);

    return ret;
}

void rastPipelineDestroy(const Ctx& ctx, RastPipeline& pipeline) {
    vkDestroyPipelineLayout(ctx.device, pipeline.pipelineLayout, nullptr);
    vkDestroyPipeline(ctx.device, pipeline.pipeline, nullptr);
}

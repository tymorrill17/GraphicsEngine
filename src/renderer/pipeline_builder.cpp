#include "renderer/pipeline_builder.h"

PipelineBuilder::PipelineBuilder(const Device& device) : _device(device) {
	clear();
}

Pipeline PipelineBuilder::buildPipeline() {
    static Logger& logger = Logger::getLogger();

    if (_pipelineLayout == VK_NULL_HANDLE) {
        throw std::runtime_error("No pipeline layout fouund! Please include a pipeline layout to the pipeline builder!");
    }

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Setup dummy color blending. Not using transparent objects yet. 
    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &_config.colorBlendAttachment
    };
    
    // Not used yet so just initialize it to default
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    // Dynamic states allow us to specify these things at command recording instead of pipeline creation
    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = &state[0]
    };

    // Build the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &_config.renderingInfo,
        .stageCount = static_cast<uint32_t>(_config.shaderModules.size()),
        .pStages = _config.shaderModules.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &_config.inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &_config.rasterizer,
        .pMultisampleState = &_config.multisampling,
        .pDepthStencilState = &_config.depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = _pipelineLayout,
        .renderPass = nullptr // Using dynamic rendering
    };

    VkPipeline vkPipeline;
    if (vkCreateGraphicsPipelines(_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline");
    }

    Pipeline newPipeline(&_device, vkPipeline, _pipelineLayout);
    logger.print("Successfully Created Render Pipeline");

    return newPipeline;
}

void PipelineBuilder::clear() {
    _config.shaderModules.clear();
    _config.vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    _config.inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    _config.rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    _config.colorBlendAttachment = {};
    _config.multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    _config.depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    _config.renderingInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    _config.colorAttachmentFormat = VK_FORMAT_UNDEFINED;
    _pipelineLayout = VK_NULL_HANDLE;
}

PipelineBuilder& PipelineBuilder::setConfig(PipelineConfig config) {
    clear();
    _config = config;
    return *this;
}

// Shaders

PipelineBuilder& PipelineBuilder::setShader(Shader& shader) {
    _config.shaderModules.push_back(Shader::pipelineShaderStageCreateInfo(shader.stage(), shader.module()));
    return *this;
}

// Pipeline State

PipelineBuilder& PipelineBuilder::setInputTopology(VkPrimitiveTopology topology) {
    _config.inputAssembly.topology = topology;
    _config.inputAssembly.primitiveRestartEnable = VK_FALSE; // Not using for now
    return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    _config.rasterizer.polygonMode = mode;
    _config.rasterizer.lineWidth = 1.0f; // Setting this to a default of 1.0
    return *this;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
    _config.rasterizer.cullMode = cullMode;
    _config.rasterizer.frontFace = frontFace;
    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampling(VkSampleCountFlagBits sampleCount) {
    // TODO: Defaulting to none until I learn more about this
    _config.multisampling.sampleShadingEnable = VK_FALSE;
    _config.multisampling.rasterizationSamples = sampleCount;
    _config.multisampling.minSampleShading = 1.0f;
    _config.multisampling.pSampleMask = nullptr;
    _config.multisampling.alphaToCoverageEnable = VK_FALSE;
    _config.multisampling.alphaToOneEnable = VK_FALSE;
    return *this;
}

PipelineBuilder& PipelineBuilder::setBlending(bool enable) {
    _config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _config.colorBlendAttachment.blendEnable = enable;
    return *this;
}

PipelineBuilder& PipelineBuilder::setColorAttachmentFormat(VkFormat format) {
    _config.colorAttachmentFormat = format;
    _config.renderingInfo.colorAttachmentCount = 1;
    _config.renderingInfo.pColorAttachmentFormats = &_config.colorAttachmentFormat;
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthAttachmentFormat(VkFormat format) {
    _config.renderingInfo.depthAttachmentFormat = format;
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthTest(VkCompareOp compareOp) {
    _config.depthStencil.depthTestEnable = compareOp == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    _config.depthStencil.depthWriteEnable = compareOp == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    _config.depthStencil.depthCompareOp = compareOp;
    _config.depthStencil.depthBoundsTestEnable = VK_FALSE;
    _config.depthStencil.stencilTestEnable = VK_FALSE;
    _config.depthStencil.front = {};
    _config.depthStencil.back = {};
    _config.depthStencil.minDepthBounds = 0.0f;
    _config.depthStencil.maxDepthBounds = 1.0f;
    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexInputState(VkPipelineVertexInputStateCreateInfo createInfo) {
    _config.vertexInputInfo = createInfo;
    return *this;
}

// Pipeline Layout

void PipelineBuilder::setPipelineLayout(VkPipelineLayout layout) {
    _pipelineLayout = layout;
}

void PipelineBuilder::setPipelineLayout(VkPipelineLayoutCreateInfo layoutInfo) {
    VkPipelineLayout layout = createPipelineLayout(_device, layoutInfo);
    _pipelineLayout = layout;
}

//-------------------------- Static methods ---------------------------------//

VkPipelineLayout PipelineBuilder::createPipelineLayout(const Device& device, VkPipelineLayoutCreateInfo createInfo) {
    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device.device(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    return pipelineLayout;
}

VkPipelineLayoutCreateInfo PipelineBuilder::pipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& setLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges) {
    VkPipelineLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
        .pSetLayouts = setLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
        .pPushConstantRanges = pushConstantRanges.data()
    };
    return createInfo;
}

VkPipelineVertexInputStateCreateInfo PipelineBuilder::vertexInputStateCreateInfo() {
    VkPipelineVertexInputStateCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0
    };
    return createInfo;
}
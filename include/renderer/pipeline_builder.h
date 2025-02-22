#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "device.h"
#include "shader.h"
#include "pipeline.h"
#include <vector>
#include <stdexcept>

class Pipeline;

struct PipelineConfig {
	// Shaders
	std::vector<VkPipelineShaderStageCreateInfo> shaderModules;

	// Pipeline State
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{ .sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{ .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo rasterizer{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineMultisampleStateCreateInfo multisampling{ .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo depthStencil{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineRenderingCreateInfo renderingInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	VkFormat colorAttachmentFormat{ VK_FORMAT_UNDEFINED };
};

class PipelineBuilder : public NonCopyable {
public:
	PipelineBuilder(const Device& device);

	// @brief Resets the PipelineBuilder to its default state
	void clear();

	// @brief Build a Pipeline with the current chosen parameters of the PipelineBuilder
	// TODO: possibly move this to the Pipeline class so that each type of pipeline can adjust how they're built?
	Pipeline buildPipeline();

	PipelineBuilder& setConfig(PipelineConfig config);
	inline PipelineConfig config() const { return _config; }

	// Shaders
	PipelineBuilder& setShader(Shader& shader);

	// Pipeline State
	PipelineBuilder& setInputTopology(VkPrimitiveTopology topology);
	PipelineBuilder& setPolygonMode(VkPolygonMode mode);
	PipelineBuilder& setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	PipelineBuilder& setMultisampling(VkSampleCountFlagBits sampleCount);
	PipelineBuilder& setBlending(bool enable);
	PipelineBuilder& setColorAttachmentFormat(VkFormat format);
	PipelineBuilder& setDepthAttachmentFormat(VkFormat format);
	PipelineBuilder& setDepthTest(VkCompareOp compareOp = VK_COMPARE_OP_NEVER);
	PipelineBuilder& setVertexInputState(VkPipelineVertexInputStateCreateInfo createInfo);

	// Pipeline Layout
	// @brief Create a default, blank VkPipelineLayoutCreateInfo struct
	static VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& setLayouts = {}, const std::vector<VkPushConstantRange>& pushConstantRanges = {});
	// Creates a pipeline layout using the given create info
	static VkPipelineLayout createPipelineLayout(const Device& device, VkPipelineLayoutCreateInfo createInfo);
	void setPipelineLayout(VkPipelineLayout layout);
	void setPipelineLayout(VkPipelineLayoutCreateInfo layoutInfo);

	// TODO: Get rid of this weird createInfo function and add some functions like above to configure the vertex input state
	static VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo();

private:
	// @brief Reference to the Vulkan device which creates the pipelines
	const Device& _device;
	PipelineConfig _config;
	// I want pipeline layouts to be created when the pipeline is built. I don't want PipelineBuilder
	// to have an instance of the pipeline layout. But let me refactor the config part first
	VkPipelineLayout _pipelineLayout;
};
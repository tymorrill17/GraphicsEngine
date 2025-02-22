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
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineRenderingCreateInfo renderingInfo;
	VkFormat colorAttachmentFormat;
};

class PipelineBuilder : public NonCopyable {
public:
	PipelineBuilder(const Device& device);

	// @brief Resets the PipelineBuilder to its default state
	void clear();

	// @brief Build a Pipeline with the current chosen parameters of the PipelineBuilder
	// TODO: possibly move this to the Pipeline class so that each type of pipeline can adjust how they're built?
	Pipeline buildPipeline();

	void setConfig(PipelineConfig config);
	inline PipelineConfig config() const { return _config; }

	// Shaders
	void setVertexShader(VkShaderModule shader);
	void setFragmentShader(VkShaderModule shader);
	void setShader(VkShaderModule shader, VkShaderStageFlagBits shaderStage);

	// Pipeline State
	void setInputTopology(VkPrimitiveTopology topology);
	void setPolygonMode(VkPolygonMode mode);
	void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	void setMultisampling(VkSampleCountFlagBits sampleCount);
	void setBlending(bool enable);
	void setColorAttachmentFormat(VkFormat format);
	void setDepthAttachmentFormat(VkFormat format);
	void setDepthTest(VkCompareOp compareOp = VK_COMPARE_OP_NEVER);
	void setVertexInputState(VkPipelineVertexInputStateCreateInfo createInfo);

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
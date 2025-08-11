#include "renderer/pipeline.h"

Pipeline::Pipeline() :
	_device(nullptr), _pipeline(VK_NULL_HANDLE), _pipelineLayout(VK_NULL_HANDLE) {}

Pipeline::Pipeline(const Device* device, VkPipeline pipeline, VkPipelineLayout pipelineLayout) : 
	_device(device),
	_pipeline(pipeline),
	_pipelineLayout(pipelineLayout) {}

Pipeline::~Pipeline() {
	cleanup();
}

void Pipeline::cleanup() {
	if (!_device) return;

	if (_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(_device->device(), _pipelineLayout, nullptr);
		_pipelineLayout = VK_NULL_HANDLE;
	}
	if (_pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(_device->device(), _pipeline, nullptr);
		_pipeline = VK_NULL_HANDLE;
	}
}

Pipeline::Pipeline(Pipeline&& other) noexcept : _pipeline(other._pipeline), _pipelineLayout(other._pipelineLayout), _device(other._device) {
	// Reset other pipeline's members
	other._pipeline = VK_NULL_HANDLE;
	other._pipelineLayout = VK_NULL_HANDLE;
}
Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
	if (this != &other) {
		// Destroy existing resources
		cleanup();

		// Transfer ownership
		_pipeline = other._pipeline;
		_pipelineLayout = other._pipelineLayout;
		// Keep the device reference intact (no need to reassign)

		// Reset the moved-from object
		other._pipeline = VK_NULL_HANDLE;
		other._pipelineLayout = VK_NULL_HANDLE;
	}
	return *this;
}

VkPipelineLayout PipelineLayout::createPipelineLayout(const Device& device, VkPipelineLayoutCreateInfo createInfo) {
	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device.device(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}
	return pipelineLayout;
}

VkPipelineLayoutCreateInfo PipelineLayout::pipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout>& setLayouts, const std::vector<VkPushConstantRange>& pushConstantRanges) {
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


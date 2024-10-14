#include "renderer/command.h"
#include "renderer/frame.h"

Command::Command(const Device& device, VkCommandPoolCreateFlags flags) : _device(device), _commandPool(VK_NULL_HANDLE), _commandBuffer(VK_NULL_HANDLE), _flags(flags) {

	// First, create the command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = _device.queueFamilyIndices().graphicsFamily.value()
	};

	if (vkCreateCommandPool(_device.device(), &commandPoolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}

	// Now allocate the command buffer
	allocateCommandBuffer();
}

Command::~Command() {
	cleanup();
}

void Command::allocateCommandBuffer(VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = _commandPool,
		.level = level,
		.commandBufferCount = 1,
	};
	if (vkAllocateCommandBuffers(_device.device(), &allocateInfo, &_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffer!");
	}
}

VkCommandBufferBeginInfo Command::commandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = flags,
		.pInheritanceInfo = nullptr
	};
	return beginInfo;
}

void Command::begin() const {
	VkCommandBufferBeginInfo beginInfo = commandBufferBeginInfo();
	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin command buffer!");
	}
}

void Command::end() const {
	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer!");
	}
}

void Command::reset(VkCommandBufferResetFlags flags) const {
	if (vkResetCommandBuffer(_commandBuffer, flags) != VK_SUCCESS) {
		throw std::runtime_error("Failed to reset the command buffer!");
	}
}

void Command::submitToQueue(VkQueue queue, Frame& frame) {
	VkCommandBufferSubmitInfo cmdSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = _commandBuffer,
		.deviceMask = 0
	};
	// This semaphore waits until the previous frame has been presented
	VkSemaphoreSubmitInfo waitSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = frame.presentSemaphore().handle(),
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		.deviceIndex = 0
	};
	// This semaphore waits until the frame is fully rendered
	VkSemaphoreSubmitInfo signalSemaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = frame.renderSemaphore().handle(),
		.value = 1,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		.deviceIndex = 0
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &waitSemaphoreInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signalSemaphoreInfo
	};

	if (vkQueueSubmit2(queue, 1, &submitInfo, frame.renderFence().handle()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit commands to queue!");
	}
}

Command::Command(Command&& other) noexcept : _device(other._device),
	_commandPool(other._commandPool),
	_commandBuffer(other._commandBuffer),
	_flags(other._flags) {
	other._commandPool = VK_NULL_HANDLE;
	other._commandBuffer = VK_NULL_HANDLE;
	other._flags = 0;
}

Command& Command::operator=(Command&& other) noexcept {
	if (this != &other) {
		cleanup();

		_commandPool = other._commandPool;
		_commandBuffer = other._commandBuffer;
		_flags = other._flags;

		other._commandPool = VK_NULL_HANDLE;
		other._commandBuffer = VK_NULL_HANDLE;
		other._flags = 0;
	}
	return *this;
}

void Command::cleanup() {
	vkDestroyCommandPool(_device.device(), _commandPool, nullptr);
}
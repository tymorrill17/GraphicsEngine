#include "renderer/buffer.h"

AllocatedBuffer::AllocatedBuffer(const Device& device, Allocator& allocator,
	size_t allocationSize, VkBufferUsageFlags usageFlags,VmaMemoryUsage memoryUsage) : 
	_buffer(VK_NULL_HANDLE), _allocation(nullptr), _allocationInfo(),
	_device(device), _allocator(allocator) {
	
	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = allocationSize,
		.usage = usageFlags
	};

	VmaAllocationCreateInfo allocationCreateInfo{
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage
	};

	if (vmaCreateBuffer(_allocator.handle(), &bufferCreateInfo, &allocationCreateInfo, &_buffer, &_allocation, &_allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create allocated buffer!");
	}
}

void AllocatedBuffer::cleanup() {
	vmaDestroyBuffer(_allocator.handle(), _buffer, _allocation);
}
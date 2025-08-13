#include "renderer/descriptor.h"

// ---------------------------------------------- DESCRIPTOR POOL -----------------------------------------------------------------

DescriptorPool::DescriptorPool(Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolSizeRatios) :
	_device(device), _descriptorPool(VK_NULL_HANDLE) {

	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolSizeRatios) {
		poolSizes.emplace_back(ratio.type, static_cast<uint32_t>(ratio.ratio * maxSets));
	}
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = maxSets,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};
	if (vkCreateDescriptorPool(_device.handle(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(_device.handle(), _descriptorPool, nullptr);
}

void DescriptorPool::clearDescriptorSets() {
	vkResetDescriptorPool(_device.handle(), _descriptorPool, 0);
}

VkDescriptorSet DescriptorPool::allocateDescriptorSet(VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = _descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};
	VkDescriptorSet set;
	if (vkAllocateDescriptorSets(_device.handle(), &allocInfo, &set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}
	return set;
}

// ---------------------------------------------- DESCRIPTOR LAYOUT BUILDER -----------------------------------------------------------------

DescriptorLayoutBuilder::DescriptorLayoutBuilder(Device& device) : _device(device) {}

DescriptorLayoutBuilder& DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding newBinding{
		.binding = binding,
		.descriptorType = descriptorType,
		.descriptorCount = 1,
		.stageFlags = stageFlags
	};
	_bindings.push_back(newBinding);
	return *this;
}

DescriptorLayoutBuilder& DescriptorLayoutBuilder::clear() {
	_bindings.clear();
	return *this;
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build() {
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(_bindings.size()),
		.pBindings = _bindings.data()
	};
	VkDescriptorSetLayout layout;
	if (vkCreateDescriptorSetLayout(_device.handle(), &descriptorSetLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to build descriptor set layout!");
	}
	return layout;
}

// ---------------------------------------------- DESCRIPTOR WRITER -----------------------------------------------------------------

DescriptorWriter::DescriptorWriter(Device& device) : _device(device) {}

DescriptorWriter& DescriptorWriter::addImageWrite(uint32_t binding, AllocatedImage& image, VkSampler sampler, VkDescriptorType descriptorType) {
	VkDescriptorImageInfo& imageInfo = _imageInfos.emplace_back( VkDescriptorImageInfo {
		.sampler = sampler,
		.imageView = image.imageView(),
		.imageLayout = image.imageLayout()
		});

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = descriptorType,
		.pImageInfo = &imageInfo
	};

	_writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::addBufferWrite(uint32_t binding, Buffer& buffer, VkDescriptorType descriptorType, size_t offset, size_t bufferSize) {
	VkDescriptorBufferInfo& bufferInfo = _bufferInfos.emplace_back(VkDescriptorBufferInfo {
		.buffer = buffer.buffer(),
		.offset = offset,
		.range = bufferSize
		});

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = descriptorType,
		.pBufferInfo = &bufferInfo
	};

	_writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::updateDescriptorSet(VkDescriptorSet descriptor) {
	for (VkWriteDescriptorSet& write : _writes) {
		write.dstSet = descriptor;
	}
	vkUpdateDescriptorSets(_device.handle(), static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);
	return *this;
}

DescriptorWriter& DescriptorWriter::clear() {
	_imageInfos.clear();
	_bufferInfos.clear();
	_writes.clear();
	return *this;
}



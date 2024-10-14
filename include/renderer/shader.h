#pragma once
#include "vulkan/vulkan.h"
#include "renderer/device.h"
#include <string>
#include <fstream>
#include <sstream>

class Shader {
public:
	// @brief Loads compiled SPIR-V shader code from specified filepath into a VkShaderModule
	//
	// @param filepath - Path to the compiled shader code
	// @param device - Vulkan device that handles shader module creation
	// @param outShaderModule - The VkShaderModule object to fill with the created shader module
	static void loadShaderModule(const std::string& filepath, const Device& device, VkShaderModule& outShaderModule);

	// @brief Populates a pipeline shader stage create info struct
	//
	// @param flags - Bit flags to enable in the create info struct
	// @param shader - Shader module to be added to the pipeline
	static VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader);
};
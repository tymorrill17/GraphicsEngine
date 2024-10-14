#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include "../utility/window.h"
#include "instance.h"
#include "queue_family.h"
#include "../NonCopyable.h"
#include <vector>
#include <set>

class Device : NonCopyable {
public:
	Device(const Instance& instance, Window& window, const std::vector<const char*>& extensions);
	~Device();

	// @brief Get the physicial device Vulkan object
	inline VkPhysicalDevice physicalDevice() const { return physDevice; }
	// @brief Get the physicial device properties
	inline VkPhysicalDeviceProperties physicalDeviceProperies() const { return physDeviceProperties; }
	// @brief Get the logical device Vulkan object
	inline VkDevice device() const { return logicalDevice; }
	// @brief Get the queue family indices object
	inline QueueFamilyIndices queueFamilyIndices() const { return indices; }
	// @brief Get the graphics queue Vulkan object
	inline VkQueue graphicsQueue() const { return graphQueue; }
	// @brief Get the present queue Vulkan object
	inline VkQueue presentQueue() const { return presQueue; }

	static VkPhysicalDeviceFeatures deviceFeatures;
	static VkPhysicalDeviceVulkan12Features features12;
	static VkPhysicalDeviceVulkan13Features features13;

private:
	// @brief Representation of the physical GPU
	VkPhysicalDevice physDevice;
	// @brief Properties of the chosen GPU
	VkPhysicalDeviceProperties physDeviceProperties;

	// @brief Logical representation of the physical device that the code can interact with
	VkDevice logicalDevice;

	const Instance& instance;
	Window& window;

	QueueFamilyIndices indices;
	VkQueue graphQueue; // Graphics queue
	VkQueue presQueue; // Present queue

	// @brief Verify that the selected physical device supports the requested extensions
	//
	// @param physicalDevice - The selected physical device to check
	// @param extensions - The requested device extensions
	// @return True if the physical device supports all of extensions. False otherwise
	static bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*>& extensions);
	
	// @brief Queries available physical devices and selects one based on whether or not it supports required device extensions
	//
	// @param instance - The current active instance of Vulkan
	// @param surface - The surface which the swapchain will present to
	// @param requiredExtensions - The device extensions that are required to present images to the screen
	// @return The selected VkPhysicalDevice object
	static VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);

	// @brief Checks whether the selected physical device has swapchain support
	//
	// @param physicalDevice - The selected physical device to check
	// @param surface - The surface which the swapchain will present to
	// @return True if the physical device has swapchain support. False otherwise 
	static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};
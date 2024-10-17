#pragma once

#include "instance.h"
#include "device.h"
#include "image.h"
#include "frame.h"
#include "queue_family.h"
#include "../NonCopyable.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <sstream>

class Device;
class Window;

// @brief Stores supported swapchain attributes
struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain : public NonCopyable {
public:
	Swapchain(const Device& device, const Window& window);
	~Swapchain();

	// @brief Get the actual VkSwapchainKHR handle
	inline VkSwapchainKHR handle() const { return _swapchain; }
	// @brief Get the swapchain image format
	inline VkFormat imageFormat() const { return _imageFormat; }
	// @brief Get the swapchain image view extent
	inline VkExtent2D extent() const { return _extent; }
	// @brief Get the number of swapchain images
	inline size_t imageCount() const { return _images.size(); }
	// @brief Get the swapchain support details
	inline SwapchainSupportDetails supportDetails() const { return _supportDetails; }
	// @brief Get the swapchain image at the specified index
	inline SwapchainImage& image(uint32_t index) { return _images[index]; }
	// @brief Get the number of frames in the swapchain
	inline uint32_t framesInFlight() const { return _framesInFlight; }

	void presentToScreen(VkQueue queue, Frame& frame, uint32_t imageIndex);

	// STATIC METHODS

	// @brief Queries swapchain support attributes
	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	// @brief Sets the swapchain extent based on the current window size
	static VkExtent2D setSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities, const Window& window);

private:
	const Device& _device;
	const Window& _window;

	// @brief Supported swapchain attributes
	SwapchainSupportDetails _supportDetails;
	VkSwapchainKHR _swapchain;
	VkSwapchainKHR _old_swapchain;

	// @brief Swapchain images
	std::vector<SwapchainImage> _images;

	// @brief Format of the swapchain images
	VkFormat _imageFormat;
	// @brief Extent of the swapchain image views
	VkExtent2D _extent;
	// @brief How many frames the swapchain contains and can be rendered in parallel
	uint32_t _framesInFlight;

	// @brief Creates a swapchain
	void createSwapchain();

	//@brief Creates swapchain image views. Images are created during swapchain creation. Called in createSwapchain()
	void createSwapchainImages();

	static VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};
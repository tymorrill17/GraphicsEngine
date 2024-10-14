#pragma once
#include "../logger/logger.h"
#include "../logger/debug_messenger.h"
#include "../utility/window.h"
#include "../utility/allocator.h"
#include "instance.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline_builder.h"
#include "frame.h"
#include "image.h"
#include "../NonCopyable.h"
#include "vulkan/vulkan.h"
#include <string>


class Engine : NonCopyable {
public:
	// @brief Construct and initialize the Vulkan 
	Engine(Window& window);

	// @brief Destroy engine instance and clean up allocations
	~Engine() = default;

	// @brief Renders the scene and presents it to the surface
	void render();


private:
	Logger* logger; // Debug logger
	Window window; // Main window to render to
	Instance instance;
	DebugMessenger debugMessenger; // Vulkan debug messenger callback for validation layers
	Device device; // Device object containing physical and logical devices
	Allocator allocator; // Allocator for buffers and images
	Swapchain swapchain; // The swapchain handles presenting images to the surface and thus to the window
	PipelineBuilder pipelineBuilder; // Pipeline builder object that abstracts and handles pipeline creation
	std::vector<Frame> frames; // Contains command buffers and sync objects for each frame in the swapchain
	uint32_t frameNumber; // Keeps track of the number of rendered frames
	AllocatedImage drawImage; // Image that gets rendered to then copied to the swapchain image

	Pipeline defaultPipeline;
	
	// @brief Gets the current frame by finding frameNumber % swapchain.framesInFlight
	Frame& getCurrentFrame();
};
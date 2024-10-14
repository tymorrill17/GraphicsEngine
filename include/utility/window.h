#pragma once
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include "../renderer/instance.h"
#include "logger/logger.h"
#include <string>
#include <vector>

// @brief Contains the window that will display the application
class Window {
public:
	// @brief Constructor for a window. Creates an SDL_Window object and initializes SDL
	// 
	// @param width - Width of the window
	// @param height - Height of the window
	// @param name - Name for the created window
	Window(glm::ivec2 dimensions, const std::string &name);

	// @brief Destroys the SDL_Window
	~Window();


	inline VkExtent2D extent() const { return windowExtent; }
	inline const struct SDL_Window* SDL_window() const { return _window; }
	inline struct SDL_Window* SDL_window() { return _window; }
	inline VkSurfaceKHR surface() const { return vkSurface; }
	inline bool shouldClose() const { return windowShouldClose; }

	// @brief Processes SDL inputs and delegates actions relating to window behavior
	void process_inputs();

	// @brief Get the required Vulkan extensions that the window system requires
	//
	// @param extensions - Populated with the required extensions
	static void getRequiredInstanceExtensions(std::vector<const char*>& extensions);

	// @brief Create the Vulkan surface that will communicate with the SDL window. Also sets the Instance pointer member in the Window object. Called in Device constructor
	//
	// @param instance - Vulkan instance to connect the surface with
	void create_surface(VkInstance instance);

	// @brief Destroys window surface. Meant to be called in Window destructor
	void destroy_surface();

private:
	struct SDL_Window* _window;
	VkExtent2D windowExtent;

	// @brief Name of the window
	std::string name;

	// @brief the Vulkan surface associated with the window
	VkSurfaceKHR vkSurface;

	// @brief VkInstance only needed to create a surface.
	VkInstance instance;

	bool windowShouldClose;

	// TODO: window resizing
};

#include "utility/window.h"
#include <sstream>

Window::Window(glm::ivec2 dimensions, const std::string& name) :
	windowExtent{ static_cast<uint32_t>(dimensions.x), static_cast<uint32_t>(dimensions.y) }, 
	instance(VK_NULL_HANDLE), 
	vkSurface(VK_NULL_HANDLE), 
	windowShouldClose(false) {

	Logger* logger = Logger::get_logger();
	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Create a window compatible with Vulkan surfaces
	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	struct SDL_Window* window = SDL_CreateWindow(
		name.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowExtent.width,
		windowExtent.height,
		windowFlags
	);

	if (window) {
		std::stringstream line;
		line << "Created a window titled \"" << name << "\" of size " << windowExtent.width << "x" << windowExtent.height << ".";
		logger->print(line.str());
	}
	else {
		throw std::runtime_error("Window creation failure!");
	}

	_window = window;
}

Window::~Window() {
	destroy_surface();
	SDL_DestroyWindow(_window);
}

void Window::getRequiredInstanceExtensions(std::vector<const char*>& extensions) {
	uint32_t sdlRequiredExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdlRequiredExtensionCount, nullptr);
	std::vector<const char*> sdlRequiredExtensions(sdlRequiredExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(nullptr, &sdlRequiredExtensionCount, sdlRequiredExtensions.data());

	extensions.assign(sdlRequiredExtensions.data(), sdlRequiredExtensions.data() + sdlRequiredExtensionCount);
}

void Window::process_inputs() {
	SDL_Event e;
	//Handle events on queue
	while (SDL_PollEvent(&e) != 0) {
		//close the window when user alt-f4s or clicks the X button			
		switch (e.type) {
		case SDL_QUIT:
			windowShouldClose = true;
			break;
		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_MINIMIZED:
				break;
			case SDL_WINDOWEVENT_RESTORED:
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

void Window::create_surface(VkInstance instance) {
	Logger* logger = Logger::get_logger();

	this->instance = instance;

	if (!SDL_Vulkan_CreateSurface(_window, instance, &vkSurface)) {
		throw std::runtime_error("Failed to create window surface!");
	}

	logger->print("Created SDL window surface for Vulkan");
}

void Window::destroy_surface() {
	if (vkSurface) {
		vkDestroySurfaceKHR(instance, vkSurface, nullptr);
	}
}
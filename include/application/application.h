#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/window.h"
#include "utility/camera.h"
#include "utility/timer.h"
#include "utility/gui.h"
#include "utility/input_manager.h"
#include "renderer/renderer.h"
#include "renderer/descriptor.h"
#include "renderer/buffer.h"
#include "physics/particle_system.h"
#include "physics/hand.h"
#include "systems/particle_render_system.h"
#include "systems/render_system.h"
#include "systems/gui_render_system.h"
#include <thread>
#include <chrono>

const uint32_t APPLICATION_WIDTH = 1920;
const uint32_t APPLICATION_HEIGHT = 1080;

const float coordinateScale = 4.5;

class Renderer;

// @brief The main program
class Application : public NonCopyable {
public:
	// @brief Constructor for a new application
	Application();

	// @brief The main running loop of the application
	void run();

private:
	// @brief The main window to display the application
	Window window;
	// @brief The graphics engine that manages drawing and rendering tasks
	Renderer renderer;
	InputManager inputManager;
};
#pragma once
#include "../renderer/engine.h"
#include "../logger/logger.h"
#include "../utility/window.h"
#include "../NonCopyable.h"

// @brief The main program
class Application : NonCopyable {
public:
	// @brief Constructor for a new application
	Application();

	// @brief The main running loop of the application
	void run() {
		main_loop();
	};

private:
	// @brief The main window to display the application
	Window window;
	// @brief The graphics engine that manages drawing and rendering tasks
	Engine engine;
	// @brief Debug logger
	Logger* logger;

	void main_loop();
};
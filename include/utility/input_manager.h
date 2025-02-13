#pragma once
#include "utility/window.h"
#include "utility/gui.h"
#include "logger/logger.h"

class InputManager {
public:
	InputManager(Window& window);

	// @brief Process SDL inputs and delegate action
	void processInputs();
private:
	Window& _window;
};
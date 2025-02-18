#pragma once
#include "utility/window.h"
#include "utility/gui.h"
#include "logger/logger.h"
#include "physics/hand.h"
#include <iostream>

class InputManager {
public:
	InputManager(Window& window);

	// @brief Process SDL inputs and delegate action
	void processInputs();

	glm::vec2 mousePosition() { return _mousePosition; }

private:
	Window& _window;

	void updateMousePosition(SDL_Event* e);

	glm::vec2 _mousePosition{ 0.0f, 0.0f };
};
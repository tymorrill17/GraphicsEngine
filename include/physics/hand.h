#pragma once
#include "glm/glm.hpp"

enum class HandAction {
	pushing,
	pulling,
	idle
};

class Hand {
public:
	Hand() {}
	Hand(float radius, float strengthFactor) :
		radius(radius),
		strengthFactor(strengthFactor) {}

	bool isInteracting() { return _action == HandAction::pushing || _action == HandAction::pulling; }
	void setAction(HandAction action) { _action = action; }
	HandAction action() { return _action; }

	void setPosition(glm::vec2 position) { _mousePosition = position; }
	glm::vec2 position() { return _mousePosition; }

	float radius{ 0.001f };
	float strengthFactor{ 0.0f };
private:
	HandAction _action{ HandAction::idle };
	glm::vec2 _mousePosition{ 0.0f, 0.0f };
};
#pragma once

#include "NonCopyable.h"
#include "renderer/command.h"

class RenderSystem : public NonCopyable {
public:
	virtual void render(Command& cmd) = 0;
};
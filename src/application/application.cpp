#include "application/application.h"

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

Application::Application() : 
	window({ WIDTH,HEIGHT }, "VulkanEngineV2"),
	engine(window) {

	// Get static logger
	logger = Logger::get_logger();
}

void Application::main_loop() {

	while (!window.shouldClose()) {
		window.process_inputs();
		engine.render();
	}
	logger->print("Shutting Down... Bye Bye!");
}

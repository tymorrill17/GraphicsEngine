#include "application/application.h"

Application::Application() : 
	window({ APPLICATION_WIDTH, APPLICATION_HEIGHT }, "VulkanEngineV2"),
	engine(window) {

	// Get static logger
	logger = Logger::get_logger();
}

void Application::main_loop() {

	while (!window.shouldClose()) {
		window.process_inputs();
		if (window.pauseRendering()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		engine.render();
		engine.resizeCallback();
	}
	logger->print("Shutting Down... Bye Bye!");
}

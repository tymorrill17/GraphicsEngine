#include "application/application.h"
#include "logger/logger.h"
#include <atomic>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
	static Logger& logger = Logger::getLogger(); // Initialize logger
	static Timer& timer = Timer::getTimer(); // Initialize timer
#ifdef _DEBUG
	logger.activate(); // If debug mode, activate logger and print to the console
#endif

	// Application is the controller class
	Application* app = new Application();
	try {
		app->run();
	}
	catch (const std::exception& e) {
		std::stringstream line;
		line << "Caught exception: " << e.what();
		logger.print(line.str());
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
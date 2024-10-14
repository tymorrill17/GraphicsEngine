#include "application/application.h"
#include "logger/logger.h"
#include <atomic>
#include <thread>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
	// Set up logger
	Logger* logger = Logger::get_logger();
#ifdef _DEBUG
	logger->set_active(true);
#endif

	// Application is the controller class
	Application* app = new Application();
	try {
		app->run();
	}
	catch (const std::exception& e) {
		std::stringstream line;
		line << "Caught exception: " << e.what();
		logger->print(line.str());
		Logger::shutdown();
		return EXIT_FAILURE;
	}
	

	// Deletes the logger
	Logger::shutdown();
	return EXIT_SUCCESS;
}
#include "application/application.h"
#include "logger/logger.h"
#include <atomic>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
	// Application is the controller class
	Application* app = new Application();
	try {
		app->run();
	}
	catch (const std::exception& e) {
		std::stringstream line;
		line << "Caught exception: " << e.what();
		std::cout << line.str() << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
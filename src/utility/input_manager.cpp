#include "utility/input_manager.h"

InputManager::InputManager(Window& window) :
	_window(window) {}

void InputManager::processInputs() {
	static Logger& logger = Logger::getLogger();
	static Gui& _gui = Gui::getGui();

	SDL_Event sdl_event;
	//Handle events on queue
	while (SDL_PollEvent(&sdl_event) != 0) {
		
		// Let the gui backend handle its inputs
		_gui.processInputs(&sdl_event);

		//close the window when user alt-f4s or clicks the X button			
		switch (sdl_event.type) {
		case SDL_QUIT:
			_window.closeWindow();
			break;
		case SDL_WINDOWEVENT:
			switch (sdl_event.window.event) {
			case SDL_WINDOWEVENT_MINIMIZED:
				_window.setPauseRendering(true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				_window.setPauseRendering(false);
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (sdl_event.key.keysym.sym) {
			case SDLK_F11:
				if (_window.isFullscreen()) {
					SDL_SetWindowFullscreen(_window.SDL_window(), 0);
					_window.setFullscreen(false);
				}
				else {
					SDL_SetWindowFullscreen(_window.SDL_window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
					_window.setFullscreen(true);
				}
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:
			switch (sdl_event.key.keysym.sym) {
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}
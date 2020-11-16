#include "SDL_manager.h"
#include "SDL.h"
#include <exception>
#include <stdexcept>
SdlManager::SdlManager() {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		throw std::runtime_error{ SDL_GetError() };
	}
}

SdlManager::~SdlManager() {
	SDL_Quit();
}

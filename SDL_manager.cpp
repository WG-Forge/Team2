#include "SDL_manager.h"
#include <SDL.h>
#include <SDL_net.h>
#include <exception>
#include <stdexcept>

SdlManager::SdlManager() {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		throw std::runtime_error{ SDL_GetError() };
	}
	if (SDLNet_Init() == -1)
	{
		SDL_Quit();
		throw std::runtime_error{ SDLNet_GetError() };
	}
}

SdlManager::~SdlManager() {
	SDLNet_Quit();
	SDL_Quit();
}

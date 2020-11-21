#include <SDL.h>
#include "SDL_manager.h"
#include "SDL_window.h"
#include "GameWorld.h"
#include <chrono>
#include <thread>
#include <iostream>

constexpr int frameTime = 33;

int main(int argC, char** argV) {
	try {
		SdlManager manager{};
		SdlWindow window{ "graph demo", 800, 600 };
		TextureManager textureManager = window.CreateTextureManager();
		GameWorld world{ "team2", textureManager };
		bool toExit = false;
		auto lastUpdateTime = std::chrono::high_resolution_clock::now();

		while (!(toExit = window.HasCloseRequest())) {
			window.SetDrawColor(0, 0, 0);
			window.Clear();
			world.Update();
			world.Draw(window);

			auto currentTime = std::chrono::high_resolution_clock::now();
			if (currentTime - lastUpdateTime < std::chrono::milliseconds{ frameTime }) {
				std::this_thread::sleep_for(std::chrono::milliseconds{ frameTime } - (currentTime - lastUpdateTime));
			}
			lastUpdateTime = std::chrono::high_resolution_clock::now();
			window.Update();
		}
	}
	catch (...) {
		std::cout << "Whoops..." << std::endl;
		std::cout << "Something went wrong" << std::endl;
	}

	return 0;
}
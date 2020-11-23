#include <SDL.h>
#include "SDL_manager.h"
#include "SDL_window.h"
#include "GameWorld.h"
#include <chrono>
#include <thread>
#include <iostream>

constexpr int frameTime = 33;
constexpr double stableThreshold = 20.0;

int main(int argC, char** argV) {
	try {
		SdlManager manager{};
		SdlWindow window{ "graph demo", 1280, 960 };
		TextureManager textureManager = window.CreateTextureManager();
		GameWorld world{ "team2", textureManager };
		bool toExit = false;
		auto lastUpdateTime = std::chrono::high_resolution_clock::now();

		std::thread updateThread{ [&world, &toExit]() {
			while (!toExit) {
				world.Update();
			}
		} };

		std::thread forceThread{ [&world, &toExit]() {
			double change = stableThreshold;
			while (!toExit && change >= stableThreshold) {
				change = world.ApplyForce();
			}
		} };

		while (!(toExit = window.HasCloseRequest())) {
			window.SetDrawColor(35, 23, 0);
			window.Clear();
			world.Draw(window);

			auto currentTime = std::chrono::high_resolution_clock::now();
			if (currentTime - lastUpdateTime < std::chrono::milliseconds{ frameTime }) {
				std::this_thread::sleep_for(std::chrono::milliseconds{ frameTime } - (currentTime - lastUpdateTime));
			}
			lastUpdateTime = std::chrono::high_resolution_clock::now();
			window.Update();
		}
		window.Close();
		updateThread.join();
		forceThread.join();
	}
	catch (const std::runtime_error& error) {
		std::cout << "got unexpected error: " << error.what() << std::endl;
	}
	catch (...) {
		std::cout << "Whoops..." << std::endl;
		std::cout << "Something went wrong" << std::endl;
	}

	return 0;
}
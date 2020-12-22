#include <SDL.h>
#include "SDL_manager.h"
#include "SDL_window.h"
#include "GameWorld.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <random>

constexpr int frameTime = 33;
constexpr double stableThreshold = 20.0;

std::string generateRandomString();

int main(int argC, char** argV) {
	try {
		SdlManager manager{};
		SdlWindow window{ "graph demo", 1280, 960 };
		TextureManager textureManager = window.CreateTextureManager();
		GameWorld world{ generateRandomString(), textureManager };
		bool toExit = false;
		auto lastUpdateTime = std::chrono::high_resolution_clock::now();

		std::thread updateThread{ [&world, &toExit]() {
			try {
				while (!toExit) {
#ifndef _DEBUG
					auto before = std::chrono::high_resolution_clock::now();
#endif
					world.MakeMove();
					world.Update();
#ifndef _DEBUG
					auto after = std::chrono::high_resolution_clock::now();
					std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << "ms" << std::endl;
#endif
				}
			}
			catch (const std::runtime_error& error) {
				std::cout << "got unexpected error: " << error.what() << std::endl;
			}
			catch (...) {
				std::cout << "Whoops..." << std::endl;
				std::cout << "Something went wrong" << std::endl;
			}
		} };

		while (!(toExit = window.HasCloseRequest())) {
			window.SetDrawColor(90, 90, 90);
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

std::string generateRandomString()
{
	std::random_device rnd{};
	std::mt19937 random{ rnd() };
	std::string result;
	for (int i = 0; i < 16; ++i) {
		result += 'A' + random() % ('Z' - 'A' + 1);
	}
	return result;
}

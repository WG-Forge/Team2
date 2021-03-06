#include <SDL.h>
#include "SDL_manager.h"
#include "SDL_window.h"
#include "GameWorld.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <random>

constexpr int frameTime = 33;
constexpr int numTurns = 500;

std::string generateRandomString();

int main(int argC, char** argV) {
	std::string gameName;
	std::string name;
	std::string buf;
	int playerCount = 1;
	std::cout << "Enter player count or leave blank for default: ";
	std::getline(std::cin, buf);
	if (buf.empty()) {
		std::cout << "1 assumed" << std::endl;
		playerCount = 1;
	}
	else if (buf[0] >= '1' && buf[0] <= '4' && buf.size() == 1) {
		playerCount = buf[0] - '0';
	}
	else {
		std::cout << "illegal value, 1 assumed" << std::endl;
		playerCount = 1;
	}
	std::cout << "Enter player name or leave blank for default: ";
	std::getline(std::cin, name);
	if (name.empty()) {
		std::cout << "'team 2' assumed" << std::endl;
		name = "team 2";
	}
	if (playerCount > 1) {
		std::cout << playerCount << " players game;" << std::endl;
		std::cout << "Enter game name or leave blank for default: ";
		std::getline(std::cin, gameName);
	}
	try {
		SdlManager manager{};
		SdlWindow window{ "graph demo", 1280, 960 };
		TextureManager textureManager = window.CreateTextureManager();
		GameWorld world{ name, gameName, playerCount, numTurns, textureManager };
		bool toExit = false;
		auto lastUpdateTime = std::chrono::high_resolution_clock::now();

		std::thread updateThread{ [&world, &toExit]() {
			try {
#ifndef _DEBUG
				int turn = 0;
#endif
				while (!toExit) {
#ifndef _DEBUG
					auto before = std::chrono::high_resolution_clock::now();
#endif
					try {
						world.MakeMove();
						world.Update();
					}
					catch (const std::runtime_error& e) {
#ifndef _DEBUG
						std::cout << e.what() << std::endl;
						--turn;
						std::cout << "score: " << world.GetScore() << std::endl;
#endif
					}
#ifndef _DEBUG
					auto after = std::chrono::high_resolution_clock::now();
					std::cout << "turn " << ++turn << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << "ms; ";
					std::cout << "score: " << world.GetScore() << std::endl;
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

#include <SDL.h>
#include "SDL_manager.h"
#include "SDL_window.h"
#include "graph.h"
#include <chrono>
#include <thread>

constexpr int frameTime = 33;
constexpr double stableThreshold = 20.0;

int main(int argC, char** argV) {
	SdlManager manager{};
	SdlWindow window{"graph demo", 800, 600};
	std::string filename;
	if (argC < 2) {
		filename = "JSON_test_files/big_graph.json";
	}
	else {
		filename = argV[1];
	}
	Graph demoGraph{ filename };
	bool toExit = false;
	auto lastUpdateTime = std::chrono::high_resolution_clock::now();
	std::thread graphCalcThread{ [&demoGraph, &toExit]() {
		double change = stableThreshold;
		while (!toExit && change >= stableThreshold) {
			change = demoGraph.ApplyForce();
		} 
	} 
	};

	while (!(toExit = window.HasCloseRequest())) {
		window.SetDrawColor(0, 0, 0);
		window.Clear();
		demoGraph.Draw(window);

		auto currentTime = std::chrono::high_resolution_clock::now();
		if (currentTime - lastUpdateTime < std::chrono::milliseconds{ frameTime }) {
			std::this_thread::sleep_for(std::chrono::milliseconds{ frameTime } - (currentTime - lastUpdateTime));
		}
		lastUpdateTime = std::chrono::high_resolution_clock::now();
		window.Update();
	}

	graphCalcThread.join();

	return 0;
}
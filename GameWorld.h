#pragma once
#include "Map.h"
#include "ServerConnection.h"

class GameWorld {
private:
	class Train {
	private:
		size_t idx;
		size_t lineIdx;
		double position;	// may become int
		double speed;		// may become int
	public:
		Train(size_t idx, size_t lineIdx, double position, double speed) :idx{ idx }, lineIdx{ lineIdx }, position{ position }, speed{ speed }{}
	};
	ServerConnection connection;
	Map map;
	std::vector<Train> trains;
	std::map<size_t, size_t> trainIdxConverter;
public:
	GameWorld(const std::string& playerName);
	void Update(); // updates map and trains
	void Draw(SdlWindow& window);
private:
	void UpdateTrains(const std::string& jsonData);
	void DrawTrains(SdlWindow& window);
};


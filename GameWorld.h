#pragma once
#include "Map.h"
#include "ServerConnection.h"

class GameWorld {
private:
	class Train {
		int idx;
		int lineIdx;
		double position;	// may become int
		double speed;		// may become int
	};
	ServerConnection connection;
	Map map;
	std::vector<Train> trains;
public:
	GameWorld(const std::string& playerName);
	void Update(); // updates map and trains
	void Draw(SdlWindow& window);
private:
	void UpdateTrains(const std::string& jsonData);
	void DrawTrains(SdlWindow& window);
};


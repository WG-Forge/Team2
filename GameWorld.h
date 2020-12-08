#pragma once
#include "Map.h"
#include "ServerConnection.h"

class GameWorld {
private:
	class Train {
	public:
		size_t idx;
		size_t lineIdx;
		size_t trueLineIdx;
		int level;
		int nextLevelPrice;
		double position;	// may become int
		double truePosition;
		double speed;		// may become int
		double capacity;
		double load;
		int cooldown;
		std::string owner;
		Train(size_t idx, size_t lineIdx, double position, double speed) : idx{ idx }, lineIdx{ lineIdx }, trueLineIdx{ lineIdx }, position{ position }, truePosition{ position }, speed{ speed } {}
	};
	bool allTrainsUpgraded = false;
	ServerConnection connection;
	TextureManager& textureManager;
	Map map;
	std::vector<Train> trains;
	std::map<size_t, size_t> trainIdxConverter;
	std::unordered_set<std::pair<int, int>> edgesBlackList;
	std::unordered_set<int> takenPosts;
	std::unordered_map<int, int> trainsTargets;
public:
	GameWorld(const std::string& playerName, TextureManager& textureManager);
	void Update(); // updates map and trains
	void Draw(SdlWindow& window);
	void MakeMove();
private:
	void MoveTrains();
	void MoveTrain(Train& train);
	void MoveTrainTo(Train& train, int to);
	void UpdateTrains(const std::string& jsonData);
	void DrawTrains(SdlWindow& window);
};


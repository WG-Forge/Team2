#pragma once
#include "Map.h"
#include "ServerConnection.h"

class GameWorld {
private:

	using TrainMoveData = std::tuple<int, int, int>;

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

	std::vector<ServerConnection> helpConnections;
	bool everythingUpgraded = false;
	ServerConnection connection;
	TextureManager& textureManager;
	Map map;
	std::vector<Train> trains;
	std::map<size_t, size_t> trainIdxConverter;
	std::unordered_set<std::pair<int, int>> edgesBlackList;
	std::unordered_set<int> pointBlackList;
	std::unordered_set<int> takenPosts;
	std::unordered_set<uint64_t> takenPositions;
	std::unordered_set<uint64_t> whitePositions;
	std::unordered_map<int, int> trainsTargets;
public:
	GameWorld(const std::string& playerName, TextureManager& textureManager);
	void Update(); // updates map and trains
	void Draw(SdlWindow& window);
	void MakeMove();
private:
	void Update(const std::string& jsonData);
	void MoveTrains();
	std::optional<TrainMoveData> MoveTrain(Train& train);
	std::optional<TrainMoveData> MoveTrainTo(Train& train, int to);
	void UpdateTrains(const std::string& jsonData);
	void DrawTrains(SdlWindow& window);
	uint64_t GetPosition(int vertex);
	uint64_t GetPosition(int lineIdx, double position);
	uint64_t GetNextPosition(int lineIdx, double position, int speed);
	uint64_t GetNextPosition(int prevLineIdx, int lineIdx, double position, int speed);
};


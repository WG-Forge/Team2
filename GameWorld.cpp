#include "GameWorld.h"
#include "json.h"
#include <sstream>
#include <unordered_set>

#define PATHFINDING_DEBUG

#ifdef _DEBUG
#ifdef PATHFINDING_DEBUG
#define _PATHFINDING_DEBUG
#endif
#endif

void makeMoveRequest(int lineIdx, int speed, int trainIdx, ServerConnection& connection) {
	try {
		connection.MoveTrain(lineIdx, speed, trainIdx);
	}
	catch (std::runtime_error& error) {
		std::cout << error.what() << std::endl;
		throw;
	}
}

GameWorld::GameWorld(const std::string& playerName, TextureManager& textureManager) : connection{ playerName },	textureManager{textureManager},
		map{ connection.GetMapStaticObjects(), connection.GetMapCoordinates(), connection.GetMapDynamicObjects(), textureManager } {
	UpdateTrains(connection.GetMapDynamicObjects());
}
 
void GameWorld::Update() {
	std::string dynamicObjects = connection.GetMapDynamicObjects();
	map.Update(dynamicObjects);
	UpdateTrains(dynamicObjects);
}

void GameWorld::Draw(SdlWindow& window) {
	map.Draw(window);
	DrawTrains(window);
}

void GameWorld::MoveTrains() {
	std::vector<std::thread> helpThreads;
	std::vector<TrainMoveData> moveData;
	int trainsCount = 0;
	for (auto& i : trains) {
		if (i.cooldown != 0) {
			continue;
		}
		if (i.owner != connection.GetPlayerIdx()) {
			continue;
		}
		++trainsCount;
		moveData.push_back(MoveTrain(i));
	}
	while (helpConnections.size() < trainsCount) {
		helpConnections.emplace_back(connection.GetLogin(), connection.GetPassword());
	}
	for (int i = 0; i < trainsCount; ++i) {
		helpThreads.emplace_back(makeMoveRequest, std::get<0>(moveData[i]), std::get<1>(moveData[i]), std::get<2>(moveData[i]), std::ref(helpConnections[i]));
	}
	for (int i = 0; i < trainsCount; ++i) {
		helpThreads[i].join();
	}
}

GameWorld::TrainMoveData GameWorld::MoveTrain(Train& train) {
	int to;
	int from;
	double edgeLength = map.GetEdgeLength(train.trueLineIdx);
	auto [first, second] = map.GetEdgeVertices(train.trueLineIdx);

	if (train.lineIdx != train.trueLineIdx) {
		auto [firstOld, secondOld] = map.GetEdgeVertices(train.lineIdx);
		if (first == firstOld) {
			train.truePosition = 0.0;
		}
		else if (first == secondOld) {
			train.truePosition = 0.0;
		}
		else if (second == firstOld) {
			train.truePosition = edgeLength;
		}
		else if (second == secondOld) {
			train.truePosition = edgeLength;
		}
	}

	if (train.truePosition >= edgeLength / 2) {
		from = second;
	}
	else {
		from = first;
	}

	if (train.load == 0) {
		if (trainsTargets.find(train.idx) != trainsTargets.end()) {
			takenPosts.erase(trainsTargets[train.idx]);
		}
		to = map.GetBestMarket(from, map.TranslateVertexIdx(connection.GetHomeIdx()), train.capacity - train.load, 0, takenPosts);
		takenPosts.insert(to);
		trainsTargets[train.idx] = to;
	}
	else {
		if (trainsTargets.find(train.idx) != trainsTargets.end()) {
			takenPosts.erase(trainsTargets[train.idx]);
			trainsTargets.erase(train.idx);
		}
		to = map.TranslateVertexIdx(connection.GetHomeIdx());
	}
	return MoveTrainTo(train, to);
}

GameWorld::TrainMoveData GameWorld::MoveTrainTo(Train& train, int to) {
	auto [first, second] = map.GetEdgeVertices(train.trueLineIdx);
	if (train.truePosition == 0.0) {
#ifdef _PATHFINDING_DEBUG
		std::cout << "from: " << first;
		std::cout << "; to: " << to;
#endif
		to = *map.GetNextOnPath(first, to, map.GetStorages());
#ifdef _PATHFINDING_DEBUG
		std::cout << "; via: " << to << std::endl;
#endif
		if (to == first) {
			return { train.trueLineIdx, 0, train.idx };
		}
		else if (to == second) {
			return { train.trueLineIdx, 1, train.idx };
		}
		else {
			train.trueLineIdx = map.GetEdgeIdx(first, to);
			return MoveTrain(train);
		}
	}
	else if (train.truePosition == map.GetEdgeLength(train.trueLineIdx)) {
#ifdef _PATHFINDING_DEBUG
		std::cout << "from: " << second;
		std::cout << "; to: " << to;
#endif
		to = *map.GetNextOnPath(second, to, map.GetStorages());
#ifdef _PATHFINDING_DEBUG
		std::cout << "; via: " << to << std::endl;
#endif
		if (to == second) {
			return { train.trueLineIdx, 0, train.idx };
		}
		else if (to == first) {
			return { train.trueLineIdx, -1, train.idx };
		}
		else {
			train.trueLineIdx = map.GetEdgeIdx(second, to);
			return MoveTrain(train);
		}
	}
	else {
		if (train.speed > 0) {
			return { train.trueLineIdx, 1, train.idx };
		}
		else if (train.speed < 0){
			return { train.trueLineIdx, -1, train.idx };
		}
	}
}

void GameWorld::MakeMove() {
	takenPosts.clear();
#ifdef _PATHFINDING_DEBUG
	std::cout << "taken posts: ";
#endif
	for (const auto& [idx, target] : trainsTargets) {
#ifdef _PATHFINDING_DEBUG
		std::cout << target << "; ";
#endif
		takenPosts.insert(target);
	}
#ifdef _PATHFINDING_DEBUG
	std::cout << std::endl;
#endif
	MoveTrains();
	connection.EndTurn();
}

void GameWorld::UpdateTrains(const std::string& jsonData) {
	std::stringstream ss;
	ss << jsonData;
	Json::Document doc = Json::Load(ss);
	auto nodeMap = doc.GetRoot().AsMap();
	trainIdxConverter.clear();
	trains.clear();
	edgesBlackList.clear();
	trains.reserve(nodeMap["trains"].AsArray().size());
	allTrainsUpgraded = true;
	for (const auto& node : nodeMap["trains"].AsArray()) {
		auto trainMap = node.AsMap();
		trainIdxConverter[trainMap["idx"].AsInt()] = trains.size();
		trains.emplace_back( static_cast<size_t>(trainMap["idx"].AsInt()), static_cast<size_t>(trainMap["line_idx"].AsInt()), 
			trainMap["position"].AsDouble(), trainMap["speed"].AsDouble());
		Train& train = *trains.rbegin();
		train.capacity = trainMap["goods_capacity"].AsDouble();
		train.load = trainMap["goods"].AsDouble();
		train.owner = trainMap["player_idx"].AsString();
		train.cooldown = trainMap["cooldown"].AsInt();
		train.level = trainMap["level"].AsInt();
		if (train.owner == connection.GetPlayerIdx()) {
			if (train.level < 3) {
				train.nextLevelPrice = trainMap["next_level_price"].AsInt();
				allTrainsUpgraded = false;
			}
		}
		else {

		}

		if (train.speed == 1.0) {
			edgesBlackList.insert(map.GetEdgeVertices(train.lineIdx));
		}
		else if (train.speed == -1.0) {
			std::pair<int, int> edge = map.GetEdgeVertices(train.lineIdx);
			std::swap(edge.first, edge.second);
			edgesBlackList.insert(edge);
		}
		else {
			std::pair<int, int> edge = map.GetEdgeVertices(train.lineIdx);
			edgesBlackList.insert(edge);
			std::swap(edge.first, edge.second);
			edgesBlackList.insert(edge);
		}
	}
}

void GameWorld::DrawTrains(SdlWindow& window) {
	for (const auto& i : trains) {
		double from, to;
		std::pair<int, int> vertices = map.GetEdgeVertices(i.lineIdx);
		std::pair<double, double> a = map.GetPointCoord(vertices.first);
		std::pair<double, double> b = map.GetPointCoord(vertices.second);
		if (i.speed >= 0) {
			from = a.first;
			to = b.first;
		}
		else {
			from = b.first;
			to = a.first;
		}

		bool toMirror;
		if (from > to) {
			toMirror = true;
		}
		else {
			toMirror = false;
		}
		double edgeLength = map.GetEdgeLength(i.lineIdx);
		double x = a.first + (b.first - a.first) * (i.position / edgeLength);
		double y = a.second + (b.second - a.second) * (i.position / edgeLength);
		SDL_Texture* texture;
		if (i.speed != 0) {
			texture = textureManager["assets\\train.png"];
		}
		else {
			texture = textureManager["assets\\train_no_smoke.png"];
		}
		window.DrawTexture(x, y, 40, 40, texture, 0.0, toMirror);
	}
}

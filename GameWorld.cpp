#include "GameWorld.h"
#include <sstream>
#include "json.h"
#include <unordered_set>

std::unordered_set<int> taken;

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
	taken.clear();
	for (auto& i : trains) {
		if (i.cooldown != 0) {
			continue;
		}
		if (i.owner != connection.GetPlayerIdx()) {
			continue;
		}
		MoveTrain(i);
	}
}

void GameWorld::MoveTrain(Train& train) {
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

	if (train.load < train.capacity) {
		if (allTrainsUpgraded) {
			to = map.GetBestMarket(from, map.TranslateVertexIdx(connection.GetHomeIdx()), train.capacity - train.load, 0, taken);
		}
		else {
			to = map.GetBestStorage(from, map.TranslateVertexIdx(connection.GetHomeIdx()), train.capacity - train.load, 0, taken);
		}
		taken.insert(to);
	}
	else {
		to = map.TranslateVertexIdx(connection.GetHomeIdx());
	}
	MoveTrainTo(train, to);
}

void GameWorld::MoveTrainTo(Train& train, int to) {
	auto [first, second] = map.GetEdgeVertices(train.trueLineIdx);
	if (train.truePosition == 0.0) {
		to = map.GetNextOnPath(first, to);
		if (to == first) {
			connection.MoveTrain(train.trueLineIdx, 0, train.idx);
		}
		else if (to == second) {
			connection.MoveTrain(train.trueLineIdx, 1, train.idx);
		}
		else {
			connection.MoveTrain(map.GetEdgeIdx(first, to), -1, train.idx);
			train.trueLineIdx = map.GetEdgeIdx(first, to);
			//MoveTrain(train);
		}
	}
	else if (train.truePosition == map.GetEdgeLength(train.trueLineIdx)) {
		to = map.GetNextOnPath(second, to);
		if (to == second) {
			connection.MoveTrain(train.trueLineIdx, 0, train.idx);
		}
		else if (to == first) {
			connection.MoveTrain(train.trueLineIdx, -1, train.idx);
		}
		else {
			connection.MoveTrain(map.GetEdgeIdx(second, to), 1, train.idx);
			train.trueLineIdx = map.GetEdgeIdx(second, to);
			//MoveTrain(train);
		}
	}
	else {
		to = map.GetNextOnPath(first, to);
		if (to == second) {
			connection.MoveTrain(train.trueLineIdx, 1, train.idx);
		}
		else {
			connection.MoveTrain(train.trueLineIdx, -1, train.idx);
		}
	}
}

void GameWorld::MakeMove() {
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
	trains.reserve(nodeMap["trains"].AsArray().size());
	allTrainsUpgraded = true;
	for (const auto& node : nodeMap["trains"].AsArray()) {
		auto trainMap = node.AsMap();
		trainIdxConverter[trainMap["idx"].AsInt()] = trains.size();
		trains.emplace_back( static_cast<size_t>(trainMap["idx"].AsInt()), static_cast<size_t>(trainMap["line_idx"].AsInt()), 
			trainMap["position"].AsDouble(), trainMap["speed"].AsDouble());
		trains[trains.size() - 1].capacity = trainMap["goods_capacity"].AsDouble();
		trains[trains.size() - 1].load = trainMap["goods"].AsDouble();
		trains[trains.size() - 1].owner = trainMap["player_idx"].AsString();
		trains[trains.size() - 1].cooldown = trainMap["cooldown"].AsInt();
		trains[trains.size() - 1].level = trainMap["level"].AsInt();
		if (trains[trains.size() - 1].owner == connection.GetPlayerIdx()) {
			if (trains[trains.size() - 1].level < 3) {
				allTrainsUpgraded = false;
			}
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

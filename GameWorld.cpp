#include "GameWorld.h"
#include <sstream>
#include "json.h"

GameWorld::GameWorld(const std::string& playerName, TextureManager& textureManager) : connection{ playerName },	textureManager{textureManager},
		map{ connection.GetMapStaticObjects(), connection.GetMapCoordinates(), connection.GetMapDynamicObjects(), textureManager } {
	UpdateTrains(connection.GetMapDynamicObjects());
}

double GameWorld::ApplyForce()
{
	double res = map.ApplyForce();
	return res;
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

void GameWorld::TestTrainMove() {
	static bool dir = true;
	if (trains.empty()) {
		return;
	}
	Train c_tr = trains[0];
	std::pair<int, int> path = map.GetEdgeVertices(c_tr.lineIdx);
	if (dir) {
		if (c_tr.position == map.GetEdgeLength(c_tr.lineIdx)) {
			connection.MoveTrain(c_tr.lineIdx, -1, c_tr.idx);
			dir = !dir;
		}
		else {
			connection.MoveTrain(c_tr.lineIdx, 1, c_tr.idx);
		}
	}
	else {
		if (c_tr.position == 0) {
			connection.MoveTrain(c_tr.lineIdx, 1, c_tr.idx);
			dir = !dir;
		}
		else {
			connection.MoveTrain(c_tr.lineIdx, -1, c_tr.idx);
		}
	}
}

void GameWorld::MakeMove() {
	TestTrainMove();
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
	for (const auto& node : nodeMap["trains"].AsArray()) {
		auto trainMap = node.AsMap();
		trainIdxConverter[trainMap["idx"].AsInt()] = trains.size();
		trains.emplace_back( static_cast<size_t>(trainMap["idx"].AsInt()), static_cast<size_t>(trainMap["line_idx"].AsInt()), 
			trainMap["position"].AsDouble(), trainMap["speed"].AsDouble());
	}
}

void GameWorld::DrawTrains(SdlWindow& window) {
	for (const auto& i : trains) {
		std::pair<int, int> vertices = map.GetEdgeVertices(i.lineIdx);
		std::pair<double, double> a = map.GetPointCoord(vertices.first);
		std::pair<double, double> b = map.GetPointCoord(vertices.second);
		double edgeLength = map.GetEdgeLength(i.lineIdx);
		double x = a.first + (b.first - a.first) * (i.position / edgeLength);
		double y = a.second + (b.second - a.second) * (i.position / edgeLength);
		window.DrawTexture(x, y, 40, 40, textureManager["assets\\train.png"]);
	}
}

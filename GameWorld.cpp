#include "GameWorld.h"
#include <sstream>
#include "json.h"

GameWorld::GameWorld(const std::string& playerName):connection(playerName), map(connection.GetMapStaticObjects(), connection.GetMapCoordinates(), connection.GetMapDynamicObjects()){
	UpdateTrains(connection.GetMapDynamicObjects());
}

void GameWorld::Update()
{
	std::string dynamicObjects = connection.GetMapDynamicObjects();
	map.Update(dynamicObjects);
	UpdateTrains(dynamicObjects);
}

void GameWorld::Draw(SdlWindow& window)
{
	map.Draw(window);
}

void GameWorld::UpdateTrains(const std::string& jsonData)
{
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

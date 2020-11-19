#include "Map.h"

Map::Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData):Graph(jsonStructureData, jsonCoordinatesData) {
	Update(jsonDynamicData);
}

void Map::Update(const std::string& jsonDynamicData)
{
}

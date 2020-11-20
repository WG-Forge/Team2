#include "Map.h"
#include "json.h"
#include <sstream>

Map::Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData):Graph(jsonStructureData, jsonCoordinatesData) {
	posts.resize(adjacencyList.size(), { Post::PostTypes::NONE, 0, "", 0 });
	Update(jsonDynamicData);
}

void Map::Update(const std::string& jsonDynamicData)
{
	std::stringstream ss;
	ss << jsonDynamicData;
	Json::Document doc = Json::Load(ss);
	auto nodeMap = doc.GetRoot().AsMap();
	for (const auto& node : nodeMap["posts"].AsArray()) {
		auto postMap = node.AsMap();
		posts[TranslateVertexIdx(postMap["post_idx"].AsInt())] = { static_cast<Post::PostTypes>(postMap["type"].AsInt()), 
			static_cast<size_t>(postMap["idx"].AsInt()),
			postMap["name"].AsString(), static_cast<size_t>(postMap["pointIdx"].AsInt()) };
	}

}

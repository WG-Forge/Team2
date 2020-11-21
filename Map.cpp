#include "Map.h"
#include "json.h"
#include <sstream>

Map::Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData):Graph(jsonStructureData, jsonCoordinatesData) {
	posts.resize(adjacencyList.size(), { Post::PostTypes::NONE, 0, "", 0 });
	Update(jsonDynamicData);
}

void Map::Draw(SdlWindow& window) {
	DrawEdges(window);
	for (int i = 0; i < posts.size(); ++i) {
		if (posts[i].type == Post::PostTypes::NONE) {
			continue;
		}
		switch (posts[i].type)
		{
		case Post::PostTypes::TOWN:
			window.SetDrawColor(255, 0, 0);
			break;
		case Post::PostTypes::MARKET:
			window.SetDrawColor(0, 255, 0);
			break;
		case Post::PostTypes::STORAGE:
			window.SetDrawColor(0, 0, 255);
			break;
		}
		window.DrawRectangle(adjacencyList[i].point.x - 10, adjacencyList[i].point.y - 10, adjacencyList[i].point.x + 10, adjacencyList[i].point.y + 10);
	}
}

void Map::Update(const std::string& jsonDynamicData) {
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

#include "Map.h"
#include "json.h"
#include <sstream>

Map::Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData, TextureManager& textureManager) : 
		Graph{ jsonStructureData, jsonCoordinatesData },
		textureManager{ textureManager } {
	posts.resize(adjacencyList.size(), { Post::PostTypes::NONE, 0, "", 0 });
	Update(jsonDynamicData);
}

void Map::Draw(SdlWindow& window) {
	DrawEdges(window);
	for (int i = 0; i < posts.size(); ++i) {
		SDL_Texture* texture = nullptr;
		switch (posts[i].type) {
		case Post::PostTypes::NONE:
			texture = textureManager["assets//none.png"];
			break;
		case Post::PostTypes::TOWN:
			texture = textureManager["assets//town.png"];
			break;
		case Post::PostTypes::MARKET:
			texture = textureManager["assets//market.png"];
			break;
		case Post::PostTypes::STORAGE:
			texture = textureManager["assets//storage.png"];
			break;
		}
		window.DrawTexture(adjacencyList[i].point.x, adjacencyList[i].point.y, 20, 20, texture);
	}
}

void Map::Update(const std::string& jsonDynamicData) {
	try {
		std::stringstream ss;
		ss << jsonDynamicData;
		Json::Document doc = Json::Load(ss);
		auto nodeMap = doc.GetRoot().AsMap();
		for (const auto& node : nodeMap["posts"].AsArray()) {
			auto postMap = node.AsMap();
			posts[TranslateVertexIdx(postMap["point_idx"].AsInt())] = { static_cast<Post::PostTypes>(postMap["type"].AsInt()),
				static_cast<size_t>(postMap["idx"].AsInt()),
				postMap["name"].AsString(), static_cast<size_t>(postMap["point_idx"].AsInt()) };
		}
	}
	catch (...) {
		throw std::runtime_error{ "Map::Update error" };
	}
}

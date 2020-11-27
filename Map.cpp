#include "Map.h"
#include "json.h"
#include <sstream>

constexpr int TEXTURE_SIDE = 40;

Map::Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData, TextureManager& textureManager) : 
		Graph{ jsonStructureData, jsonCoordinatesData },
		textureManager{ textureManager } {
	posts.resize(adjacencyList.size(), { Post::PostTypes::NONE, 0, "", 0 });
	Update(jsonDynamicData);
}

int Map::GetClosestMarket(int from)
{
	while (true) {
		if (posts[(from++) % posts.size()].type == Post::PostTypes::MARKET) {
			return from - 1;
		}
	}
}

void Map::Draw(SdlWindow& window) {
	DrawEdges(window);
	for (int i = 0; i < posts.size(); ++i) {
		SDL_Texture* texture = nullptr;
		int textureSide = TEXTURE_SIDE;
		int offsetY = 0;
		switch (posts[i].type) {
		case Post::PostTypes::NONE:
			texture = textureManager["assets//none.png"];
			break;
		case Post::PostTypes::TOWN:
			texture = textureManager["assets//town.png"];
			textureSide *= 2;
			offsetY -= TEXTURE_SIDE * 0.66;
			break;
		case Post::PostTypes::MARKET:
			texture = textureManager["assets//market.png"];
			break;
		case Post::PostTypes::STORAGE:
			texture = textureManager["assets//storage.png"];
			break;
		}
		 
		window.DrawTexture(adjacencyList[i].point.x, adjacencyList[i].point.y, textureSide, textureSide, texture, offsetY); 
		switch (posts[i].type) {
		case Post::PostTypes::NONE:
			break;
		case Post::PostTypes::TOWN:
		{
			int x = adjacencyList[i].point.x;
			int y = adjacencyList[i].point.y;
			window.SetDrawColor(255, 0, 0);
			window.FillRectangle(x, y, 15, textureSide, -textureSide);
			window.SetDrawColor(0, 255, 0);
			window.FillRectangle(x, y, 5, textureSide * (posts[i].goodsLoad / posts[i].goodsCapacity), -textureSide);
			window.SetDrawColor(0, 0, 255);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide * (posts[i].armorLoad / posts[i].armorCapacity), 5 - textureSide);
			window.SetDrawColor(255, 0, 255);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide * (posts[i].populationLoad / posts[i].populationCapacity), -(5 + textureSide));
		}
			break;
		case Post::PostTypes::MARKET:
			window.SetDrawColor(255, 0, 0);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide, -textureSide / 1.5);
			window.SetDrawColor(0, 255, 0);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide * (posts[i].goodsLoad / posts[i].goodsCapacity), -textureSide / 1.5);
			break;
		case Post::PostTypes::STORAGE:
			window.SetDrawColor(255, 0, 0);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide, -textureSide / 1.5);
			window.SetDrawColor(0, 0, 255);
			window.FillRectangle(adjacencyList[i].point.x, adjacencyList[i].point.y, 5, textureSide * (posts[i].armorLoad / posts[i].armorCapacity), -textureSide / 1.5);
			break;
		}
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
			if (posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].type == Post::PostTypes::TOWN) {
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].goodsCapacity = postMap["product_capacity"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].goodsLoad = postMap["product"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].armorCapacity = postMap["armor_capacity"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].armorLoad = postMap["armor"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].populationCapacity = postMap["population_capacity"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].populationLoad = postMap["population"].AsDouble();
			}
			else if(posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].type == Post::PostTypes::MARKET) {
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].goodsCapacity = postMap["product_capacity"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].goodsLoad = postMap["product"].AsDouble();
			}
			else if (posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].type == Post::PostTypes::STORAGE) {
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].armorCapacity = postMap["armor_capacity"].AsDouble();
				posts[TranslateVertexIdx(postMap["point_idx"].AsInt())].armorLoad = postMap["armor"].AsDouble();
			}
		}
	}
	catch (...) {
		throw std::runtime_error{ "Map::Update error" };
	}
}

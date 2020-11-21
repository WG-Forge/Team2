#pragma once
#include "graph.h"

struct Event {};

struct Post {
	enum class PostTypes {
		NONE = 0,
		TOWN,
		MARKET,
		STORAGE
	};
	PostTypes type;
	size_t idx;
	std::string name;
	size_t pointIdx;
};

class Map : public Graph
{
private:
	TextureManager& textureManager;
	std::map<size_t, size_t> postIdxConverter;
	std::vector <Post> posts;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData, const std::string& jsonDynamicData, TextureManager& textureManager);
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonDynamicData); // updated postsInfo
};
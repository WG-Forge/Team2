#pragma once
#include "graph.h"

enum class PostTypes {
	NONE = 0,
	TOWN,
	MARKET,
	STORAGE
};

class Map : public Graph
{
private:
	std::vector<PostTypes> postsInfo;
public:
	Map(const std::string& jsonStructureData, const std::string& jsonCoordinatesData); // postsInfo is filled with none, size = adjacencyList.size()
	void Draw(SdlWindow& window) override;
	void Update(const std::string& jsonData); // updated postsInfo
	~Map() override;
};